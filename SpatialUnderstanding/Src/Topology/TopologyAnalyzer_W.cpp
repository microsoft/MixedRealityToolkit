// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include <pch.h>
#include <Topology\TopologyAnalyzer_W.h>
#include <PlaySpace\PlaySpaceInfos_W.h>
#include <Eigen_Z.h>

#define TOPOLOGIE_MAX_ZONE_ID	2048

U32		IncreaseCellDist(U32 v)
{
	if( v == CELL_NO_DIST )
		return CELL_NO_DIST;

	return v+1;
}

TopologyAnalyzer_W::TopologyAnalyzer_W()
{
	m_IsAnalysed = FALSE;
	m_fSizeVoxel = 0.f;
	m_YGround = 0.f;

	U16	excludedProcessorAffinities[] = { 0, 3, 5 };//3, 5};
	m_AnalyzeJob.Init(excludedProcessorAffinities, _countof(excludedProcessorAffinities), "Topology");
}

//-----------------------------------

TopologyAnalyzer_W::~TopologyAnalyzer_W()
{
	m_AnalyzeJob.Shut();
}

//-----------------------------------

void	TopologyAnalyzer_W::Analyze(const PlaySpaceInfos_W* _pPlayspaceInfo,Bool _UseJob)
{
	if (!_UseJob)
	{
		m_AnalyzeJob.WaitForCompletion();
		Reset();
		OneFrameAnalyze(_pPlayspaceInfo);
		return;
	}

	// Job...
	if (!m_AnalyzeJob.PollForCompletion())
		return;
	Reset();
	AnalyzeTask_NumCount = 0;
	AnalyzeTask_PlayspaceInfo = _pPlayspaceInfo;
	m_AnalyzeJob.Run(1, AnalyzeJobTask, this);
}

//-----------------------------------

S32 TopologyAnalyzer_W::AnalyzeTask_NumCount = 0;
const PlaySpaceInfos_W* TopologyAnalyzer_W::AnalyzeTask_PlayspaceInfo = NULL;

void	TopologyAnalyzer_W::AnalyzeJobTask(const U16 taskIndex, const U16 tasksCount, void* userData)
{
	S32 CurJobNum = Thread_Z::SafeIncrement(&AnalyzeTask_NumCount);
	if (CurJobNum != 1)
		return;

	TopologyAnalyzer_W* pTopology = (TopologyAnalyzer_W*)userData;
	pTopology->OneFrameAnalyze(AnalyzeTask_PlayspaceInfo);
}

//-----------------------------------

void TopologyAnalyzer_W::OneFrameAnalyze(const PlaySpaceInfos_W* _pPlayspaceInfo)
{
	{
		SetupSurface(_pPlayspaceInfo);
		SetupCeiling(_pPlayspaceInfo);
		if ((PlaySpaceInfos_W::g_TypeScan & PlaySpaceInfos_W::PSI_SCAN_NEW))
		{
			// Too small surfaces are not taken
			for (S32 i = m_daSurfaces.GetSize() - 1; i >= 0; --i)
			{
				S32 nbCellX = m_daSurfaces[i].m_vMaxPos2D.x - m_daSurfaces[i].m_vMinPos2D.x + 1;
				S32 nbCellY = m_daSurfaces[i].m_vMaxPos2D.y - m_daSurfaces[i].m_vMinPos2D.y + 1;

				if (nbCellX * nbCellY <= 16)
					m_daSurfaces.Remove(i);
			}
		}

		ComputeCellsInfo(_pPlayspaceInfo);
		SetupNeighbors(_pPlayspaceInfo);
		SetupSurfaceGroups(_pPlayspaceInfo);
		SetupWall(_pPlayspaceInfo);
	}

	m_fSizeVoxel = _pPlayspaceInfo->m_SizeVoxel;
	m_YGround = _pPlayspaceInfo->m_YGround;
	m_YCeiling = _pPlayspaceInfo->m_YCeiling;
    m_vMinPos2D = Vec2i(0, 0);
    m_vMaxPos2D = Vec2i(_pPlayspaceInfo->m_NbCellX - 1, _pPlayspaceInfo->m_NbCellY - 1);
    m_vMinPos = _pPlayspaceInfo->m_vMinAvailable;
	m_vPlaySpaceCenter = _pPlayspaceInfo->m_vCenter;
	m_vPlayerCenter =  _pPlayspaceInfo->m_vOriginalPlayerPos;
	m_vPlayerDirection =  _pPlayspaceInfo->m_vOriginalPlayerDir;
	Vec3f	testcenter=m_vPlaySpaceCenter;
	testcenter.y=Min(GetYGround()+1.6f,GetYCeiling()-0.5f);
	m_vRoomCenter = m_vPlaySpaceCenter;
	m_vRoomCenter.y=testcenter.y;
	m_vRoomCenter.x=0.f;
	m_vRoomCenter.z=0.f;
	S32	nbcells=0;
	for (S32 s = 0; s < m_daSurfaces.GetSize(); ++s)
	{
		if (!m_daSurfaces[s].m_bIsCeiling)
			continue;

		S32 cellsSize = m_daSurfaces[s].m_daCells.GetSize();
		for (S32 c = 0; c < cellsSize; ++c)
		{
			if (m_daSurfaces[s].m_daCells[c].m_iDistFromVoid != 0 && m_daSurfaces[s].m_daCells[c].m_iDistFromWall != 0)
			{
				Vec3f cellPos = GetCellPosition(m_daSurfaces[s], c);
				Vec3f result;
				cellPos.y=testcenter.y;
				if (const_cast<PlaySpaceInfos_W*>(_pPlayspaceInfo)->RayCastVoxel(testcenter, cellPos, result))
					continue;
				m_vRoomCenter.x+=cellPos.x;
				m_vRoomCenter.z+=cellPos.z;
				nbcells++;
			}
		}
	}
	if (nbcells>0)
	{
		m_vRoomCenter.x/=nbcells;
		m_vRoomCenter.z/=nbcells;
	}
	else
	{
		m_vRoomCenter.x=m_vPlaySpaceCenter.x;
		m_vRoomCenter.z=m_vPlaySpaceCenter.z;
	}

	m_IsAnalysed = TRUE;
}

//-----------------------------------

void	TopologyAnalyzer_W::Reset()
{
	m_AnalyzeJob.WaitForCompletion();

	m_IsAnalysed = FALSE;
	m_daSurfaces.Flush();
	m_daSurfaceGroups.Flush();
	m_daWalls.Flush();
}

//-----------------------------------

static Float fSurfaceTolerance = 0.9397f; // cos(20)

void	TopologyAnalyzer_W::SetupSurface(const PlaySpaceInfos_W* _pPlayspaceInfos)
{
	S32 nbCellX = _pPlayspaceInfos->m_NbCellX;
	S32 nbCellY = _pPlayspaceInfos->m_NbCellY;
	Float fCellArea = _pPlayspaceInfos->m_SizeVoxel * _pPlayspaceInfos->m_SizeVoxel;
	Float fSizeCell = _pPlayspaceInfos->m_SizeVoxel;
	Float fHalfCell = .5f * _pPlayspaceInfos->m_SizeVoxel;

	PlaySpace_CellInfos *pCell = _pPlayspaceInfos->m_pSurfaceInfosV3->m_Surfels.m_CellBoard.GetArrayPtr();
	for (S32 y = 0; y < nbCellY; y++)
	{
		for (S32 x = 0; x < nbCellX; x++)
		{
			PlaySpace_Surfel *pSurfel = pSurfel = pCell->pFirst;
			while (pSurfel)
			{
				if ((pSurfel->ZoneId < 0)
					|| (pSurfel->iDir != PlaySpace_Surfel::SURFDIR_UP)
					|| ((pSurfel->Flags & PlaySpace_Surfel::SURFFLAG_VIRTUAL) != 0)
					|| (pSurfel->Normal.y < fSurfaceTolerance)
					|| (pSurfel->Point.y < (_pPlayspaceInfos->m_YGround-0.1f))
					)
				{
					pSurfel = pSurfel->pNext;
					continue;
				}

				S16 zone = pSurfel->ZoneId;
				Float curHeight = pSurfel->Point.y;
				// find surface
				Surface* pSurface = NULL;
				for (S32 s = 0; s < m_daSurfaces.GetSize(); s++)
				{
					if (m_daSurfaces[s].m_iZoneId == zone
						&& Abs(curHeight-m_daSurfaces[s].m_fWorldHeight) < fSizeCell)
					{
						pSurface = &m_daSurfaces[s];
						break;
					}
				}
				// if not, create it
				if (!pSurface)
				{
					pSurface = &(m_daSurfaces[m_daSurfaces.Add()]);
					pSurface->m_iZoneId = zone;
					pSurface->m_fWorldHeight = pSurfel->Point.y;	// min;
					pSurface->m_fHeightFromGround = pSurface->m_fWorldHeight - _pPlayspaceInfos->m_YGround;
					pSurface->m_vMaxPos = Vec3f(pSurfel->Point.x /*+ fHalfCell*/, pSurface->m_fWorldHeight, pSurfel->Point.z /*+ fHalfCell*/);
					pSurface->m_vMinPos = Vec3f(pSurfel->Point.x /*- fHalfCell*/, pSurface->m_fWorldHeight, pSurfel->Point.z /*- fHalfCell*/);
					pSurface->m_vMaxPos2D = Vec2i(x, y);
					pSurface->m_vMinPos2D = pSurface->m_vMaxPos2D;
					pSurface->m_bIsGround = ((pSurface->m_fHeightFromGround < 0.1f));	//(pSurfel == pSurfelMin);
					pSurface->m_bIsCeiling = FALSE;
				}

				// find upeer surfel
				Float space = 1000.f;
				PlaySpace_Surfel *pUpSurfel = pSurfel->pNext;
				while(pUpSurfel)
				{
					if( pUpSurfel->iDir != PlaySpace_Surfel::SURFDIR_UP 
						&& (pUpSurfel->Point.y - pSurfel->Point.y) > fSizeCell )
					{
						space = pUpSurfel->Point.y - pSurfel->Point.y;
						break;
					}

					pUpSurfel = pUpSurfel->pNext;
				}

				// ref pos (to compute min/max) 
				Vec3f minRefPos(pSurfel->Point.x /*- fHalfCell*/,pSurface->m_fWorldHeight,pSurfel->Point.z /*- fHalfCell*/);
				Vec3f maxRefPos(pSurfel->Point.x /*+ fHalfCell*/,pSurface->m_fWorldHeight,pSurfel->Point.z /*+ fHalfCell*/);

				// compute infos
				pSurface->m_iNbCell++;
				pSurface->m_fArea += fCellArea;
				pSurface->m_fWorldHeight = (pSurface->m_fWorldHeight * (pSurface->m_iNbCell-1) + pSurfel->Point.y) / Float(pSurface->m_iNbCell);// average
				pSurface->m_fHeightFromGround = pSurface->m_fWorldHeight - _pPlayspaceInfos->m_YGround;
				pSurface->m_bIsGround = ((pSurface->m_fHeightFromGround < 0.1f));	//(pSurfel == pSurfelMin);
				pSurface->m_fMinSpace = Min(pSurface->m_fMinSpace, space);
				pSurface->m_vMinPos.x = Min(pSurface->m_vMinPos.x,minRefPos.x);
				pSurface->m_vMinPos.y = pSurface->m_fWorldHeight;
				pSurface->m_vMinPos.z = Min(pSurface->m_vMinPos.z,minRefPos.z);
				pSurface->m_vMaxPos.x = Max(pSurface->m_vMaxPos.x,maxRefPos.x);
				pSurface->m_vMaxPos.y = pSurface->m_fWorldHeight;
				pSurface->m_vMaxPos.z = Max(pSurface->m_vMaxPos.z,maxRefPos.z);
				pSurface->m_vMinPos2D.x = Min(pSurface->m_vMinPos2D.x, x);
				pSurface->m_vMinPos2D.y = Min(pSurface->m_vMinPos2D.y, y);
				pSurface->m_vMaxPos2D.x = Max(pSurface->m_vMaxPos2D.x, x);
				pSurface->m_vMaxPos2D.y = Max(pSurface->m_vMaxPos2D.y, y);
				pSurfel = pSurfel->pNext;
			}
			pCell++;		// Next cell
		}
	}

	// remove 1D surfaces
	for (S32 s = m_daSurfaces.GetSize() - 1; s >= 0; s--)
	{
		Surface& currentSurface = m_daSurfaces[s];
		if (currentSurface.m_vMinPos2D.x == currentSurface.m_vMaxPos2D.x
			|| currentSurface.m_vMinPos2D.y == currentSurface.m_vMaxPos2D.y)
		{
			m_daSurfaces.Remove(s);
		}
	}
}

//-----------------------------------

void	TopologyAnalyzer_W::SetupNeighborsV3(const PlaySpaceInfos_W* _pPlayspaceInfos)
{
	PlaySpace_SurfelBoard	&SurfelBoard = _pPlayspaceInfos->m_pSurfaceInfosV3->m_Surfels;
	for (S32 s1 = 0; s1 < m_daSurfaces.GetSize(); s1++)
	{
		Surface& surface1 = m_daSurfaces[s1];
		
		for (S32 s2 = s1 + 1; s2 < m_daSurfaces.GetSize(); s2++)
		{
			Surface& surface2 = m_daSurfaces[s2];

			if( Abs(surface2.m_fWorldHeight - surface1.m_fWorldHeight) > 1.f )
				continue;

			Surface		*pSurfaceToCheck;
			S32			ZoneIdToCheck;
			S32			ZoneIdToAccess;
			if (surface1.m_vMaxPos.y > surface2.m_vMaxPos.y)
			{
				pSurfaceToCheck	= &surface1;
				ZoneIdToCheck = surface1.m_iZoneId;
				ZoneIdToAccess = surface2.m_iZoneId;
			}
			else
			{
				pSurfaceToCheck	= &surface2;
				ZoneIdToCheck = surface2.m_iZoneId;
				ZoneIdToAccess = surface1.m_iZoneId;
			}

			Bool	ZoneAreLinked = FALSE;
			S32		Size = pSurfaceToCheck->m_daCells.GetSize();
			S32		c1 = 0;
			S32		EndX = pSurfaceToCheck->m_vMaxPos2D.x;
			S32		EndY = pSurfaceToCheck->m_vMaxPos2D.y;

			for (S32 y1 = pSurfaceToCheck->m_vMinPos2D.y ; y1<=EndY ; y1++)
			for (S32 x1 = pSurfaceToCheck->m_vMinPos2D.x ; x1<=EndX ; x1++,c1++)
			{
				CellInfo &CurCell = pSurfaceToCheck->m_daCells[c1];
				if (!CurCell.IsValid() || CurCell.m_iDistFromVoid == 0 || CurCell.m_iDistFromVoid == CELL_NO_DIST)
					continue;

				PlaySpace_CellInfos	*pCell  = SurfelBoard.GetCell(x1,y1);
				if (!pCell)
					continue;
				PlaySpace_Surfel	*pSurfel = pCell->pFirst;
				while (pSurfel)
				{
					if (pSurfel->ZoneId == ZoneIdToCheck)
					{
						S32 zoneId = -1;
						Vec3i	Dir = VEC3I_NULL;	
						Dir.x = -1;
						if(	 pSurfel->x > 0 && SurfelBoard.CanGoDown(pSurfel,Dir,zoneId) && zoneId == ZoneIdToAccess )	
						{
							ZoneAreLinked = TRUE;
							break;
						}

						Dir.x = +1;
						if( pSurfel->x < (_pPlayspaceInfos->m_NbCellX-1) && SurfelBoard.CanGoDown(pSurfel,Dir,zoneId)  && zoneId == ZoneIdToAccess )	
						{
							ZoneAreLinked = TRUE;
							break;
						}

						Dir.x = 0;
						Dir.z = -1;

						if( pSurfel->z > 0 && SurfelBoard.CanGoDown(pSurfel,Dir,zoneId)  && zoneId == ZoneIdToAccess )	
						{
							ZoneAreLinked = TRUE;
							break;
						}

						Dir.z = +1;
						if( pSurfel->z < (_pPlayspaceInfos->m_NbCellY-1) && SurfelBoard.CanGoDown(pSurfel,Dir,zoneId)  && zoneId == ZoneIdToAccess )	
						{
							ZoneAreLinked = TRUE;
							break;
						}

						// Stop.
						break;
					}
					// Next Surfel.
					pSurfel = pSurfel->pNext;
				}

				// Linked ?
				if (ZoneAreLinked)
				{
					surface1.m_daNeighbors.Add(&surface2);
					surface2.m_daNeighbors.Add(&surface1);
					break;
				}
			}
		}
	}
}

//-----------------------------------

void	TopologyAnalyzer_W::SetupNeighbors(const PlaySpaceInfos_W* _pPlayspaceInfos)
{
	if (PlaySpaceInfos_W::g_TypeScan & PlaySpaceInfos_W::PSI_SCAN_NEW)
	{
		SetupNeighborsV3(_pPlayspaceInfos);
		return;
	}

	for (S32 s1 = 0; s1 < m_daSurfaces.GetSize(); s1++)
	{
		Surface& surface1 = m_daSurfaces[s1];
			
		S32 surface1SizeX = surface1.m_vMaxPos2D.x - surface1.m_vMinPos2D.x + 1;

		for (S32 s2 = s1 + 1; s2 < m_daSurfaces.GetSize(); s2++)
		{
			Surface& surface2 = m_daSurfaces[s2];

			if( Abs(surface2.m_fWorldHeight - surface1.m_fWorldHeight) > 1.f )
				continue;
			
			S32 surface2SizeX = surface2.m_vMaxPos2D.x - surface2.m_vMinPos2D.x + 1;

			for (S32 c1 = 0; c1 < surface1.m_daCells.GetSize(); c1++)
			{
				if (!surface1.m_daCells[c1].IsValid() || surface1.m_daCells[c1].m_iDistFromVoid == 0 || surface1.m_daCells[c1].m_iDistFromVoid == CELL_NO_DIST)
					continue;

				S32 x1 = c1 % surface1SizeX + surface1.m_vMinPos2D.x;
				S32 y1 = c1 / surface1SizeX + surface1.m_vMinPos2D.y;

				bool neighborsAdd = false;

				for (S32 c2 = 0; c2 < surface2.m_daCells.GetSize(); c2++)
				{
					if (!surface2.m_daCells[c2].IsValid() || surface2.m_daCells[c2].m_iDistFromVoid == 0 || surface2.m_daCells[c2].m_iDistFromVoid == CELL_NO_DIST)
						continue;

					S32 x2 = c2 % surface2SizeX + surface2.m_vMinPos2D.x;
					S32 y2 = c2 / surface2SizeX + surface2.m_vMinPos2D.y;

					if ((x1 == x2 && y1 == y2) ||
						(x1 == x2 && (y1 == y2 - 1 || y1 == y2 + 1)) ||
						(y1 == y2 && (x1 == x2 - 1 || x1 == x2 + 1)))
					{
						surface1.m_daNeighbors.Add(&surface2);
						surface2.m_daNeighbors.Add(&surface1);

						neighborsAdd = true;
						break;
					}
				}

				if (neighborsAdd)
					break;
			}
		}
	}
}

//-----------------------------------

void    TopologyAnalyzer_W::SetupSurfaceGroups(const PlaySpaceInfos_W* _pPlayspaceInfos)
{
	// Make groups of neighbors surfaces
	for (S32 s = 0; s < m_daSurfaces.GetSize(); s++)
	{
		const Surface& surface = m_daSurfaces[s];

		// Ground is not in a group
		if (surface.m_bIsGround || surface.m_bIsCeiling)
			continue;

		// Check if the surface is not already in other group
		bool inAGroup = false;
		for (S32 g = 0; g < m_daSurfaceGroups.GetSize(); g++)
		{
			S32 size = m_daSurfaceGroups[g].GetSize();
			if (m_daSurfaceGroups[g].Contains(&surface) != -1)
			{
				inAGroup = true;
				break;
			}
		}

		// Create the group of this surface
		if (!inAGroup)
		{
			m_daSurfaceGroups.Add();
			m_daSurfaceGroups[m_daSurfaceGroups.GetSize() - 1].Add(&surface);
			AddAllNeighborsRec(m_daSurfaceGroups[m_daSurfaceGroups.GetSize() - 1], &surface);
		}
	}
}

//-----------------------------------

void	TopologyAnalyzer_W::SetupWall(const PlaySpaceInfos_W* _pPlayspaceInfos)
{
	S32 nbCellX = _pPlayspaceInfos->m_NbCellX;
	S32 nbCellY = _pPlayspaceInfos->m_NbCellY;
	Vec3f Origin = _pPlayspaceInfos->m_vMinAvailable;
	Float fSizeCell = _pPlayspaceInfos->m_SizeVoxel;

	// First : Get all walls and compute normal + centroid.
	PlaySpace_CellInfos *pCell = _pPlayspaceInfos->m_pSurfaceInfosV3->m_Surfels.m_CellBoard.GetArrayPtr();
	for (S32 y = 0; y < nbCellY; y++)
	{
		for (S32 x = 0; x < nbCellX; x++)
		{
			PlaySpace_Surfel* pCur = pCell->pFirst;
			while (pCur)
			{
				if ((pCur->ZoneId != -1)
					&& (pCur->iDir != PlaySpace_Surfel::SURFDIR_UP)
					&& (pCur->iDir != PlaySpace_Surfel::SURFDIR_DOWN)
					)
				{
					Wall* pWall = NULL;
					EXCEPTIONC_Z(pCur->ZoneId < _pPlayspaceInfos->m_NbZoneID, "Error ZoneID TopologyAnalyzer_W::SetupWall");
					Bool bIsVirtual = (pCur->Flags & PlaySpace_Surfel::SURFFLAG_VIRTUAL);
					Bool bIsExternal = (pCur->Flags & PlaySpace_Surfel::SURFFLAG_EXTERNAL);
						
					// find previous wall
					for(S32 w=0; w<m_daWalls.GetSize(); w++)
					{
						Wall* pPrevWall = &(m_daWalls[w]);
						if( pPrevWall->m_ZoneID == pCur->ZoneId
							&& pPrevWall->m_bIsVirtual == bIsVirtual
							&& pPrevWall->m_bIsExternal == bIsExternal)
						{
							pWall = pPrevWall;
							break;
						}
					}

					//create new wall
					if (!pWall)
					{
						pWall = &(m_daWalls[m_daWalls.Add()]);
						pWall->m_vNormal = VEC3F_NULL;
						pWall->m_fDistWall_DeprecatedV3 = 0.f;
						pWall->m_bIsVirtual = bIsVirtual;
						pWall->m_ZoneID = pCur->ZoneId;
						pWall->m_bIsExternal = bIsExternal;
					}

					// update wall
					pWall->m_daSurfels.Add(pCur);
					pWall->m_daPoints.Add(pCur->Point);
					pWall->m_vCentroid = ((pWall->m_vCentroid * pWall->m_fNbSurfel) + pCur->Point) / (pWall->m_fNbSurfel + 1.f);
					pWall->m_fNbSurfel += 1.f;
					pWall->m_vNormal += pCur->Normal;
				}
				pCur = pCur->pNext;
			}
			pCell++;
		}
	}

	// Second pass : normalize normal,up,tangent, compute min/max, basestart, baseend, size
	for(S32 w=0; w<m_daWalls.GetSize(); w++)
	{
		Wall* pWall = &(m_daWalls[w]);

		// Eigen to find normal
		pWall->m_vNormal.CNormalize();
		Vec3f eigenvectors[3];
		Float eigenvalues[3];
		GenericEigen3D(pWall->m_daPoints.GetArrayPtr(), pWall->m_daPoints.GetSize(), eigenvectors, eigenvalues);

		// Sort Eigen
		if (eigenvalues[2] > eigenvalues[1])
		{
			::Swap(eigenvalues[2], eigenvalues[1]);
			::Swap(eigenvectors[2], eigenvectors[1]);
		}
		if (eigenvalues[1] > eigenvalues[0])
		{
			::Swap(eigenvalues[0], eigenvalues[1]);
			::Swap(eigenvectors[0], eigenvectors[1]);
		}
		if (eigenvalues[2] > eigenvalues[1])
		{
			::Swap(eigenvalues[2], eigenvalues[1]);
			::Swap(eigenvectors[2], eigenvectors[1]);
		}
		if (eigenvalues[1] < 0.2f)
		{
			Vec3f eigenNormal = eigenvectors[0] ^ eigenvectors[1];
			eigenNormal.ANormalize();
			if( (eigenNormal * pWall->m_vNormal ) < 0.f )
				eigenNormal = -eigenNormal;

			pWall->m_vNormal = eigenNormal;
		}
				

		// up / tangent
		Bool	IsOrientedX = (Abs(pWall->m_vNormal.x) < Abs(pWall->m_vNormal.z));
		if (IsOrientedX)
		{
			pWall->m_vUp = VEC3F_LEFT ^ pWall->m_vNormal;
			pWall->m_vTangent = pWall->m_vNormal ^ pWall->m_vUp;

			pWall->m_vTangent.y = 0;
			pWall->m_vTangent.CHNormalize();

			pWall->m_vUp = pWall->m_vTangent ^ pWall->m_vNormal;
			pWall->m_vUp.CNormalize();
			if (pWall->m_vUp.y < 0.f)
				pWall->m_vUp = -pWall->m_vUp;
		}
		else
		{
			pWall->m_vUp = VEC3F_FRONT ^ pWall->m_vNormal;
			pWall->m_vTangent = pWall->m_vNormal ^ pWall->m_vUp;

			pWall->m_vTangent.y = 0;
			pWall->m_vTangent.CHNormalize();

			pWall->m_vUp = pWall->m_vTangent ^ pWall->m_vNormal;
			pWall->m_vUp.CNormalize();
			if (pWall->m_vUp.y < 0.f)
				pWall->m_vUp = -pWall->m_vUp;
		}

		// compute min/max projected on wall
		Vec2f projMin(Float_Max);
		Vec2f projMax(-Float_Max);
		for(S32 s=0; s<pWall->m_daSurfels.GetSize(); s++)
		{
			PlaySpace_Surfel* pCur = pWall->m_daSurfels[s];
			Vec3f centroidToSurfel = pCur->Point - pWall->m_vCentroid;
			Float dist = (centroidToSurfel*pWall->m_vNormal);
			if(!pWall->m_bIsVirtual && Abs(dist) > 0.02f )
				continue;

			Float projX = (centroidToSurfel*pWall->m_vTangent);
			Float projY = (centroidToSurfel*pWall->m_vUp);
			projMin.x = Min(projMin.x,projX);
			projMin.y = Min(projMin.y,projY);
			projMax.x = Max(projMax.x,projX);
			projMax.y = Max(projMax.y,projY);
		}

		// setup bases & size
		pWall->m_fWidth = projMax.x - projMin.x;
		pWall->m_fHeight = projMax.y - projMin.y;
		pWall->m_vBaseStart = pWall->m_vCentroid + projMin.x * pWall->m_vTangent + projMin.y * pWall->m_vUp;
		pWall->m_vBaseEnd = pWall->m_vCentroid + projMax.x * pWall->m_vTangent + projMin.y * pWall->m_vUp;
		pWall->m_vCentroid = pWall->m_vBaseStart + pWall->m_fWidth * .5f * pWall->m_vTangent + pWall->m_fHeight * .5f * pWall->m_vUp;

		// setup columns
		if( pWall->m_fWidth >= fSizeCell) 
		{
			S32 nbColumns = FLOORINT(pWall->m_fWidth / fSizeCell) + 1;
			Float columnSize = pWall->m_fWidth / Float(nbColumns);
			pWall->m_daColumns.SetSize(nbColumns);
			for(S32 c=0; c<pWall->m_daColumns.GetSize(); c++)
			{
				WallColumnInfo& column = pWall->m_daColumns[c];
				column.m_fMin = 2000.f;
				column.m_fMax = -2000.f;
				column.m_bIsValid = FALSE;
			}

			// init columns from surfel
			for(S32 s=0; s<pWall->m_daSurfels.GetSize(); s++)
			{
				PlaySpace_Surfel* pCur = pWall->m_daSurfels[s];
				Vec3f centroidToSurfel = pCur->Point - pWall->m_vCentroid;
				Float dist = (centroidToSurfel*pWall->m_vNormal);
				if(!pWall->m_bIsVirtual && Abs(dist) > 0.08f )
					continue;

				Float projX = (centroidToSurfel*pWall->m_vTangent);
				S32 colIdx = Clamp(FLOORINT((pWall->m_fWidth * .5f + projX) / columnSize),0,pWall->m_daColumns.GetSize()-1);
				WallColumnInfo& column = pWall->m_daColumns[colIdx];
				column.m_fMin = Min(column.m_fMin,pCur->Point.y);
				column.m_fMax = Max(column.m_fMax,pCur->Point.y);
				column.m_bIsValid = (column.m_fMax - column.m_fMin) > fSizeCell;
				column.m_daSurfels.Add(pCur);

				Float columSize = (column.m_fMax - column.m_fMin);
				S32 nbParts = FLOORINT(columSize / fSizeCell) + 1;
				column.m_fPartHeight = columSize / Float(nbParts);
			}

			// init space at the bottom of the columns
			S32 dx = 0;
			S32 dy = 0;
			Vec3f vHNormal = pWall->m_vNormal;
			vHNormal.y = 0.f;
			S32 WallDir = PlaySpace_Surfel::Normal2iDir(vHNormal);
			if (WallDir == PlaySpace_Surfel::SURFDIR_LEFT)
				dx = 1;
			else if (WallDir == PlaySpace_Surfel::SURFDIR_RIGHT)
				dx = -1;
			else if (WallDir == PlaySpace_Surfel::SURFDIR_FRONT)
				dy = 1;
			else if (WallDir == PlaySpace_Surfel::SURFDIR_BACK)
				dy = -1;

			pWall->m_fArea = pWall->m_fWidth * pWall->m_fHeight;
			if (dx!=0 || dy!=0)
			{
				pWall->m_fArea = 0.f; // compute from columns
				for(S32 c=0; c<pWall->m_daColumns.GetSize(); c++)
				{
					WallColumnInfo& column = pWall->m_daColumns[c];
					if( !column.m_bIsValid )
						continue;

					S32 x=0,z=0;
					Vec3f startColumn = pWall->m_vBaseStart + pWall->m_vTangent * (columnSize * (Float(c) + 0.5f));
					x = FLOORINT((startColumn.x - Origin.x) / fSizeCell);
					z = FLOORINT((startColumn.z - Origin.z) / fSizeCell);
					column.m_iSpaceBottomSize = ComputeSpaceBottomSize_V3(_pPlayspaceInfos, x, z, column.m_fMin, dx, dy);
					Float columSize = (column.m_fMax - column.m_fMin);
					S32 nbParts = FLOORINT(columSize / fSizeCell) + 1;
					column.m_daSpaceInFrontOf.SetSize(nbParts);
					for(S32 p=0;p<nbParts; p++)
						column.m_daSpaceInFrontOf[p] = 0;

					// init surfel on columns
					for (S32 s = 0; s < column.m_daSurfels.GetSize(); s++)
					{
						PlaySpace_Surfel* pCur = column.m_daSurfels[s];
						Float surfelHeight = pCur->Point.y;

						if (surfelHeight > column.m_fMax - Float_Eps || surfelHeight < column.m_fMin + Float_Eps)
							continue;

						S32 idx = FLOORINT((surfelHeight - column.m_fMin) / column.m_fPartHeight);
						column.m_daSpaceInFrontOf[idx] = 1000;
						if(idx>0)
							column.m_daSpaceInFrontOf[idx-1] = 1000;
						if(idx<column.m_daSpaceInFrontOf.GetSize()-1)
							column.m_daSpaceInFrontOf[idx + 1] = 1000;
					}
						
					ComputeSpaceInFrontOf_V3(_pPlayspaceInfos, x, z, *pWall, column, dx, dy);

					pWall->m_fArea += (column.m_fMax - column.m_fMin) * columnSize;
				}
			}
		}
	}

	// remove 1D surfaces
	for(S32 w=m_daWalls.GetSize()-1; w>=0; w--)
	{

		Wall& wall = m_daWalls[w];
		if( (wall.m_daColumns.GetSize() <= 1 )
			|| (wall.m_fHeight < (_pPlayspaceInfos->m_SizeVoxel * 2.f)))
		{
			m_daWalls.Remove(w);
		}
	}
}

//-----------------------------------

S32		TopologyAnalyzer_W::ComputeSpaceBottomSize(const PlaySpace_SurfaceInfos	* _pFrom, S32 _delta)
{
	S32 size = 1;
	const PlaySpace_SurfaceInfos	* pCurCell = _pFrom;
	
	while((pCurCell->Level[0].Flags & PlaySpace_SurfaceInfos::PSI_BORDER) == 0
		&& (pCurCell->Level[0].Flags & PlaySpace_SurfaceInfos::PSI_WALL) == 0
		&& pCurCell->Level[0].hSnap <= _pFrom->Level[0].hSnap + 0.01f )
	{
		size++;
		pCurCell += _delta;
	}

	return size;
}

S32		TopologyAnalyzer_W::ComputeSpaceBottomSize_V3(const PlaySpaceInfos_W* _pPlayspaceInfos, S32 x, S32 y, Float height, S32 dx, S32 dy)
{
	S32 size = 1;
	x += dx;
	y += dy;

	while ((x >= 0) && (x < _pPlayspaceInfos->m_NbCellX) && (y >= 0) && (y < _pPlayspaceInfos->m_NbCellY))
	{
		PlaySpace_CellInfos* pCell = &_pPlayspaceInfos->m_pSurfaceInfosV3->m_Surfels.m_CellBoard[x + y * _pPlayspaceInfos->m_NbCellX];
		PlaySpace_Surfel *pSurfel = pCell->pFirst;
		while (pSurfel)
		{
			if ((pSurfel->iDir != PlaySpace_Surfel::SURFDIR_DOWN) && (pSurfel->Point.y >(height + 0.01f)))
				return size;

			pSurfel = pSurfel->pNext;
		}

		size++;
		x += dx;
		y += dy;
	}

	return size;
}

//-----------------------------------

S32		TopologyAnalyzer_W::ComputeSpaceInFrontOf_V3(const PlaySpaceInfos_W* _pPlayspaceInfos, S32 x, S32 y, Wall& wall, WallColumnInfo& column, S32 dx, S32 dy)
{
	S32 size = 0;
	//x += dx;
	//y += dy;

	Float fSizeVoxel = _pPlayspaceInfos->m_SizeVoxel;
	Float fHalfSizeVoxel = fSizeVoxel * .5f;
	Float fHeightLimit = column.m_fMin + column.m_fPartHeight * .5f;
	while ((x >= 0) && (x < _pPlayspaceInfos->m_NbCellX) && (y >= 0) && (y < _pPlayspaceInfos->m_NbCellY))
	{
		PlaySpace_CellInfos* pCell = &_pPlayspaceInfos->m_pSurfaceInfosV3->m_Surfels.m_CellBoard[x + y * _pPlayspaceInfos->m_NbCellX];
		PlaySpace_Surfel *pSurfel = pCell->pFirst;
		while (pSurfel)
		{
			Float surfelHeight = pSurfel->Point.y;
			
			if( surfelHeight > column.m_fMax - Float_Eps )
				break;

			if( wall.m_ZoneID != pSurfel->ZoneId 
				&& surfelHeight > fHeightLimit 
				&& (pSurfel->Point - wall.m_vCentroid)*wall.m_vNormal > 0.01f)
			{
				S32 idx = FLOORINT((surfelHeight - column.m_fMin)/column.m_fPartHeight);
				column.m_daSpaceInFrontOf[idx] =  Min(size,column.m_daSpaceInFrontOf[idx]);
			}
			
			pSurfel = pSurfel->pNext;
		}

		size++;
		x += dx;
		y += dy;
	}

	// clamp column to playsapce size
	for(S32 c=0;c<column.m_daSpaceInFrontOf.GetSize();c++)
	{
		column.m_daSpaceInFrontOf[c] =  Min(size,column.m_daSpaceInFrontOf[c]);
	}

	return size;
}

//-----------------------------------

TopologyAnalyzer_W::WallColumnInfo&		TopologyAnalyzer_W::AddWallColumn(const PlaySpaceInfos_W* _pPlayspaceInfos, const Vec2f& _2dPos, Float _height, Float _yMin, Float _yMax, const Vec3f& _normal, Bool _isVirtual, Bool _isExternal)
{
	// find surface, if not, add new one
	Float fHalfCell = .5f * _pPlayspaceInfos->m_SizeVoxel;
	Vec3f halfCellDir = VEC3F_NULL;
	halfCellDir.x= POS_Z(_normal.z* fHalfCell);
	halfCellDir.z= POS_Z(_normal.x* fHalfCell);
	Wall* pWall = GetWall(_pPlayspaceInfos,_2dPos,_normal,_height,_isVirtual, _isExternal);
	if( !pWall )
	{
		pWall = &(m_daWalls[m_daWalls.Add()]);
		pWall->m_vNormal = _normal;
		pWall->m_fDistWall_DeprecatedV3 = _height;
		pWall->m_vBaseStart = Vec3f(_2dPos.x, _yMin, _2dPos.y) - halfCellDir;
		pWall->m_vBaseEnd = Vec3f(_2dPos.x, _yMax, _2dPos.y) + halfCellDir;
		pWall->m_bIsVirtual = _isVirtual;
	}

	// update wall infos
	pWall->m_bIsExternal &= _isExternal;
	Vec3f refPos = Vec3f(_2dPos.x,_yMin,_2dPos.y) - halfCellDir;
	pWall->m_vBaseStart.x = Min(pWall->m_vBaseStart.x, refPos.x);
	pWall->m_vBaseStart.y = Min(pWall->m_vBaseStart.y, refPos.y);
	pWall->m_vBaseStart.z = Min(pWall->m_vBaseStart.z, refPos.z);
	refPos = Vec3f(_2dPos.x,_yMax,_2dPos.y) + halfCellDir;
	pWall->m_vBaseEnd.x = Max(pWall->m_vBaseEnd.x, refPos.x);
	pWall->m_vBaseEnd.y = Max(pWall->m_vBaseEnd.y, refPos.y);
	pWall->m_vBaseEnd.z = Max(pWall->m_vBaseEnd.z, refPos.z);
	
	// add column
	WallColumnInfo& column = pWall->m_daColumns[pWall->m_daColumns.Add()];
	column.m_fMin = _yMin;
	column.m_fMax = _yMax;

	return column;
}

//-----------------------------------

void	TopologyAnalyzer_W::SetupCeiling(const PlaySpaceInfos_W* _pPlayspaceInfos)
{
	S32 nbCellX = _pPlayspaceInfos->m_NbCellX;
	S32 nbCellY = _pPlayspaceInfos->m_NbCellY;
	Float fCellArea = _pPlayspaceInfos->m_SizeVoxel * _pPlayspaceInfos->m_SizeVoxel;
	Float fSizeCell = _pPlayspaceInfos->m_SizeVoxel;
	Float fHalfCell = .5f * _pPlayspaceInfos->m_SizeVoxel;

	PlaySpace_CellInfos *pCell = _pPlayspaceInfos->m_pSurfaceInfosV3->m_Surfels.m_CellBoard.GetArrayPtr();
	for (S32 y = 0; y < nbCellY; y++)
	{
		for (S32 x = 0; x < nbCellX; x++)
		{
			Float min = 1e8f;
			Float max = -1e8;
			PlaySpace_Surfel *pSurfelMin = NULL;
			PlaySpace_Surfel *pSurfelMax = NULL;
			PlaySpace_Surfel *pSurfel = pCell->pFirst;
			while (pSurfel)
			{
				if (pSurfel->Point.y < min)
				{
					min = pSurfel->Point.y;
					pSurfelMin = pSurfel;
				}
				if (pSurfel->Point.y > max)
				{
					max = pSurfel->Point.y;
					pSurfelMax = pSurfel;
				}
				pSurfel = pSurfel->pNext;
			}
			pSurfel = pCell->pFirst;
			while (pSurfel)
			{
				if (pSurfel->ZoneId < 0)
				{
					pSurfel = pSurfel->pNext;
					continue;
				}
				// No virtual test cause of no real ceiling scanned
				if (pSurfel->iDir != PlaySpace_Surfel::SURFDIR_DOWN)
				{
					pSurfel = pSurfel->pNext;
					continue;
				}

				if (pSurfel->Point.y < _pPlayspaceInfos->m_YCeiling - 0.2f)
				{
					pSurfel = pSurfel->pNext;
					continue;
				}

				S16 zone = pSurfel->ZoneId;
				Float curHeight = pSurfel->Point.y;
				// find surface
				Surface* pSurface = NULL;
				for (S32 s = 0; s < m_daSurfaces.GetSize(); s++)
				{
					if (m_daSurfaces[s].m_iZoneId == zone
						&& Abs(curHeight-m_daSurfaces[s].m_fWorldHeight) < fSizeCell)
					{
						pSurface = &m_daSurfaces[s];
						break;
					}
				}
				// if not, create it
				if (!pSurface)
				{
					pSurface = &(m_daSurfaces[m_daSurfaces.Add()]);
					pSurface->m_iZoneId = zone;
					pSurface->m_fWorldHeight = pSurfel->Point.y;	// min;
					pSurface->m_fHeightFromGround = pSurface->m_fWorldHeight - _pPlayspaceInfos->m_YGround;
					pSurface->m_vMaxPos = Vec3f(pSurfel->Point.x /*+ fHalfCell*/, pSurface->m_fWorldHeight, pSurfel->Point.z /*+ fHalfCell*/);
					pSurface->m_vMinPos = Vec3f(pSurfel->Point.x /*- fHalfCell*/, pSurface->m_fWorldHeight, pSurfel->Point.z /*- fHalfCell*/);
					pSurface->m_vMaxPos2D = Vec2i(x, y);
					pSurface->m_vMinPos2D = pSurface->m_vMaxPos2D;
					pSurface->m_bIsGround = ((pSurface->m_fHeightFromGround < 0.1f) && (pSurface->m_fHeightFromGround > -0.1f));	//(pSurfel == pSurfelMin);
					pSurface->m_bIsCeiling = TRUE;
				}

				// ref pos (to compute min/max) 
				Vec3f minRefPos(pSurfel->Point.x /*- fHalfCell*/,pSurface->m_fWorldHeight,pSurfel->Point.z /*- fHalfCell*/);
				Vec3f maxRefPos(pSurfel->Point.x /*+ fHalfCell*/,pSurface->m_fWorldHeight,pSurfel->Point.z /*+ fHalfCell*/);

				// compute infos
				pSurface->m_iNbCell++;
				pSurface->m_fArea += fCellArea;
				pSurface->m_fWorldHeight = (pSurface->m_fWorldHeight * (pSurface->m_iNbCell-1) + pSurfel->Point.y) / Float(pSurface->m_iNbCell);// average
				pSurface->m_fMinSpace = Min(pSurface->m_fMinSpace, max - min);
				pSurface->m_vMinPos.x = Min(pSurface->m_vMinPos.x,minRefPos.x);
				pSurface->m_vMinPos.y = pSurface->m_fWorldHeight;
				pSurface->m_vMinPos.z = Min(pSurface->m_vMinPos.z,minRefPos.z);
				pSurface->m_vMaxPos.x = Max(pSurface->m_vMaxPos.x,maxRefPos.x);
				pSurface->m_vMaxPos.y = pSurface->m_fWorldHeight;
				pSurface->m_vMaxPos.z = Max(pSurface->m_vMaxPos.z,maxRefPos.z);
				pSurface->m_vMinPos2D.x = Min(pSurface->m_vMinPos2D.x, x);
				pSurface->m_vMinPos2D.y = Min(pSurface->m_vMinPos2D.y, y);
				pSurface->m_vMaxPos2D.x = Max(pSurface->m_vMaxPos2D.x, x);
				pSurface->m_vMaxPos2D.y = Max(pSurface->m_vMaxPos2D.y, y);
				pSurfel = pSurfel->pNext;
			}
			pCell++;		// Next cell
		}
	}
}

//-----------------------------------

void	TopologyAnalyzer_W::ComputeCellsInfo(const PlaySpaceInfos_W* _pPlayspaceInfos)
{
	S32 nbCellX = _pPlayspaceInfos->m_NbCellX;
	S32 nbCellY = _pPlayspaceInfos->m_NbCellY;

	static Vec2i allPos[] =
	{
		Vec2i(-1, -1),
		Vec2i(-1, 0),
		Vec2i(-1, 1),
		Vec2i(0, -1),
		Vec2i(0, 1),
		Vec2i(1, -1),
		Vec2i(1, 0),
		Vec2i(1, 1)
	};

	static Vec2i allPosPrev[] =
	{
		Vec2i(-1, -1),
		Vec2i(0, -1),
		Vec2i(1, -1),
		Vec2i(-1, 0)
	};

	static Vec2i allPosNext[] =
	{
		Vec2i(-1, 1),
		Vec2i(0, 1),
		Vec2i(1, 1),
		Vec2i(1, 0)
	};

	S32 NbPSpaceCellX = _pPlayspaceInfos->m_NbCellX;
	S32 NbPSpaceCellY = _pPlayspaceInfos->m_NbCellY;
	Float fHalfCell = .5f * _pPlayspaceInfos->m_SizeVoxel;

	for (S32 s = 0; s<m_daSurfaces.GetSize(); s++)
	{
		Surface& currentSurface = m_daSurfaces[s];

		// init cells
		S32 surfaceSizeX = currentSurface.m_vMaxPos2D.x - currentSurface.m_vMinPos2D.x + 1;
		S32 surfaceSizeY = currentSurface.m_vMaxPos2D.y - currentSurface.m_vMinPos2D.y + 1;
		currentSurface.m_daCells.SetSize(surfaceSizeX * surfaceSizeY);

		// positive pass
		for (S32 y = 0; y<surfaceSizeY; y++)
			for (S32 x = 0; x < surfaceSizeX; x++)
			{
				Vec2i realPos(currentSurface.m_vMinPos2D.x + x, currentSurface.m_vMinPos2D.y + y);
				S32 idx = realPos.y * _pPlayspaceInfos->m_NbCellX + realPos.x;
				PlaySpace_CellInfos &pCell = _pPlayspaceInfos->m_pSurfaceInfosV3->m_Surfels.m_CellBoard[idx];
				CellInfo& cell = currentSurface.m_daCells[y*surfaceSizeX + x];
				Bool found = FALSE;
				PlaySpace_Surfel *pSurfel = pCell.pFirst;
				while (pSurfel)
				{
					Bool isInSurface = FALSE;
					if (pSurfel->ZoneId == currentSurface.m_iZoneId)
					{
						if (currentSurface.m_bIsCeiling)
						{
							// No virtual test cause of no real ceiling scanned
							if (pSurfel->Point.y > currentSurface.m_fWorldHeight - fHalfCell
								&& pSurfel->Point.y < currentSurface.m_fWorldHeight + fHalfCell)
								isInSurface = TRUE;
						}
						else
						{
							if((pSurfel->Flags & PlaySpace_Surfel::SURFFLAG_VIRTUAL) == 0)
							{
								if ((currentSurface.m_bIsGround || pSurfel->Point.y < currentSurface.m_fWorldHeight + 0.005f)
									&& pSurfel->Point.y > currentSurface.m_fWorldHeight - fHalfCell)
									isInSurface = TRUE;
							}
						}
					}

					if (isInSurface )
					{
						if (x == 0 || y == 0 || x == surfaceSizeX - 1 || y == surfaceSizeY - 1)
							cell.m_iDistFromBorder = 1;
						else
						{
							cell.m_iDistFromBorder = Min<U32>(cell.m_iDistFromBorder, IncreaseCellDist(currentSurface.m_daCells[(y - 1)*surfaceSizeX + x - 1].m_iDistFromBorder));
							cell.m_iDistFromBorder = Min<U32>(cell.m_iDistFromBorder, IncreaseCellDist(currentSurface.m_daCells[(y - 1)*surfaceSizeX + x].m_iDistFromBorder));
							cell.m_iDistFromBorder = Min<U32>(cell.m_iDistFromBorder, IncreaseCellDist(currentSurface.m_daCells[(y - 1)*surfaceSizeX + x + 1].m_iDistFromBorder));
							cell.m_iDistFromBorder = Min<U32>(cell.m_iDistFromBorder, IncreaseCellDist(currentSurface.m_daCells[y*surfaceSizeX + x - 1].m_iDistFromBorder));
						}
						found = TRUE;
						break;
					}

					pSurfel = pSurfel->pNext;
				}

				if (!found)
				{
					cell.m_iDistFromBorder = 0;
					cell.m_iSpaceToUp = 0;
				}
				else
				{
					// compute space
					pSurfel = pCell.pFirst;
					while (pSurfel)
					{
						Float delta = (pSurfel->Point.y - currentSurface.m_fWorldHeight);
						if( currentSurface.m_bIsCeiling )
							delta = -delta;

						if( delta > _pPlayspaceInfos->m_SizeVoxel )
						{
							cell.m_iSpaceToUp = Min<U32>(cell.m_iSpaceToUp,FLOORINT(delta / _pPlayspaceInfos->m_SizeVoxel ) + 1); // cause of ceilling case, no break possible !!!
						}

						pSurfel = pSurfel->pNext;
					}
				}
					
				// Get Local Height.
				Float LocalHeight = currentSurface.m_fWorldHeight;

				if (realPos.x >= 0 && realPos.y >= 0 && realPos.x < NbPSpaceCellX && realPos.y < NbPSpaceCellY)
				{
					S32 idx = realPos.y * NbPSpaceCellX + realPos.x;
					PlaySpace_CellInfos &pCell = _pPlayspaceInfos->m_pSurfaceInfosV3->m_Surfels.m_CellBoard[idx];
					PlaySpace_Surfel *pSurfel = pCell.pFirst;
					S32 MyZoneId = currentSurface.m_iZoneId;
			}	

				// dist from wall/void
				if (IsWall_V3(_pPlayspaceInfos, LocalHeight, realPos))			// wall
				{
//DRAW_DEBUG_SPHERE3D(currentSurface.m_vMinPos + Vec3f(x * 0.08f, 0.f, y*0.08f), COLOR_RED*0.99f, 0.04f, .displayDuration(1000.f));
					cell.m_iDistFromWall = 0;
				}
				else if (IsVoid_V3(_pPlayspaceInfos, LocalHeight, realPos))		// void
				{
//DRAW_DEBUG_SPHERE3D(currentSurface.m_vMinPos + Vec3f(x * 0.08f, 0.f, y*0.08f), COLOR_GREEN*0.99f, 0.04f, .displayDuration(1000.f));
					cell.m_iDistFromVoid = 0;
					if (IsFloor_V3(_pPlayspaceInfos, LocalHeight, realPos))
						cell.m_iDistFromFloor = 0;
				}

				// propagation
				for (S32 i = 0; i<_countof(allPosPrev); i++)
				{
					cell.m_iDistFromWall = ComputeCellDistWall_V3(_pPlayspaceInfos, currentSurface, Vec2i(x, y), allPosPrev[i]);
					cell.m_iDistFromVoid = ComputeCellDistVoid_V3(_pPlayspaceInfos, currentSurface, Vec2i(x, y), allPosPrev[i]);
					cell.m_iDistFromFloor = ComputeCellDistFloor_V3(_pPlayspaceInfos, currentSurface, Vec2i(x, y), allPosPrev[i]);
				}
			}

		// negative pass
		for (S32 y = surfaceSizeY - 1; y >= 0; y--)
			for (S32 x = surfaceSizeX - 1; x >= 0; x--)
			{
				CellInfo& cell = currentSurface.m_daCells[y*surfaceSizeX + x];
				if( x < surfaceSizeX - 1)
					cell.m_iDistFromBorder = Min<U32>(cell.m_iDistFromBorder, IncreaseCellDist(currentSurface.m_daCells[y*surfaceSizeX + x + 1].m_iDistFromBorder));
				if (y < surfaceSizeY - 1)
				{
					if (x < surfaceSizeX - 1)
						cell.m_iDistFromBorder = Min<U32>(cell.m_iDistFromBorder, IncreaseCellDist(currentSurface.m_daCells[(y + 1)*surfaceSizeX + x + 1].m_iDistFromBorder));
					cell.m_iDistFromBorder = Min<U32>(cell.m_iDistFromBorder, IncreaseCellDist(currentSurface.m_daCells[(y + 1)*surfaceSizeX + x].m_iDistFromBorder));
					if (x > 0)
						cell.m_iDistFromBorder = Min<U32>(cell.m_iDistFromBorder, IncreaseCellDist(currentSurface.m_daCells[(y + 1)*surfaceSizeX + x - 1].m_iDistFromBorder));
				}

				// propagation
				for (S32 i = 0; i<_countof(allPosNext); i++)
				{
					cell.m_iDistFromWall = ComputeCellDistWall_V3(_pPlayspaceInfos, currentSurface, Vec2i(x, y), allPosNext[i]);
					cell.m_iDistFromVoid = ComputeCellDistVoid_V3(_pPlayspaceInfos, currentSurface, Vec2i(x, y), allPosNext[i]);
					cell.m_iDistFromFloor = ComputeCellDistFloor_V3(_pPlayspaceInfos, currentSurface, Vec2i(x, y), allPosNext[i]);
				}
			}
	}
}

//-----------------------------------

U32		TopologyAnalyzer_W::ComputeCellDistWall_V3(const PlaySpaceInfos_W* _pPlayspaceInfos, const Surface& _surface, const Vec2i& _curPos, const Vec2i& _delta)
{
	Vec2i otherPos = _curPos + _delta;
	S32 surfaceSizeX = _surface.m_vMaxPos2D.x - _surface.m_vMinPos2D.x + 1;
	S32 surfaceSizeY = _surface.m_vMaxPos2D.y - _surface.m_vMinPos2D.y + 1;
	if (otherPos.x < 0 || otherPos.y < 0 || otherPos.x >= surfaceSizeX || otherPos.y >= surfaceSizeY)
	{
		Vec2i realPos(_surface.m_vMinPos2D.x + otherPos.x, _surface.m_vMinPos2D.y + otherPos.y);
		if (IsWall_V3(_pPlayspaceInfos, _surface.m_fWorldHeight, realPos))
			return Min<U32>(1, _surface.m_daCells[_curPos.y*surfaceSizeX + _curPos.x].m_iDistFromWall);

		return _surface.m_daCells[_curPos.y*surfaceSizeX + _curPos.x].m_iDistFromWall;
	}

	return Min<U32>(_surface.m_daCells[_curPos.y*surfaceSizeX + _curPos.x].m_iDistFromWall,
		IncreaseCellDist(_surface.m_daCells[otherPos.y*surfaceSizeX + otherPos.x].m_iDistFromWall));
}

//-------------------------------------

U32		TopologyAnalyzer_W::ComputeCellDistVoid_V3(const PlaySpaceInfos_W* _pPlayspaceInfos, const Surface& _surface, const Vec2i& _curPos, const Vec2i& _delta)
{
	Vec2i otherPos = _curPos + _delta;
	S32 surfaceSizeX = _surface.m_vMaxPos2D.x - _surface.m_vMinPos2D.x + 1;
	S32 surfaceSizeY = _surface.m_vMaxPos2D.y - _surface.m_vMinPos2D.y + 1;
	if (otherPos.x < 0 || otherPos.y < 0 || otherPos.x >= surfaceSizeX || otherPos.y >= surfaceSizeY)
	{
		Vec2i realPos(_surface.m_vMinPos2D.x + otherPos.x, _surface.m_vMinPos2D.y + otherPos.y);
		if (IsVoid_V3(_pPlayspaceInfos, _surface.m_fWorldHeight, realPos))
			return Min<U32>(1, _surface.m_daCells[_curPos.y*surfaceSizeX + _curPos.x].m_iDistFromVoid);

		return _surface.m_daCells[_curPos.y*surfaceSizeX + _curPos.x].m_iDistFromVoid;
	}

	return Min<U32>(_surface.m_daCells[_curPos.y*surfaceSizeX + _curPos.x].m_iDistFromVoid,
		IncreaseCellDist(_surface.m_daCells[otherPos.y*surfaceSizeX + otherPos.x].m_iDistFromVoid));
}

//-------------------------------------

U32		TopologyAnalyzer_W::ComputeCellDistFloor_V3(const PlaySpaceInfos_W* _pPlayspaceInfos, const Surface& _surface, const Vec2i& _curPos, const Vec2i& _delta)
{
	Vec2i otherPos = _curPos + _delta;
	S32 surfaceSizeX = _surface.m_vMaxPos2D.x - _surface.m_vMinPos2D.x + 1;
	S32 surfaceSizeY = _surface.m_vMaxPos2D.y - _surface.m_vMinPos2D.y + 1;
	if (otherPos.x < 0 || otherPos.y < 0 || otherPos.x >= surfaceSizeX || otherPos.y >= surfaceSizeY)
	{
		Vec2i realPos(_surface.m_vMinPos2D.x + otherPos.x, _surface.m_vMinPos2D.y + otherPos.y);
		if (IsFloor_V3(_pPlayspaceInfos, _surface.m_fWorldHeight, realPos))
			return Min<U32>(1, _surface.m_daCells[_curPos.y*surfaceSizeX + _curPos.x].m_iDistFromFloor);

		// test
		otherPos += _delta;
		realPos.Set(_surface.m_vMinPos2D.x + otherPos.x, _surface.m_vMinPos2D.y + otherPos.y);
		if (IsFloor_V3(_pPlayspaceInfos, _surface.m_fWorldHeight, realPos))
			return Min<U32>(1, _surface.m_daCells[_curPos.y*surfaceSizeX + _curPos.x].m_iDistFromFloor);

		return _surface.m_daCells[_curPos.y*surfaceSizeX + _curPos.x].m_iDistFromFloor;
	}

	return Min<U32>(_surface.m_daCells[_curPos.y*surfaceSizeX + _curPos.x].m_iDistFromFloor,
		IncreaseCellDist(_surface.m_daCells[otherPos.y*surfaceSizeX + otherPos.x].m_iDistFromFloor));
}

//-------------------------------------

Bool	TopologyAnalyzer_W::IsWall_V3(const PlaySpaceInfos_W* _pPlayspaceInfos, Float _currentHeight, const Vec2i& _pos) const
{
	S32 nbCellX = _pPlayspaceInfos->m_NbCellX;
	S32 nbCellY = _pPlayspaceInfos->m_NbCellY;
	//Float halfVoxel = .5f * _pPlayspaceInfos->m_SizeVoxel;
	if (_pos.x >= 0 && _pos.y >= 0 && _pos.x < nbCellX && _pos.y < nbCellY)
	{
		S32 idx = _pos.y * _pPlayspaceInfos->m_NbCellX + _pos.x;
		PlaySpace_CellInfos &pCell = _pPlayspaceInfos->m_pSurfaceInfosV3->m_Surfels.m_CellBoard[idx];
		PlaySpace_Surfel *pSurfel = pCell.pFirst;
		while (pSurfel)
		{
			if ((pSurfel->iDir == PlaySpace_Surfel::SURFDIR_LEFT) || (pSurfel->iDir == PlaySpace_Surfel::SURFDIR_RIGHT) ||
				(pSurfel->iDir == PlaySpace_Surfel::SURFDIR_FRONT) || (pSurfel->iDir == PlaySpace_Surfel::SURFDIR_BACK))
			{
				Float Dist = pSurfel->Point.y - _currentHeight;
				if ((Dist > 0.04f) && (Dist < 0.16f))
					// Ajouter l'orientation.
					return TRUE;
			}
			pSurfel = pSurfel->pNext;
		}
	}
	return FALSE;
}

//-------------------------------------

Bool	TopologyAnalyzer_W::IsVoid_V3(const PlaySpaceInfos_W* _pPlayspaceInfos, Float _currentHeight, const Vec2i& _pos)
{
	S32 nbCellX = _pPlayspaceInfos->m_NbCellX;
	S32 nbCellY = _pPlayspaceInfos->m_NbCellY;
	Float halfVoxel = .5f * _pPlayspaceInfos->m_SizeVoxel;
	if (_pos.x >= 0 && _pos.y >= 0 && _pos.x < nbCellX && _pos.y < nbCellY)
	{
		S32 idx = _pos.y * _pPlayspaceInfos->m_NbCellX + _pos.x;
		PlaySpace_CellInfos &pCell = _pPlayspaceInfos->m_pSurfaceInfosV3->m_Surfels.m_CellBoard[idx];
		PlaySpace_Surfel *pSurfel = pCell.pFirst;

		// case no surfel under height => not void
		if( pSurfel && pSurfel->Point.y - _currentHeight > halfVoxel)
			return FALSE;

		while (pSurfel)
		{
			Float dist = _currentHeight - pSurfel->Point.y;
			if (dist < 0.0f)
				dist = -dist;

			if (dist < halfVoxel)
				return FALSE;

			pSurfel = pSurfel->pNext;
		}
		return TRUE;
	}

	return FALSE;
}

//-------------------------------------

Bool	TopologyAnalyzer_W::IsFloor_V3(const PlaySpaceInfos_W* _pPlayspaceInfos, Float _currentHeight, const Vec2i& _pos)
{
	if (IsVoid_V3(_pPlayspaceInfos, _currentHeight, _pos))
	{
		S32 nbCellX = _pPlayspaceInfos->m_NbCellX;
		S32 nbCellY = _pPlayspaceInfos->m_NbCellY;

		if (_pos.x >= 0 && _pos.y >= 0 && _pos.x < nbCellX && _pos.y < nbCellY)
		{
			S32 idx = _pos.y * _pPlayspaceInfos->m_NbCellX + _pos.x;
			PlaySpace_CellInfos &pCell = _pPlayspaceInfos->m_pSurfaceInfosV3->m_Surfels.m_CellBoard[idx];
			PlaySpace_Surfel *pSurfel = pCell.pFirst;
			Float maxHeight = -1e8f;
			while (pSurfel)
			{
				if ((pSurfel->Point.y <= _currentHeight) && (pSurfel->Point.y > maxHeight))
					maxHeight = pSurfel->Point.y;

				pSurfel = pSurfel->pNext;
			}

			if (Abs(_pPlayspaceInfos->m_YGround - maxHeight) < 0.1f)
					return TRUE;
		}
	}
	return FALSE;
}

//-----------------------------------
// _saFace:
//     
//    2 ________ 3
//     |________|
//    1          0
Bool TopologyAnalyzer_W::RayCastOnFace(const Vec3f& _vPos, const Vec3f& _vDir, const SafeArray_Z<Vec3f, 4>& _saFace, const Vec3f& _vNormal, Vec3f& _outResult) const
{
	Segment_Z seg = Segment_Z(_vPos, _vPos + _vDir * 1000000.f);

	Vec3f vSphereCenter = (_saFace[0] + _saFace[1] + _saFace[2] + _saFace[3]) * 0.25f;
	if (!SegmentVsSphere(seg, Sphere_Z(vSphereCenter, (vSphereCenter - _saFace[0]).GetNorm())))
		return FALSE;

	Vec4f vectors[7];
	vectors[0] = _saFace[0];
	vectors[1] = _saFace[1];
	vectors[2] = _saFace[3];
	vectors[3] = _saFace[2];
	vectors[4] = _vNormal;
	vectors[5] = _vNormal;
	vectors[6] = _vNormal;

	CollisionReport_Z report;
	if (!LineVsPatch(seg, vectors, report, 1))
		return FALSE;

	_outResult = report.Inter;
	return TRUE;
}

//-----------------------------------

Bool TopologyAnalyzer_W::TopologyRayCast(const Vec3f& _vPos, const Vec3f& _vDir, TopologyRayCastResult_W& _outResult) const
{
	Bool bOneResultFound = FALSE;
	Float fSquareMinDist;

	Vec3f vPoint;
	Vec3f vNormal;
	TopologyRayCastResult_W::Type type;
	S32 iSurfaceIdx;
	S32 iCellIdx;
	S32 iWallIdx;
	S32 iColumnIdx;

	Float fHalfSizeVoxel = m_fSizeVoxel / 2.f;
	SafeArray_Z<Vec3f, 4> face;

	// Check surfaces
	for (S32 i = 0; i < m_daSurfaces.GetSize(); ++i)
	{
		const Surface& surface = m_daSurfaces[i];

		face[0] = Vec3f(surface.m_vMinPos.x - fHalfSizeVoxel, surface.m_vMinPos.y, surface.m_vMinPos.z - fHalfSizeVoxel);
		face[1] = Vec3f(surface.m_vMaxPos.x + fHalfSizeVoxel, surface.m_vMinPos.y, surface.m_vMinPos.z - fHalfSizeVoxel);
		face[2] = Vec3f(surface.m_vMaxPos.x + fHalfSizeVoxel, surface.m_vMinPos.y, surface.m_vMaxPos.z + fHalfSizeVoxel);
		face[3] = Vec3f(surface.m_vMinPos.x - fHalfSizeVoxel, surface.m_vMinPos.y, surface.m_vMaxPos.z + fHalfSizeVoxel);
		
		Vec3f p;
		Vec3f vNorm = VEC3F_UP;
		if (!RayCastOnFace(_vPos, _vDir, face, vNorm, p))
		{
			vNorm = VEC3F_DOWN;
			if (!RayCastOnFace(_vPos, _vDir, face, vNorm, p))
				continue;
		}

		const CellInfo* pCell = GetCellInSurface(p, surface, m_fSizeVoxel);
		if (pCell == NULL)
			continue;

		if (pCell->m_iDistFromVoid == 0)
			continue;

		Float fSquareDist = (_vPos - p).GetNorm2();
		if (!bOneResultFound || fSquareDist < fSquareMinDist)
		{
			bOneResultFound = TRUE;
			fSquareMinDist = fSquareDist;
			vPoint = p;
			vNormal = vNorm;
			type = TopologyRayCastResult_W::SURFACE;
			iSurfaceIdx = i;
			iCellIdx = pCell - surface.m_daCells.GetArrayPtr();
		}
	}

	// Check walls
	for (S32 i = 0; i < m_daWalls.GetSize(); ++i)
	{
		const Wall& wall = m_daWalls[i];

		face[0] = wall.m_vBaseStart;
		face[1] = wall.m_vBaseStart + wall.m_vTangent * wall.m_fWidth;
		face[2] = wall.m_vBaseStart + wall.m_vTangent * wall.m_fWidth + wall.m_vUp * wall.m_fHeight;
		face[3] = wall.m_vBaseStart + wall.m_vUp * wall.m_fHeight;

		Vec3f p;
		if (!RayCastOnFace(_vPos, _vDir, face, wall.m_vNormal, p))
			continue;

		Float fColumnsize = wall.m_fWidth / wall.m_daColumns.GetSize();
		S32 iCIdx = (wall.m_vTangent * (p - wall.m_vBaseStart)) / fColumnsize;

		if (iCIdx < 0 || iCIdx >= wall.m_daColumns.GetSize())
			continue;

		if (p.y < wall.m_daColumns[iCIdx].m_fMin || p.y > wall.m_daColumns[iCIdx].m_fMax)
			continue;

		Float fSquareDist = (_vPos - p).GetNorm2();
		if (!bOneResultFound || fSquareDist < fSquareMinDist)
		{
			bOneResultFound = TRUE;
			fSquareMinDist = fSquareDist;
			vPoint = p;
			vNormal = wall.m_vNormal;
			type = TopologyRayCastResult_W::WALL;
			iWallIdx = i;
			iColumnIdx = iCIdx;
		}
	}

	if (!bOneResultFound)
		return FALSE;
	
	_outResult.m_vPos = vPoint;
	_outResult.m_vNormal = vNormal;
	_outResult.m_type = type;

	if (type == TopologyRayCastResult_W::SURFACE)
	{
		_outResult.m_iSufaceIdx = iSurfaceIdx;
		_outResult.m_iCellIdx = iCellIdx;
	}
	else
	{
		_outResult.m_iWallIdx = iWallIdx;
		_outResult.m_iColumnIdx = iColumnIdx;
	}

	return TRUE;
}

//-----------------------------------
// _saFace:
//     
//    2 ________ 3             y
//     |________|              ^
//    1          0             |
//                        x <--
void TopologyAnalyzer_W::GetNearestPointOnFace(const Vec3f& _vRefPoint, const SafeArray_Z<Vec3f, 4>& _saFace, Vec3f& _vOutNearestPoint) const
{
	Vec3f vX = _saFace[1] - _saFace[0];
	Vec3f vY = _saFace[3] - _saFace[0];

	Float fXDotX = vX * vX;
	Float fYDotY = vY * vY;

	Float fXDotPoint = vX * (_vRefPoint - _saFace[0]);
	Float fYDotPoint = vY * (_vRefPoint - _saFace[0]);

	Vec3f vNearestPoint;
	if (fXDotPoint < 0.f)
	{
		if (fYDotPoint < 0.f)
			_vOutNearestPoint = _saFace[0];
		else if (fYDotPoint > fYDotY)
			_vOutNearestPoint = _saFace[3];
		else
			_vOutNearestPoint = _saFace[0] + vY * (fYDotPoint / fYDotY);
	}
	else if (fXDotPoint > fXDotX)
	{
		if (fYDotPoint < 0.f)
			_vOutNearestPoint = _saFace[1];
		else if (fYDotPoint > fYDotY)
			_vOutNearestPoint = _saFace[2];
		else
			_vOutNearestPoint = _saFace[1] + vY * (fYDotPoint / fYDotY);
	}
	else
	{
		if (fYDotPoint < 0.f)
			_vOutNearestPoint = _saFace[0] + vX * (fXDotPoint / fXDotX);
		else if (fYDotPoint > fYDotY)
			_vOutNearestPoint = _saFace[3] + vX * (fXDotPoint / fXDotX);
		else
			_vOutNearestPoint = _saFace[0] + vX * (fXDotPoint / fXDotX) + vY * (fYDotPoint / fYDotY);
	}
}

//-----------------------------------
// _saFace:
//     
//    2 ________ 3
//     |________|
//    1          0
Float TopologyAnalyzer_W::GetDistPointVsFace(const Vec3f& _vRefPoint, const SafeArray_Z<Vec3f, 4>& _saFace, Vec3f* _vOutNearestPoint) const
{
	Vec3f vNearestPoint;
	GetNearestPointOnFace(_vRefPoint, _saFace, vNearestPoint);

	if (_vOutNearestPoint != NULL)
		*_vOutNearestPoint = vNearestPoint;

	return (vNearestPoint - _vRefPoint).GetNorm();
}

//-----------------------------------

void TopologyAnalyzer_W::GetNearestPointOnSurface(const Vec3f& _vRefPoint, const Surface& _surface, Vec3f& _vOutNearestPoint) const
{
	Float fHalfSizeVoxel = m_fSizeVoxel / 2.f;

	SafeArray_Z<Vec3f, 4> face;
	face[0] = Vec3f(_surface.m_vMinPos.x - fHalfSizeVoxel, _surface.m_vMinPos.y, _surface.m_vMinPos.z - fHalfSizeVoxel);
	face[1] = Vec3f(_surface.m_vMaxPos.x + fHalfSizeVoxel, _surface.m_vMinPos.y, _surface.m_vMinPos.z - fHalfSizeVoxel);
	face[2] = Vec3f(_surface.m_vMaxPos.x + fHalfSizeVoxel, _surface.m_vMinPos.y, _surface.m_vMaxPos.z + fHalfSizeVoxel);
	face[3] = Vec3f(_surface.m_vMinPos.x - fHalfSizeVoxel, _surface.m_vMinPos.y, _surface.m_vMaxPos.z + fHalfSizeVoxel);

	GetNearestPointOnFace(_vRefPoint, face, _vOutNearestPoint);
}

//-----------------------------------

Float TopologyAnalyzer_W::GetDistPointVsSurface(const Vec3f& _vRefPoint, const Surface& _surface, Vec3f* _vOutNearestPoint) const
{
	Vec3f vNearestPoint;
	GetNearestPointOnSurface(_vRefPoint, _surface, vNearestPoint);

	if (_vOutNearestPoint != NULL)
		*_vOutNearestPoint = vNearestPoint;

	return (vNearestPoint - _vRefPoint).GetNorm();
}

//-----------------------------------

void TopologyAnalyzer_W::GetNearestPointOnWall(const Vec3f& _vRefPoint, const Wall& _wall, Vec3f& _vOutNearestPoint) const
{
	SafeArray_Z<Vec3f, 4> face;
	face[0] = _wall.m_vBaseStart;
	face[1] = _wall.m_vBaseStart + _wall.m_vTangent * _wall.m_fWidth;
	face[2] = _wall.m_vBaseStart + _wall.m_vTangent * _wall.m_fWidth + _wall.m_vUp * _wall.m_fHeight;
	face[3] = _wall.m_vBaseStart + _wall.m_vUp * _wall.m_fHeight;

	return GetNearestPointOnFace(_vRefPoint, face, _vOutNearestPoint);
}

//----------------------------------

Float TopologyAnalyzer_W::GetDistPointVsWall(const Vec3f& _vRefPoint, const Wall& _wall, Vec3f* _vOutNearestPoint) const
{
	Vec3f vNearestPoint;
	GetNearestPointOnWall(_vRefPoint, _wall, vNearestPoint);

	if (_vOutNearestPoint != NULL)
		*_vOutNearestPoint = vNearestPoint;

	return (vNearestPoint - _vRefPoint).GetNorm();
}

//-----------------------------------

Vec3f   TopologyAnalyzer_W::GetCellPosition(const Surface& _surface, S32 _iCell) const
{
    const S32 surfaceSizeX = _surface.m_vMaxPos2D.x - _surface.m_vMinPos2D.x + 1;
    const S32 surfaceSizeY = _surface.m_vMaxPos2D.y - _surface.m_vMinPos2D.y + 1;

    const S32 y = _iCell / surfaceSizeX;
    const S32 x = _iCell % surfaceSizeX;

    Vec3f pos;
    pos.x = _surface.m_vMinPos.x + x * m_fSizeVoxel;
	pos.y = _surface.m_vMinPos.y;
	pos.z = _surface.m_vMinPos.z + y * m_fSizeVoxel; 

	if((PlaySpaceInfos_W::g_TypeScan & PlaySpaceInfos_W::PSI_SCAN_NEW) == 0) // old_scan
	{
		pos.x += m_fSizeVoxel * .5f;
		pos.z +=m_fSizeVoxel * .5f;
	}

    return pos;
}

//-----------------------------------

const TopologyAnalyzer_W::Surface*	TopologyAnalyzer_W::GetSurfaceFromPos(const Vec3f& _pos) const
{
	for(S32 i=0; i<m_daSurfaces.GetSize(); i++)
	{
		const CellInfo* pCell = GetCellInSurface(_pos,m_daSurfaces[i],m_fSizeVoxel);

		if( pCell && pCell->IsValid() )
		{
			return &(m_daSurfaces[i]);
		}
	}

	return NULL;
}

const TopologyAnalyzer_W::Surface*	TopologyAnalyzer_W::GetHigherSurfaceInBox(const Vec3f& _pos, const Vec3f& _size) const
{
	const TopologyAnalyzer_W::Surface* pResult = NULL;
	for(S32 i=0; i<m_daSurfaces.GetSize(); i++)
	{
		const TopologyAnalyzer_W::Surface& surface = m_daSurfaces[i];

		if( pResult && pResult->m_fWorldHeight > surface.m_fWorldHeight )
			continue;

		if( Abs(surface.m_fWorldHeight - _pos.y) > _size.y * .5f )
			continue;

		if(surface.m_vMinPos.x > _pos.x  || surface.m_vMaxPos.x < _pos.x) 
			continue;

		if(surface.m_vMinPos.z > _pos.z  || surface.m_vMaxPos.z < _pos.z) 
			continue;

		pResult = &surface;
	}

	return pResult;
}

//-----------------------------------

S32		TopologyAnalyzer_W::GetSurfaceIndexFromZoneId(S32 _iZoneId) const
{
	for(S32 i=0; i<m_daSurfaces.GetSize(); i++)
	{
		if( m_daSurfaces[i].m_iZoneId == _iZoneId )
		{
			return i;
		}
	}

	return -1;
}

//-----------------------------------

void TopologyAnalyzer_W::GetAllRectanglePos(Float _minWidth, Float _minLength, const Surface& _surface, Vec3fDA& _outPos, Vec3fDA& _outLengthDirection) const
{
    if (_minWidth > _minLength)
    {
        GetAllRectanglePos(_minLength, _minWidth, _surface, _outPos, _outLengthDirection);
        for (S32 i = 0; i < _outLengthDirection.GetSize(); ++i)
            _outLengthDirection[i] = VEC3F_DOWN ^ _outLengthDirection[i];
        return;
    }

    U32 widthMinSize = FLOOR(_minWidth * 0.5f / m_fSizeVoxel) + 1;

    QuatDA rots;
    rots.Add(Quat(Vec3f(1, 0, 0), Vec3f(1, 0, 0)));
    rots.Add(Quat(Vec3f(1, 0, 0), Vec3f(0, 0, 1)));
    rots.Add(Quat(Vec3f(1, 0, 0), Vec3f(-1, 0, 0)));
    rots.Add(Quat(Vec3f(1, 0, 0), Vec3f(0, 0, -1)));

    for(S32 c = 0; c < _surface.m_daCells.GetSize(); ++c)
	{
	    if (_surface.m_daCells[c].m_iDistFromBorder >= widthMinSize)
		{
            Vec3f cellPos = GetCellPosition(_surface, c);

            for (S32 i = 0; i < rots.GetSize(); ++i)
            {
                Vec3f topLeft = cellPos + rots[i] * Vec3f(-_minWidth / 2.f, 0.f, -_minLength / 2.f);
                Vec3f topRight = cellPos + rots[i] * Vec3f(-_minWidth / 2.f, 0.f, _minLength / 2.f);
                Vec3f bottomLeft = cellPos + rots[i] * Vec3f(_minWidth / 2.f, 0.f, -_minLength / 2.f);
                Vec3f bottomRight = cellPos + rots[i] * Vec3f(_minWidth / 2.f, 0.f, _minLength / 2.f);

				if (RectangleIsOk(topLeft, topRight, bottomLeft, bottomRight, _surface, m_fSizeVoxel,TOPOLOGY_REQUEST_VALID))
                {
                    _outPos.Add(cellPos);
                    _outLengthDirection.Add(rots[i] * VEC3F_FRONT);
                }
            }
        }
    }
}

//-----------------------------------

void TopologyAnalyzer_W::GetGroupsAwayFromWalls(Float _surfaceMinHeight, Float _surfaceMaxHeight, S32DA& _outGroups) const
{
    for (S32 g = 0; g < m_daSurfaceGroups.GetSize(); g++)
	{
		const DynArray_Z<const Surface*>& group = m_daSurfaceGroups[g];
        Bool groupAgainWall = FALSE;
        Bool surfacesOk = TRUE;

		for (S32 s = 0; s < group.GetSize(); s++)
		{
			const Surface& surface = *group[s];

			if (surface.m_fHeightFromGround < _surfaceMinHeight || surface.m_fHeightFromGround > _surfaceMaxHeight)
            {
                surfacesOk = FALSE;
                break;
            }

	        // Check if the surface is against a wall
			for (S32 c = 0; c < surface.m_daCells.GetSize(); c++)
			{
				if (surface.m_daCells[c].m_iDistFromWall != 1)
                    continue;

				DynArray_Z<Vec2i> cellNeighbors;
				GetCellNeighbors(surface, c, cellNeighbors);

                Bool cellOk = FALSE;
				for (S32 cn = 0; cn < cellNeighbors.GetSize(); cn++)
				{
					for (S32 s2 = 0; s2 < group.GetSize(); s2++)
					{
						if (s2 == s)
                            continue;

						if (GetCellInSurface(cellNeighbors[cn], *group[s2]))
                        {
                            cellOk = TRUE;
                            break;
					    }
                    }

				    if (cellOk)
						break;
				}

                if (!cellOk)
                {
                    groupAgainWall = TRUE;
                    break;
                }
		    } // for cells in surface

            if (groupAgainWall)
                break;
		} // for surface in group

        if (!groupAgainWall && surfacesOk)
            _outGroups.Add(g);
	} // for each group
}

//-----------------------------------

Bool TopologyAnalyzer_W::SpaceOnWallIsFree(S32 _firstColumn, S32 _lastColumn, Float _bottomY, Float _topY, S32 _iDepth, Bool _bAllowPartially, const Wall& _wall)
{
	if(_firstColumn<0 || _firstColumn>=_wall.m_daColumns.GetSize() || _lastColumn<0 || _lastColumn>=_wall.m_daColumns.GetSize())
	{
		return FALSE;
	}

	S32 limit = (_lastColumn - _firstColumn) * FLOORINT((_topY - _bottomY)/0.08f) / 20; // 10% of object in front of
	S32 count = 0;
	
	for (S32 i = _firstColumn; i <= _lastColumn; ++i)
	{
		static const Float Eps = 0.001f; // Changement due to precision error (Float_Eps => Eps)

		Bool bColumnOk = FALSE;
		const TopologyAnalyzer_W::WallColumnInfo& column = _wall.m_daColumns[i];
		if (column.m_bIsValid && column.m_fMin <= _bottomY + Eps && column.m_fMax >= _topY - Eps)
		{
			bColumnOk = TRUE;
			S32 p0 = FLOORINT((_bottomY - column.m_fMin + Eps)/column.m_fPartHeight);
			S32 p1 = FLOORINT((_topY - column.m_fMin - Eps)/column.m_fPartHeight);
			for(S32 p=p0; p<=p1; p++)
			{
				if( (p < column.m_daSpaceInFrontOf.GetSize()) &&
					(column.m_daSpaceInFrontOf[p] < _iDepth) )
				{
					count++;
					if( count >= limit )
					{
						bColumnOk = FALSE;
						break;
					}
				}
			}
		}

		if( bColumnOk )
		{
			if( _bAllowPartially )
				return TRUE; // one column ok, partially ok
		}
		else
		{
			if( !_bAllowPartially )
				return FALSE; // one column not ok, not partially not ok
		}
	}
		
	if( _bAllowPartially )
		return FALSE; // no column validated partially

	return TRUE; // all colum validated
}

//----------------------------------

Bool TopologyAnalyzer_W::SpaceOnWallIsFreeFloat(Float _fLeftOffset, Float _fRightOffset, Float _fBottomY, Float _fTopY, S32 _iDepth, Bool _bAllowPartially, const Wall& _wall) const
{
	Float fColumnsize = _wall.m_fWidth / _wall.m_daColumns.GetSize();
	return SpaceOnWallIsFree((S32)(_fLeftOffset / fColumnsize), (S32)(_fRightOffset / fColumnsize), _fBottomY, _fTopY, _iDepth, _bAllowPartially, _wall);
}

//-----------------------------------

const TopologyAnalyzer_W::CellInfo* TopologyAnalyzer_W::GetCellInSurface(const Vec3f& _globalPos, const Surface& _surface, Float _fSizeVoxel)
{
	Vec2i pos;
	
	if((PlaySpaceInfos_W::g_TypeScan & PlaySpaceInfos_W::PSI_SCAN_NEW)) // new_scan
	{
		pos.x = ROUNDINT((_globalPos.x - _surface.m_vMinPos.x) / _fSizeVoxel);
		pos.y = ROUNDINT((_globalPos.z - _surface.m_vMinPos.z) / _fSizeVoxel);	
	}
	else
	{
		pos.x = FLOORINT((_globalPos.x - _surface.m_vMinPos.x) / _fSizeVoxel);
		pos.y = FLOORINT((_globalPos.z - _surface.m_vMinPos.z) / _fSizeVoxel);	
	}

	// nouveau test entier qui lui ne foire pas...
	S32 surfaceSizeX = _surface.m_vMaxPos2D.x - _surface.m_vMinPos2D.x + 1;
	S32 surfaceSizeY = _surface.m_vMaxPos2D.y - _surface.m_vMinPos2D.y + 1;
	if (pos.x<0) return NULL;
	if (pos.x>=surfaceSizeX) return NULL;
	if (pos.y<0) return NULL;
	if (pos.y>=surfaceSizeY) return NULL;
	
	return &(_surface.m_daCells[pos.y * surfaceSizeX + pos.x]);
}

//-----------------------------------

const TopologyAnalyzer_W::CellInfo*	TopologyAnalyzer_W::GetCellInSurface(const Vec2i& _globalPos, const Surface& _surface)
{
	S32 surfaceSizeX = _surface.m_vMaxPos2D.x - _surface.m_vMinPos2D.x + 1;
	S32 surfaceSizeY = _surface.m_vMaxPos2D.y - _surface.m_vMinPos2D.y + 1;

	if( _globalPos.x >= _surface.m_vMinPos2D.x
		&& _globalPos.y >= _surface.m_vMinPos2D.y
		&& _globalPos.x <= _surface.m_vMaxPos2D.x
		&& _globalPos.y <= _surface.m_vMaxPos2D.y )
	{
		Vec2i localPos = _globalPos - _surface.m_vMinPos2D;
		return &(_surface.m_daCells[localPos.y * surfaceSizeX + localPos.x]);
	}

	return NULL;
}

//-----------------------------------

void    TopologyAnalyzer_W::AddAllNeighborsRec(DynArray_Z<const Surface*>& group, const Surface* _surface)
{
	for (S32 n = 0; n < _surface->m_daNeighbors.GetSize(); n++)
	{
		const Surface* neighbor = _surface->m_daNeighbors[n];

		if (!neighbor->m_bIsGround && !neighbor->m_bIsCeiling && group.Contains(neighbor) == -1)
		{
			group.Add(neighbor);
			AddAllNeighborsRec(group, neighbor);
		}
	}
}

//-----------------------------------

void	TopologyAnalyzer_W::GetCellNeighbors(const Surface& _surface, S32 _c, DynArray_Z<Vec2i>& _outNeighbors)
{
	S32 surfaceSizeX = _surface.m_vMaxPos2D.x - _surface.m_vMinPos2D.x + 1;
	S32 surfaceSizeY = _surface.m_vMaxPos2D.y - _surface.m_vMinPos2D.y + 1;

	const DynArray_Z<CellInfo>&	cells = _surface.m_daCells;

	ASSERT_Z(_c >= 0 && _c < cells.GetSize());
	ASSERT_Z(cells[_c].m_iDistFromWall == 1);

	S32 x = _c % surfaceSizeX;
	S32 y = _c / surfaceSizeX;

	Vec2i realPos;
	realPos.x = x + _surface.m_vMinPos2D.x;
	realPos.y = y + _surface.m_vMinPos2D.y;

	_outNeighbors.Add(Vec2i(realPos.x - 1, realPos.y - 1));
	_outNeighbors.Add(Vec2i(realPos.x - 1, realPos.y    ));
	_outNeighbors.Add(Vec2i(realPos.x - 1, realPos.y + 1));
	_outNeighbors.Add(Vec2i(realPos.x    , realPos.y - 1));
	_outNeighbors.Add(Vec2i(realPos.x    , realPos.y + 1));
	_outNeighbors.Add(Vec2i(realPos.x + 1, realPos.y - 1));
	_outNeighbors.Add(Vec2i(realPos.x + 1, realPos.y    ));
	_outNeighbors.Add(Vec2i(realPos.x + 1, realPos.y + 1));
}

//-----------------------------------

Vec3f	TopologyAnalyzer_W::GetDominantDirection(const Vec3fDA& _rectangle, Float& _outLength)
{
	static const S32 NbPos = 512;

	// Get NbPos couple of points with the max distance
	SafeArray_Z<Vec3f, 512> saFirstPos;
	SafeArray_Z<Vec3f, NbPos> saSecondPos;
	SafeArray_Z<Float, NbPos> saMaxDists;

	saMaxDists[0] = 0;

	for (S32 i = 0; i < _rectangle.GetSize(); i++)
	{
		for (S32 j = i + 1; j < _rectangle.GetSize(); j++)
		{
			Float dist = Vec3_Dot(_rectangle[i] - _rectangle[j], _rectangle[i] - _rectangle[j]);

			for (S32 k = 0; k < saMaxDists.GetSize(); k++)
			{
				if (dist > saMaxDists[k])
				{
					for (S32 l = saMaxDists.GetSize() - 1; l > k; l--)
					{
						saFirstPos[l] = saFirstPos[l - 1];
						saSecondPos[l] = saSecondPos[l - 1];
						saMaxDists[l] = saMaxDists[l - 1];
					}

					saFirstPos[k] = _rectangle[i];
					saSecondPos[k] = _rectangle[j];
					saMaxDists[k] = dist;

					break;
				}
			}
		}
	}

	// Sort all points
	DynArray_Z<Vec3f> daAllPos;

	for (S32 i = 0; i < saMaxDists.GetSize(); i++)
	{
		if (saMaxDists[i] == 0)
			break;

		daAllPos.Add(saFirstPos[i]);
		daAllPos.Add(saSecondPos[i]);
	}

	_outLength = Sqrt(Vec3_Dot(saSecondPos[0] - saFirstPos[0], saSecondPos[0] - saFirstPos[0]));

	Vec3f infPoint = saFirstPos[0] + (saSecondPos[0] - saFirstPos[0]) * 20000;

	// Selection sort
	for (S32 i = 0; i < daAllPos.GetSize() - 1; i++)
	{
		S32 min = i;
		Float minDist = Vec3_Dot(infPoint - daAllPos[min], infPoint - daAllPos[min]);

		for (S32 j = i + 1; j < daAllPos.GetSize(); j++)
		{
			Float dist = Vec3_Dot(infPoint - daAllPos[j], infPoint - daAllPos[j]);
			if (dist < minDist)
			{
				min = j;
				minDist = dist;
			}
		}

		Vec3f tmp = daAllPos[i];
		daAllPos[i] = daAllPos[min];
		daAllPos[min] = tmp;
	}

	// Get average
	Vec3f firstPos = VEC3F_NULL;
	Vec3f secondPos = VEC3F_NULL;

	S32 nbCouple = daAllPos.GetSize() / 2;
	for (S32 i = 0; i < nbCouple; i++)
	{
		firstPos += daAllPos[i];
		secondPos += daAllPos[nbCouple + i];
	}

	//ASSERT_Z(nbCouple != 0);
	if (nbCouple != 0)
	{
		firstPos /= nbCouple;
		secondPos /= nbCouple;
	}

	return secondPos - firstPos;
}

//-----------------------------------

Float    TopologyAnalyzer_W::GetSurfaceArea(const Surface& _surface, Float _fSizeVoxel)
{
    Float area = 0;
    for (S32 c = 0; c < _surface.m_daCells.GetSize(); ++c)
    {
        if (_surface.m_daCells[c].m_iDistFromVoid != 0 && _surface.m_daCells[c].m_iDistFromWall != 0)
            area += _fSizeVoxel * _fSizeVoxel;
    }
    
    return area;
}

//-----------------------------------

Bool	TopologyAnalyzer_W::CellIsOk(const Vec3f& _pos, const Surface& _surface,Float _fSizeVoxel, U32 _request, U32 _limit /*= 0*/)
{
	const CellInfo* pCell = GetCellInSurface(_pos, _surface, _fSizeVoxel);

	// INSIDE / OUTSIDE
	if (!pCell )
	{
		if( (_request & TOPOLOGY_REQUEST_OUTSIDE) != 0 )
			return TRUE;

		return FALSE;
	}

	if( (_request & TOPOLOGY_REQUEST_OUTSIDE) != 0 )
			return FALSE;

	if( (_request & TOPOLOGY_REQUEST_VALID) != 0 && !pCell->IsValid())
		return FALSE;

	if( (_request & TOPOLOGY_REQUEST_WALL) != 0 && pCell->m_iDistFromWall != 0)
		return FALSE;

	if( (_request & TOPOLOGY_REQUEST_VOID) != 0 && pCell->m_iDistFromVoid != 0)
		return FALSE;

	if( (_request & TOPOLOGY_REQUEST_SPACE) != 0 && pCell->m_iSpaceToUp < _limit)
		return FALSE;

    return TRUE;
}

//-----------------------------------

Bool	TopologyAnalyzer_W::LineIsOk(const Vec3f& _first, const Vec3f& _last, const Surface& _surface, Float _fSizeVoxel, U32 _request, U32 _limit /*= 0*/)
{
	Vec3f delta = _last - _first;
	Float length = delta.GetNorm();
	if( length < Float_Eps )
		return TRUE;

	S32 testCount = FLOORINT(length / _fSizeVoxel) + 1;
	delta /= (Float)testCount;
	for(S32 i=0; i<=testCount; i++)
	{
		Vec3f cell = _first + delta * i;
		if (CellIsOk(cell, _surface, _fSizeVoxel, _request,_limit))
		{
			if( (_request & TOPOLOGY_REQUEST_ONLYONE) != 0 ) // ONLYONE
				return TRUE; // stop at first good cell
		}
		else
		{
			if( (_request & TOPOLOGY_REQUEST_ONLYONE) == 0 ) // ALL
				return FALSE; // stop at first bad cell
		}
	}

	if( (_request & TOPOLOGY_REQUEST_ONLYONE) != 0 ) 
		return FALSE;

	return TRUE;
}

//-----------------------------------

Bool TopologyAnalyzer_W::RectangleIsOk(const Vec3f& _topLeft, const Vec3f& _topRight, const Vec3f& _bottomLeft, const Vec3f& _bottomRight, const Surface& _surface, Float _fSizeVoxel, U32 _request, U32 _limit /*= 0*/)
{
#if 1 // OPTIMIZED
	// deltaX
	Vec3f deltaX = _topRight - _topLeft;
	Float lengthX = deltaX.GetNorm();
	if (lengthX < Float_Eps)
		return TRUE;

	// deltaY
	Vec3f deltaY = _bottomLeft - _topLeft;
	Float lengthY = deltaY.GetNorm();
	if (lengthY < Float_Eps)
		return TRUE;

	// early test to fast skip function
	if ((_request & TOPOLOGY_REQUEST_ONLYONE) != 0) // ONLYONE
	{
		if (CellIsOk(_topLeft, _surface, _fSizeVoxel, _request, _limit)
			|| CellIsOk(_topRight, _surface, _fSizeVoxel, _request, _limit)
			|| CellIsOk(_bottomLeft, _surface, _fSizeVoxel, _request, _limit)
			|| CellIsOk(_bottomRight, _surface, _fSizeVoxel, _request, _limit))
			return TRUE;
	}
	else
	{
		if (!CellIsOk(_topLeft, _surface, _fSizeVoxel, _request, _limit)
			|| !CellIsOk(_topRight, _surface, _fSizeVoxel, _request, _limit)
			|| !CellIsOk(_bottomLeft, _surface, _fSizeVoxel, _request, _limit)
			|| !CellIsOk(_bottomRight, _surface, _fSizeVoxel, _request, _limit))
			return FALSE;
	}

	// counts
	S32 testCountX = FLOORINT(lengthX / _fSizeVoxel) + 1;
	deltaX /= (Float)testCountX;
	S32 testCountY = FLOORINT(lengthY / _fSizeVoxel) + 1;
	deltaY /= (Float)testCountY;
	
	Vec3f lineStart = _topLeft;
	for(S32 y=0; y<= testCountY; y++)
	{
		Vec3f cellPos = lineStart;
		for (S32 x = 0; x <= testCountX; x++)
		{
			if (CellIsOk(cellPos, _surface, _fSizeVoxel, _request, _limit))
			{
				if ((_request & TOPOLOGY_REQUEST_ONLYONE) != 0) // ONLYONE
					return TRUE; // stop at first good cell
			}
			else
			{
				if ((_request & TOPOLOGY_REQUEST_ONLYONE) == 0) // ALL
					return FALSE; // stop at first bad cell
			}

			cellPos += deltaX;
		}
		lineStart += deltaY;
	}

	if( (_request & TOPOLOGY_REQUEST_ONLYONE) != 0 )
		return FALSE;

	return TRUE;
#else
	Vec3f delta = _bottomLeft - _topLeft;
	Float length = delta.GetNorm();
	if( length < Float_Eps )
		return TRUE;

	S32 testCount = FLOORINT(length / _fSizeVoxel) + 1;
	delta /= (Float)testCount;
	
	
	for(S32 i=0; i<=testCount; i++)
	{
		Vec3f first = _topLeft + delta * i;
		Vec3f last = _topRight + delta * i;
		if (LineIsOk(first,last, _surface, _fSizeVoxel, _request,_limit))
		{
			if( (_request & TOPOLOGY_REQUEST_ONLYONE) != 0 ) // ONLYONE
				return TRUE; // stop at first good cell
		}
		else
		{
			if( (_request & TOPOLOGY_REQUEST_ONLYONE) == 0 ) // ALL
				return FALSE; // stop at first bad cell
		}
	}

	if( (_request & TOPOLOGY_REQUEST_ONLYONE) != 0 )
		return FALSE;

	return TRUE;
#endif

}

//-----------------------------------

Bool TopologyAnalyzer_W::RectangleIsOkOnFloor(const Vec3f& _topLeft, const Vec3f& _topRight, const Vec3f& _bottomLeft, const Vec3f& _bottomRight, U32 _request, U32 _limit /*= 0*/) const
{
	for (S32 i = 0; i < m_daSurfaces.GetSize(); ++i)
	{
		if (m_daSurfaces[i].m_bIsGround && RectangleIsOk(_topLeft, _topRight, _bottomLeft, _bottomRight, m_daSurfaces[i], m_fSizeVoxel, _request,_limit))
			return TRUE;
	}
	
	return FALSE;
}

//-----------------------------------

void	TopologyAnalyzer_W::GetAllPosOnWall(Float _heightMin, Float _heightSizeMin, Float _widthMin, Float _fDepth, Vec3fDA& _outPos, Vec3fDA& _outNormal, Bool _bAllowVirtualWall)
{
	S32 iDepth = FLOORINT(_fDepth / m_fSizeVoxel);
	for(S32 w=0; w<m_daWalls.GetSize(); w++)
	{
		Wall& wall = m_daWalls[w];
		if (wall.m_bIsVirtual && !_bAllowVirtualWall)
			continue;
		Float heightMin = Max(wall.m_vBaseStart.y,m_YGround + _heightMin);
		Float heightMax = heightMin + wall.m_fHeight;
		if (heightMax > heightMin && (heightMax - heightMin) > _heightSizeMin)
		{
			Vec3f deltaDir = wall.m_vTangent;
			Float width = wall.m_fWidth;
			if( width > _widthMin )
			{
				Float columnDelta = width / (wall.m_daColumns.GetSize() - 1);
				S32 count = 0;
				Float fMin = heightMin;
				Float fMax = 1e7f;
				Vec3fDA validPos;
				for(S32 i=0; i<wall.m_daColumns.GetSize();i++)
				{
					WallColumnInfo& info = wall.m_daColumns[i];
					fMin = Max(fMin,info.m_fMin);
					fMax = Min(fMax,info.m_fMax);

					if( !info.m_bIsValid )
					{
						count = 0;
						continue;
					}

					// space check
					S32 p0 = FLOORINT((fMin - info.m_fMin)/info.m_fPartHeight);
					S32 p1 = FLOORINT((fMax - info.m_fMin)/info.m_fPartHeight);
					Bool bGoToNext = FALSE;
					for(S32 p=p0; p<=p1; p++)
					{
						if( (info.m_daSpaceInFrontOf.GetSize() > p) &&
							(info.m_daSpaceInFrontOf[p] < iDepth) )
						{
							bGoToNext = TRUE;
							count = 0;
							break;
						}
					}

					if( !bGoToNext )
					{
						if( (fMax - fMin) > _heightSizeMin )
						{
							count++;
							if(count * columnDelta >= _widthMin - Float_Eps )
							{
								Vec3f middle = wall.m_vBaseStart + (i * columnDelta - _widthMin * .5f + .5f * columnDelta ) * deltaDir;
								middle.y = fMin + _heightSizeMin * .5f;
								_outPos.Add(middle);
								_outNormal.Add(wall.m_vNormal);
								count--;
							}
						}
						else
						{
							if( (info.m_fMax - info.m_fMin) > _heightSizeMin )
							{
								count = 1;
								fMin = Max(info.m_fMin,heightMin);
								fMax = info.m_fMax;
							}
							else
							{
								count = 0;
								fMin = heightMin;
								fMax = 1e7f;
							}
						}
					}
				}
			}
		}
	}
}

//-----------------------------------

void	TopologyAnalyzer_W::GetAllPosOnWallNearFloor(Float _heightMin, Float _widthMin, Float _floorDist, Float _fDepth, Vec3fDA& _outPos, Vec3fDA& _outNormal) const
{
	S32 minFloorDist = FLOOR(_floorDist/m_fSizeVoxel) + 1;
	S32 iDepth = FLOORINT(_fDepth / m_fSizeVoxel);

	for(S32 w=0; w<m_daWalls.GetSize(); w++)
	{
		const Wall& wall = m_daWalls[w];
		Float heightMin = wall.m_vBaseStart.y;
		Float heightMax = heightMin + wall.m_fHeight;

		if (heightMin <= m_YGround + 0.08f && heightMax > m_YGround + _heightMin)
		{
			Vec3f deltaDir = wall.m_vTangent;
			Float width = wall.m_fWidth;
			if( width > _widthMin )
			{
				Float columnDelta = width / (wall.m_daColumns.GetSize() - 1);
				S32 count = 0;
				Float fMin = heightMin;
				Float fMax = heightMax;
				Vec3fDA validPos;
				for(S32 i=0; i<wall.m_daColumns.GetSize();i++)
				{
					const WallColumnInfo& info = wall.m_daColumns[i];
					fMin = info.m_fMin;
					fMax = info.m_fMax;

					if( !info.m_bIsValid )
					{
						count = 0;
						continue;
					}

					// space check
					S32 p0 = FLOORINT((fMin - info.m_fMin)/info.m_fPartHeight);
					S32 p1 = FLOORINT((fMax - info.m_fMin)/info.m_fPartHeight) - 1;
					Bool bGoToNext = FALSE;
					for(S32 p=p0; p<=p1; p++)
					{
						if ((info.m_daSpaceInFrontOf.GetSize() > p) &&
							(info.m_daSpaceInFrontOf[p] < iDepth))
						{
							bGoToNext = TRUE;
							count = 0;
							break;
						}
					}

					if( !bGoToNext )
					{
						if( info.m_iSpaceBottomSize >= minFloorDist
							&& fMin <= m_YGround + 0.08f 
							&& fMax > m_YGround + _heightMin )
						{
							count++;
							if(count * columnDelta >= _widthMin - Float_Eps )
							{
								Vec3f middle = wall.m_vBaseStart + (i * columnDelta - _widthMin * .5f + .5f * columnDelta ) * deltaDir;
								middle.y = fMin + _heightMin * .5f;
								_outPos.Add(middle);
								_outNormal.Add(wall.m_vNormal);
								count--;
							}
						}
						else
						{
							count = 0;
							fMin = heightMin;
							fMax = heightMax;
						}
					}
				}
			}
		}
	}
}

//-----------------------------------

void	TopologyAnalyzer_W::GetAllPosOnFloor(Float _minSize, Vec3fDA& _outPos, Float _minHeight /*= 0.f*/) const
{
	Float halfSize = _minSize*.5f;
	U32 minSize = FLOOR(halfSize/m_fSizeVoxel) + 1;
	U32 minHeight = FLOOR(_minHeight/m_fSizeVoxel);

	for(S32 s=0; s<m_daSurfaces.GetSize();s++)
	{
		const Surface& surface = m_daSurfaces[s];
		if( surface.m_bIsGround )
		{
			for(S32 b=0; b<surface.m_daCells.GetSize(); b++)
			{
				if( surface.m_daCells[b].m_iDistFromBorder >= minSize
					&& surface.m_daCells[b].m_iSpaceToUp >= minHeight)
				{
					Vec3f pos = GetCellPosition(surface, b);
					if( minHeight == 0 || RectangleIsOk(pos+Vec3f(halfSize,0.f,halfSize),pos+Vec3f(-halfSize,0.f,halfSize),pos+Vec3f(halfSize,0.f,-halfSize),pos+Vec3f(-halfSize,0.f,-halfSize),surface,m_fSizeVoxel,TOPOLOGY_REQUEST_SPACE,minHeight) )
					{
						_outPos.Add(pos);
					}
				}
			}
		}
	}
}

//-----------------------------------

void TopologyAnalyzer_W::GetAllRectanglePosOnFloor(Float _minWidth, Float _minLength, Vec3fDA& _outPos, Vec3fDA& _outLengthDirection) const
{
	S32 size = m_daSurfaces.GetSize();
	for(S32 s = 0; s < size; ++s)
	{
        if(m_daSurfaces[s].m_bIsGround)
            GetAllRectanglePos(_minWidth, _minLength, m_daSurfaces[s], _outPos, _outLengthDirection);
    }
}

//-----------------------------------

void TopologyAnalyzer_W::GetAllPosSittable(Float _minHeight, Float _maxHeight, Float _depth, Vec3fDA& _outPos, Vec3fDA& _outNormal) const
{
	S32 depthVox = Max((S32)1,(S32)FLOORINT(1.f+(_depth * .5f / m_fSizeVoxel)));

	S32 dist = depthVox;

//   U32 dist = FLOORINT(_depth * .5f / m_fSizeVoxel);
//	dist=Max((U32)2,dist);

    Vec3fDA normals;

    normals.Add(Vec3f(-1, 0, 0));
    normals.Add(Vec3f(0, 0, -1));
    normals.Add(Vec3f(0, 0, 1));
    normals.Add(Vec3f(1, 0, 0));

	normals.Add(Vec3f(-0.7071f, 0, -0.7071f));
	normals.Add(Vec3f(0.7071f, 0, -0.7071f));
	normals.Add(Vec3f(-0.7071f, 0, 0.7071f));
	normals.Add(Vec3f(0.7071f, 0, 0.7071f));

	for (S32 s = 0; s < m_daSurfaces.GetSize(); ++s)
	{
		const Surface& surface = m_daSurfaces[s];
		S32 surfaceSizeX = surface.m_vMaxPos2D.x - surface.m_vMinPos2D.x + 1;
		S32 surfaceSizeY = surface.m_vMaxPos2D.y - surface.m_vMinPos2D.y + 1;

		if (surface.m_fHeightFromGround > _minHeight - Float_Eps && surface.m_fHeightFromGround < _maxHeight + Float_Eps )
		{
			for (S32 c = 0; c < surface.m_daCells.GetSize(); ++c)
			{
				const CellInfo* pMidCell =&surface.m_daCells[c];
				if (!pMidCell->IsValid())
					continue;
				if (pMidCell->m_iDistFromWall<=(U32)1)
					continue;
                if (pMidCell->m_iDistFromFloor > (U32)dist)
                    continue;
                if (pMidCell->m_iDistFromBorder > (U32)dist)
                    continue;

                Vec3f pos = GetCellPosition(surface, c);

                for (S32 n = 0; n < normals.GetSize(); ++n)
                {
                    const Vec3f& normal = normals[n];

					{
						// Check normal
						const CellInfo* pCell = GetCellInSurface(pos + normal * m_fSizeVoxel * (dist-1), surface, m_fSizeVoxel);
						if (!pCell || pCell->m_iDistFromFloor > 1 || pCell->m_iDistFromWall <= 1)
							continue;
					}
					{
						// Check normal
						const CellInfo* pCell = GetCellInSurface(pos + normal * m_fSizeVoxel * (dist), surface, m_fSizeVoxel);
						if (pCell && pCell->IsValid())
							continue;
					}

					Bool	withOk=TRUE;
					for (S32 i = 1; i < dist-1; i++)
					{
						const CellInfo* pCell = GetCellInSurface(pos + normal*m_fSizeVoxel * i, surface, m_fSizeVoxel);
						if ((!pCell) || (pCell->m_iDistFromFloor > (U32)dist) || (!pCell->IsValid()))
						{
							withOk = FALSE;
							break;
						}
					}
					if (!withOk)
						continue;

                    _outPos.Add(pos);
                    _outNormal[_outNormal.Add(normal)].Normalize();
                }
			}
		}
	}
}

//-----------------------------------

void TopologyAnalyzer_W::GetAllLargePosSittable(Float _minHeight, Float _maxHeight, Float _depth, Float _widthMin, Vec3fDA& _outPos, Vec3fDA& _outNormal) const
{
	static Vec3fDA normals;
	static Bool bNormalsInitialized = FALSE;
	if (!bNormalsInitialized)
	{
		bNormalsInitialized = TRUE;
		normals.Add(Vec3f(-1, 0, 0));
		normals.Add(Vec3f(0, 0, -1));
		normals.Add(Vec3f(0, 0, 1));
		normals.Add(Vec3f(1, 0, 0));

		normals.Add(Vec3f(-0.7071f, 0, -0.7071f));
		normals.Add(Vec3f(0.7071f, 0, -0.7071f));
		normals.Add(Vec3f(-0.7071f, 0, 0.7071f));
		normals.Add(Vec3f(0.7071f, 0, 0.7071f));
	}

	S32 size = m_daSurfaces.GetSize();
	for (S32 i = 0; i < size; ++i)
		GetAllLargePosSittableOnSurface(m_daSurfaces[i], normals, _minHeight, _maxHeight, _depth, _widthMin, _outPos, _outNormal);
}

//-----------------------------------

void TopologyAnalyzer_W::GetAllLargePosSittableOnSurface(const Surface& _surfaces, const Vec3fDA& _daPossibleNormals, Float _minHeight, Float _maxHeight, Float _depth, Float _widthMin, Vec3fDA& _outPos, Vec3fDA& _outNormal) const
{
	S32 widthVox = Max((S32)1,(S32)FLOORINT(1.f+(_widthMin * .5f / m_fSizeVoxel)));
	S32 depthVox = Max((S32)1,(S32)FLOORINT(1.f+(_depth * .5f / m_fSizeVoxel)));

	S32 dist = Min(widthVox,depthVox);

	S32 surfaceSizeX = _surfaces.m_vMaxPos2D.x - _surfaces.m_vMinPos2D.x + 1;
	S32 surfaceSizeY = _surfaces.m_vMaxPos2D.y - _surfaces.m_vMinPos2D.y + 1;

	if (_surfaces.m_fHeightFromGround > _minHeight - Float_Eps && _surfaces.m_fHeightFromGround < _maxHeight + Float_Eps )
	{
		for (S32 c = 0; c < _surfaces.m_daCells.GetSize(); ++c)
		{
			const CellInfo* pMidCell =&_surfaces.m_daCells[c];
			if (!pMidCell->IsValid())
				continue;
			if (pMidCell->m_iDistFromWall<(U32)dist)
				continue;
			if (pMidCell->m_iDistFromFloor > (U32)dist)
				continue;
			if (pMidCell->m_iDistFromBorder > (U32)dist)
				continue;
			if(pMidCell->m_iSpaceToUp < 12 ) // 12 * 0.08 ~= 1meter
				continue;

			Vec3f pos = GetCellPosition(_surfaces, c);
			for (S32 n = 0; n < _daPossibleNormals.GetSize(); ++n)
			{
				const Vec3f& normal = _daPossibleNormals[n];
				Vec3f tangent = normal ^ VEC3F_DOWN;
				tangent.Normalize(); // We need the tangent normalized
				Vec3f	NorPos=normal * m_fSizeVoxel * (dist);


				{
					// Check normal
					const CellInfo* pCell = GetCellInSurface(pos + normal * m_fSizeVoxel * (dist-1), _surfaces, m_fSizeVoxel);
					if (!pCell || pCell->m_iDistFromFloor > 1 || pCell->m_iDistFromWall <= 1)
						continue;
				}
				{
					// Check normal
					const CellInfo* pCell = GetCellInSurface(pos + normal * m_fSizeVoxel * (dist), _surfaces, m_fSizeVoxel);
					if (pCell && pCell->IsValid())
						continue;
				}

				Bool	withOk=TRUE;
				for (S32 i = 1; i < dist-1; i++)
				{
					const CellInfo* pCell = GetCellInSurface(pos + normal*m_fSizeVoxel * i, _surfaces, m_fSizeVoxel);
					if ((!pCell) || (pCell->m_iDistFromFloor > (U32)dist) || (!pCell->IsValid()))
					{
						withOk = FALSE;
						break;
					}
				}
				if (!withOk)
					continue;

				// Check width
				for (S32 i = 0; i < widthVox; ++i)
				{
					Vec3f	TanPos1=pos + tangent * m_fSizeVoxel * i;
					Vec3f	TanPos2=pos - tangent * m_fSizeVoxel * i;
					const CellInfo* pCell = GetCellInSurface(TanPos1 + NorPos, _surfaces, m_fSizeVoxel);
					if (pCell && pCell->m_iDistFromFloor > 0)
					{
						withOk = FALSE;
						break;
					}
/*					if (!pCell || (pCell->m_iDistFromFloor > 1) || (!pCell->IsValid()))
					{
						withOk = FALSE;
						break;
					}
*/					pCell = GetCellInSurface(TanPos2 + NorPos, _surfaces, m_fSizeVoxel);
					if (pCell && pCell->m_iDistFromFloor > 0)
					{
						withOk = FALSE;
						break;
					}
/*					if (!pCell || (pCell->m_iDistFromFloor > 1) || (!pCell->IsValid()))
					{
						withOk = FALSE;
						break;
					}
*/					pCell = GetCellInSurface(TanPos1, _surfaces, m_fSizeVoxel);
					if ((!pCell) || (!pCell->IsValid()) )
					{
						withOk = FALSE;
						break;
					}
					pCell = GetCellInSurface(TanPos2, _surfaces, m_fSizeVoxel);
					if ((!pCell) || (!pCell->IsValid()) )
					{
						withOk = FALSE;
						break;
					}
				}

				if (!withOk)
					continue;

				_outPos.Add(pos);
				_outNormal[_outNormal.Add(normal)].Normalize();
			}
		}
	}
}

//-----------------------------------

TopologyAnalyzer_W::Wall*	TopologyAnalyzer_W::GetWall(const PlaySpaceInfos_W* _pPlayspaceInfos, const Vec2f& _pos, const Vec3f& _normal, Float _height, Bool _isVirtual, Bool _isExternal)
{
	for(S32 w=0; w<m_daWalls.GetSize(); w++)
	{
		TopologyAnalyzer_W::Wall& curWall = m_daWalls[w];
		if( curWall.m_vNormal * _normal > .99f
			&& curWall.m_bIsVirtual == _isVirtual
			&& POS_Z(curWall.m_fDistWall_DeprecatedV3 - _height) < 0.03f)
		{
			Vec3f tangent = VEC3F_UP ^ _normal;
			Float posOnTangent = tangent.x * _pos.x + tangent.z * _pos.y;
			Float minOnTangent = tangent * curWall.m_vBaseStart;
			Float maxOnTangent = tangent * curWall.m_vBaseEnd;
			Float distToMin = POS_Z(posOnTangent - minOnTangent);
			Float distToMax = POS_Z(posOnTangent - maxOnTangent);
			if( distToMin < _pPlayspaceInfos->m_SizeVoxel * 1.5f || distToMax < _pPlayspaceInfos->m_SizeVoxel * 1.5f)
			{
				return &m_daWalls[w];
			}
		}
	}

	return NULL;
}

//-----------------------------------

void	TopologyAnalyzer_W::GetLargestPosOnFloor(Vec3fDA& _outPos) const
{
	U32 maxDist = 0;

	for(S32 s = 0; s < m_daSurfaces.GetSize(); s++)
	{
		const Surface& surface = m_daSurfaces[s];

		if (surface.m_bIsGround)
        {
			for(S32 c = 0; c < surface.m_daCells.GetSize(); c++)
			{
				Vec3f pos = GetCellPosition(surface, c);

				if (surface.m_daCells[c].m_iDistFromBorder == maxDist)
					_outPos.Add(pos);
				else if (surface.m_daCells[c].m_iDistFromBorder > maxDist)
				{
					_outPos.Flush();
					_outPos.Add(pos);
					maxDist = surface.m_daCells[c].m_iDistFromBorder;
				}
			}
		}
	}
}

//-----------------------------------
const TopologyAnalyzer_W::Wall *TopologyAnalyzer_W::GetLargestWall(Bool _bAllowVirtualWall /*= TRUE*/, Bool _bForceExternalWall /*=FALSE*/) const
{
	Float fMaxArea = -1.f;
	S32 iBiggestWall = -1;
	for(S32 w = 0; w < m_daWalls.GetSize(); ++w)
	{
		const TopologyAnalyzer_W::Wall& wall = m_daWalls[w];
		if (wall.m_bIsVirtual && ! _bAllowVirtualWall)
			continue;

		if(!wall.m_bIsExternal && _bForceExternalWall )
			continue;

		if(wall.m_fArea>fMaxArea)
		{
			fMaxArea = wall.m_fArea;
			iBiggestWall = w;
		}
	}
	if (iBiggestWall>=0)
		return &m_daWalls[iBiggestWall];
	else
		return 0;
}

void	TopologyAnalyzer_W::GetAllLargePosOnWall(Float _heightMin, Float _heightSizeMin, Float _widthMin, Float _fDepth, Vec3fDA& _outPos, Vec3fDA& _outNormal, FloatDA& _outWidth, Bool _bAllowVirtualWall) const
{
	S32 iDepth = FLOORINT(_fDepth / m_fSizeVoxel);
	for (S32 w = 0; w < m_daWalls.GetSize(); w++)
	{
		const Wall& wall = m_daWalls[w];
		if (wall.m_bIsVirtual && !_bAllowVirtualWall)
			continue;
		S32 nbColumn = wall.m_daColumns.GetSize();
		Float wallWidth = nbColumn * m_fSizeVoxel;

		if (wallWidth >= _widthMin)
		{
			Vec3f tangent = wall.m_vTangent;

			S32 nbColumnMin = _widthMin / m_fSizeVoxel;
			S32 firstColumn = nbColumnMin / 2;
			S32 lastColumn = wall.m_daColumns.GetSize() - 1 - nbColumnMin / 2;

			for (S32 c = firstColumn; c <= lastColumn; c++)
			{
				const WallColumnInfo& column = wall.m_daColumns[c];

				Vec3f pos = wall.m_vBaseStart + tangent * c * m_fSizeVoxel;

				Float minYPos = Max(column.m_fMin, m_YGround + _heightMin);
				Float maxYPos = Min(column.m_fMax, column.m_fMax - _heightSizeMin);

				// space check
				S32 p0 = Max(FLOORINT((minYPos - column.m_fMin)/column.m_fPartHeight), 0);
				S32 p1 = Max(FLOORINT((maxYPos - column.m_fMin)/column.m_fPartHeight), 0);
				Bool bGoToNext = FALSE;
				for(S32 p=p0; p<=p1; p++)
				{
					if( (column.m_daSpaceInFrontOf.GetSize() > p) &&
						(column.m_daSpaceInFrontOf[p] < iDepth) )
					{
						bGoToNext = TRUE;
						break;
					}
				}

				if( !bGoToNext )
				{
					Float width = Min(c, nbColumn - 1 - c) * m_fSizeVoxel * 2.f;
					pos.y = minYPos;
					while (pos.y <= maxYPos)
					{
						_outPos.Add(pos);
						_outNormal.Add(wall.m_vNormal);
						_outWidth.Add(width);

						pos.y += m_fSizeVoxel;
					}
				}
			}
		}
	}
}

//--------------------------------

void	TopologyAnalyzer_W::GetAllPosOnWallNearCeiling(Float _ceilingDist, Float _widthMin, Float _fDepth, Vec3fDA& _outPos, Vec3fDA& _outNormal, FloatDA& _outWidth) const
{
	Float ceilingHeight = GetYCeiling();
	S32 iDepth = FLOORINT(_fDepth / m_fSizeVoxel);

	for (S32 w = 0; w < m_daWalls.GetSize(); w++)
	{
		const Wall& wall = m_daWalls[w];
		S32 nbColumn = wall.m_daColumns.GetSize();
		Float wallWidth = nbColumn * m_fSizeVoxel;

		if (wallWidth >= _widthMin)
		{
			Vec3f tangent = wall.m_vTangent;
			S32 nbColumnMin = _widthMin / m_fSizeVoxel;
			S32 firstColumn = nbColumnMin / 2;
			S32 lastColumn = wall.m_daColumns.GetSize() - 1 - nbColumnMin / 2;

			for (S32 c = firstColumn; c <= lastColumn; c++)
			{
				const WallColumnInfo& column = wall.m_daColumns[c];

				if( !column.m_bIsValid )
				{
					continue;
				}

				Vec3f pos = wall.m_vBaseStart + tangent * c * m_fSizeVoxel;
				pos.y = ceilingHeight - _ceilingDist;

				Float width = Min(c, nbColumn - 1 - c) * m_fSizeVoxel * 2.f;
				if (pos.y >= column.m_fMin && pos.y <= column.m_fMax)
				{
					// space check
					S32 p0 = FLOORINT((pos.y - column.m_fMin)/column.m_fPartHeight);
					S32 p1 = FLOORINT((column.m_fMax - column.m_fMin)/column.m_fPartHeight);
					Bool bGoToNext = FALSE;
					for(S32 p=p0; p<=p1; p++)
					{
						if ((column.m_daSpaceInFrontOf.GetSize() > p) &&
							(column.m_daSpaceInFrontOf[p] < iDepth))
						{
							bGoToNext = TRUE;
							break;
						}
					}

					if( !bGoToNext )
					{
						_outPos.Add(pos);
						_outNormal.Add(wall.m_vNormal);
						_outWidth.Add(width);
					}
				}
			}
		}
	}
}

//--------------------------------

void TopologyAnalyzer_W::GetAllCouch(Float _fSitHeightMin, Float _fSitHeightMax, Float _fBackHeightMin, Float _fBackHeightMax,  Float _fSitAreaMin, Vec3fDA& _outPos, Vec3fDA& _outNormal, FloatDA& _outLength, FloatDA& _outWidth, Bool _displayDebug) const
{
    // Get group of surface who can be a couch
	DynArray_Z<S32> groupOk;

	for (S32 g = 0; g < m_daSurfaceGroups.GetSize(); ++g)
	{
		const DynArray_Z<const Surface*>& group = m_daSurfaceGroups[g];

        DynArray_Z<const Surface*> backSurfaces;
        DynArray_Z<const Surface*> sitSurfaces;
        Float totalBackArea = 0;
        Float totalSitArea = 0;
		for (S32 s = 0; s < group.GetSize(); ++s)
		{
            if (group[s]->m_fHeightFromGround >= _fBackHeightMin && group[s]->m_fHeightFromGround <= _fBackHeightMax)
            {
                backSurfaces.Add(group[s]);
                totalBackArea += GetSurfaceArea(*group[s], m_fSizeVoxel);
            }
            else if (group[s]->m_fHeightFromGround >= _fSitHeightMin && group[s]->m_fHeightFromGround <= _fSitHeightMax)
            {
                sitSurfaces.Add(group[s]);
                totalSitArea += GetSurfaceArea(*group[s], m_fSizeVoxel);
            }
        } // for surface in group

        if (group.GetSize() == backSurfaces.GetSize() + sitSurfaces.GetSize() &&
            backSurfaces.GetSize() > 0 && sitSurfaces.GetSize() &&
            totalSitArea >= _fSitAreaMin && totalSitArea >= totalBackArea)
        {
            Vec3f backMedian = VEC3F_NULL;
            S32 nbBackCase = 0;
            for (S32 i = 0; i < backSurfaces.GetSize(); ++i)
            {
                const Surface& surface = *backSurfaces[i];

                for (S32 c = 0; c < surface.m_daCells.GetSize(); ++c)
                {
                    if (surface.m_daCells[c].m_iDistFromVoid != 0 && surface.m_daCells[c].m_iDistFromWall != 0)
                    {
				        Vec3f pos = GetCellPosition(surface, c);

                        backMedian += pos;
                        ++nbBackCase;

                        //if (_displayDebug)
                        //    DRAW_DEBUG_SPHERE3D_FAST(pos, 0.02f, COLOR_RED);
                    }
                }
            }
            backMedian /= nbBackCase;

            Vec3fDA sitPos;
            Vec3f sitMedian = VEC3F_NULL;
            S32 nbSitCase = 0;
            for (S32 i = 0; i < sitSurfaces.GetSize(); ++i)
            {
                const Surface& surface = *sitSurfaces[i];

                for (S32 c = 0; c < surface.m_daCells.GetSize(); ++c)
                {
                    if (surface.m_daCells[c].m_iDistFromVoid != 0 && surface.m_daCells[c].m_iDistFromWall != 0)
                    {
				        Vec3f pos = GetCellPosition(surface, c);

                        sitPos.Add(pos);

                        sitMedian += pos;
                        ++nbSitCase;

                        //if (_displayDebug)
                        //    DRAW_DEBUG_SPHERE3D_FAST(pos, 0.02f, COLOR_BLUE);
                    }
                }
            }
            sitMedian /= nbSitCase;

            Float length;
            Vec3f direction = GetDominantDirection(sitPos, length);
            direction.Normalize();

            Vec3f normal = VEC3F_UP ^ direction;
            if ((Vec3_Dot(normal, backMedian - sitMedian) > 0))
                normal *= -1.f;
            normal.Normalize();

            S32 best = -1;
		    Float maxDist;

		    for (S32 c = 0; c < sitPos.GetSize(); ++c)
		    {
			    const Float dot = Vec3_Dot(direction, sitPos[c] - sitMedian);
			    const Float dotMin = 0.1f;

			    if (dot <= dotMin && dot >= -dotMin)
			    {
				    Float dist = Vec3f(sitPos[c] - sitMedian).GetNorm2();
				    if (best < 0 || dist > maxDist)
				    {
					    best = c;
					    maxDist = dist;
				    }
			    }
		    }

		    ASSERT_Z(best >= 0);
		    Float width = Vec3f(sitPos[best] - sitMedian).GetNorm() * 2.f;

            _outPos.Add(sitMedian);
            _outNormal.Add(normal);
            _outLength.Add(length);
            _outWidth.Add(width);
        }
	} // for each group
}

//--------------------------------

void TopologyAnalyzer_W::GetAllPosOnSurface(const Surface& _surface, Float _fminSize, Vec3fDA& _outPos) const
{
    U32 minSize = FLOOR(_fminSize * .5f / m_fSizeVoxel) + 1;

    for(S32 c = 0; c < _surface.m_daCells.GetSize(); ++c)
    {
        if (_surface.m_daCells[c].m_iDistFromBorder >= minSize)
        {
            Vec3f pos = GetCellPosition(_surface, c);
            _outPos.Add(pos);
        }
    }
}

//--------------------------------

void TopologyAnalyzer_W::GetAllPosOnSurfaceInSphere(const Vec3f& _sphereCenter, Float _sphereRadius, Float _posRadius, Vec3fDA& _outPos) const
{
    Float sphereRadius2 = _sphereRadius * _sphereRadius;
    U32 _posRadius2D = FLOORINT(_posRadius / m_fSizeVoxel);

    Vec2i sphereCenter2D = Vec2i((_sphereCenter.x - m_vMinPos.x) / m_fSizeVoxel, (_sphereCenter.z - m_vMinPos.z) / m_fSizeVoxel);
    S32 sphereRadius2D = _sphereRadius / m_fSizeVoxel;

    for (S32 y = sphereCenter2D.y - sphereRadius2D; y <= sphereCenter2D.y + sphereRadius2D; ++y)
    {
        for (S32 x = sphereCenter2D.x - sphereRadius2D; x <= sphereCenter2D.x + sphereRadius2D; ++x)
        {
            if (x < m_vMinPos2D.x || x > m_vMaxPos2D.x || y < m_vMinPos2D.y || y > m_vMaxPos2D.y) continue;

            Vec3f planePos(x * m_fSizeVoxel + m_vMinPos.x, _sphereCenter.y, y * m_fSizeVoxel + m_vMinPos.z);
            if (Vec3_GetNorm2(planePos - _sphereCenter) > sphereRadius2) continue;

            // Find if the cell is in an horizontal surface
            for (S32 s = 0; s < m_daSurfaces.GetSize(); ++s)
            {
                const Surface& surface = m_daSurfaces[s];
                S32 surfaceSizeX = surface.m_vMaxPos2D.x - surface.m_vMinPos2D.x + 1;

                if (x < surface.m_vMinPos2D.x || x > surface.m_vMaxPos2D.x || y < surface.m_vMinPos2D.y || y > surface.m_vMaxPos2D.y) continue;

                S32 inSurfaceX = x - surface.m_vMinPos2D.x;
                S32 inSurfaceY = y - surface.m_vMinPos2D.y;

                // The cell is in the surface... we check if the cell is not in wall, in void and if the cell have a good raduis around her
                const CellInfo& cell = surface.m_daCells[inSurfaceY * surfaceSizeX + inSurfaceX];
                if (cell.m_iDistFromWall < _posRadius2D || cell.m_iDistFromVoid == 0) continue;

                Vec3f pos(planePos.x, surface.m_vMinPos.y, planePos.z);
                if (Vec3_GetNorm2(pos - _sphereCenter) <= sphereRadius2)
                {
                    _outPos.Add(pos);
                    break; // Normaly a cell can only be in one surface
                }
            }
        }
    }
}

//--------------------------------

Bool TopologyAnalyzer_W::IsFullWall(const Wall& _wall) const
{
	return ((_wall.m_vBaseStart.y+_wall.m_fHeight) >= GetYCeiling() - 0.5f); // full wall is only wall that reaches up to the ceiling...
}

//---------------------------

Bool TopologyAnalyzer_W::CellIsVisibleFromPlayspaceCenter(const PlaySpaceInfos_W* _pPlayspaceInfos,const Vec3f& _vPos) const
{
	Vec3f	PlaySpaceCenter=m_vRoomCenter;
	//PlaySpaceCenter.y=Min((_pPlayspaceInfos->m_YGround+1.8f),(_pPlayspaceInfos->m_YCeiling-0.3f));
	Vec3f	FromPos=_vPos;
	FromPos.y= m_vRoomCenter.y;
	Vec3f delta = (PlaySpaceCenter-FromPos);

	for (Float i=0.f; i<1.f; i+=0.01f)	// 100 steps
	{
		Vec3f	testpos=FromPos+i*delta;
		Vec3f	DeltaFromCentre=PlaySpaceCenter-testpos;
		if (DeltaFromCentre.GetNorm2()>1.f)	// On ne teste pas à moins d'1m du centre de la piece... si jamais il joue autour d'une cheminee...
		{
			Vec2i	itestpos;
			_pPlayspaceInfos->ComputeCellIndice(testpos, itestpos.x, itestpos.y);
			if (IsWall_V3(_pPlayspaceInfos,testpos.y,itestpos))
				return FALSE;
		}
	}
	return TRUE;
}

//--------------------------------

Bool TopologyAnalyzer_W::RectangleIsInPlaySpace(const PlaySpaceInfos_W* _pPlaySpaceInfos, const Vec2f& _vPos, const Vec2f& _vSize, const Quat& _qRot) const
{
	Vec2f topLeft = _vPos + _qRot * Vec3f(-_vSize.x * .5f, 0.f, -_vSize.y * .5f);
    Vec2f topRight = _vPos + _qRot * Vec3f(-_vSize.x * .5f, 0.f, _vSize.y * .5f);
    Vec2f bottomLeft = _vPos + _qRot * Vec3f(_vSize.x * .5f, 0.f, -_vSize.y * .5f);
    Vec2f bottomRight = _vPos + _qRot * Vec3f(_vSize.x * .5f, 0.f, _vSize.y * .5f);

	Vec2f min = Vec2f(_pPlaySpaceInfos->m_vMinAvailable.x, _pPlaySpaceInfos->m_vMinAvailable.z);
	Vec2f max = Vec2f(_pPlaySpaceInfos->m_vMaxAvailable.x, _pPlaySpaceInfos->m_vMaxAvailable.z);

	return (topLeft.x >= min.x && topLeft.x <= max.x && topLeft.y >= min.y && topLeft.y <= max.y &&
		topRight.x >= min.x && topRight.x <= max.x && topRight.y >= min.y && topRight.y <= max.y &&
		bottomLeft.x >= min.x && bottomLeft.x <= max.x && bottomLeft.y >= min.y && bottomLeft.y <= max.y &&
		bottomRight.x >= min.x && bottomRight.x <= max.x && bottomRight.y >= min.y && bottomRight.y <= max.y);
}

//--------------------------------

Float TopologyAnalyzer_W::GetMaxYInRectangle(const PlaySpaceInfos_W* _pPlayspaceInfos, const Vec2f& _vPos, const Vec2f& _vSize, const Quat& _rot, Float _fMinY, Float _fMaxY) const
{
    Vec2f bottomRight = _vPos + _rot * Vec3f(-_vSize.x * .5f, 0.f, -_vSize.y * .5f);
    Vec2f topRight = _vPos + _rot * Vec3f(-_vSize.x * .5f, 0.f, _vSize.y * .5f);
    Vec2f bottomLeft = _vPos + _rot * Vec3f(_vSize.x * .5f, 0.f, -_vSize.y * .5f);
    Vec2f topLeft = _vPos + _rot * Vec3f(_vSize.x * .5f, 0.f, _vSize.y * .5f);

    S32 nbCellX = _pPlayspaceInfos->m_NbCellX;
    S32 nbCellY = _pPlayspaceInfos->m_NbCellY;

    Vec2f leftToRight = topRight - topLeft;
    Float width = _vSize.x;
    leftToRight *= 1.f / width;

    Vec2f topToBottom = bottomLeft - topLeft;
    Float length = _vSize.y;
    topToBottom *= 1.f / length;

    Float maxY = _fMinY;
    for (Float i = m_fSizeVoxel * .5f; i < width; i += m_fSizeVoxel)
    {
        for (Float j = m_fSizeVoxel * .5f; j < length; j += m_fSizeVoxel)
        {
            Vec2f cell = topLeft + leftToRight * i + topToBottom * j;
            Vec3f tmpPos = Vec3f(cell.x, 0.f, cell.y);

            S32 cellX, cellY;
            if (!_pPlayspaceInfos->ComputeCellIndice(tmpPos, cellX, cellY))
                continue;

            S32 idx = cellY * nbCellX + cellX;

			PlaySpace_CellInfos *pCell = _pPlayspaceInfos->m_pSurfaceInfosV3->m_Surfels.m_CellBoard.GetArrayPtr() +idx;
			PlaySpace_Surfel *pSurfel = pCell->pFirst;
			if( !pSurfel )
			{
				if( _pPlayspaceInfos->m_YCeiling < _fMaxY )
					return _pPlayspaceInfos->m_YCeiling; // nothing higher than ceilling
			}
			else
			{
				while(pSurfel)
				{
					if (pSurfel->iDir != PlaySpace_Surfel::SURFDIR_DOWN
						&& _fMinY <= pSurfel->Point.y
						&& _fMaxY > pSurfel->Point.y)
						maxY = Max(pSurfel->Point.y,maxY);

					pSurfel = pSurfel->pNext;
				}
			}
        }
    }

    return maxY;
}

//--------------------------------

Bool    TopologyAnalyzer_W::HasWallNear(const Vec3f& _vPos, Float _radius, Float _wallHeightMin) const
{
    for (S32 w = 0; w < m_daWalls.GetSize(); ++w)
    {
        const Wall& wall = m_daWalls[w];

        if (wall.m_bIsVirtual)
            continue;

        if (wall.m_fHeight < _wallHeightMin)
            continue;

		Float fDist = GetDistPointVsWall(_vPos, wall);
		if (fDist <= _radius)
			return TRUE;
    }

    return FALSE;
}

//--------------------------------

Float    TopologyAnalyzer_W::GetDistWallNear(const Vec3f& _vPos, Float _wallHeightMin, Bool _virtual) const
{
	Float distMin(Float_Max);
	Float distInter(0.f);

    for (S32 w = 0; w < m_daWalls.GetSize(); ++w)
    {
        const Wall& wall = m_daWalls[w];

        if (wall.m_bIsVirtual!=_virtual)
            continue;

        if (wall.m_fHeight < _wallHeightMin)
            continue;

		distInter = GetDistPointVsWall(_vPos, wall);

		if (distInter<distMin)
			distMin=distInter;
    }

    return distMin;
}

//--------------------------------

Float TopologyAnalyzer_W::GetDistNearestWall(const Vec3f& _vPos, Float _wallHeightMin) const
{
	Float distMin = Float_Max;

    for (S32 w = 0; w < m_daWalls.GetSize(); ++w)
    {
        const Wall& wall = m_daWalls[w];

         if (wall.m_fHeight < _wallHeightMin)
            continue;

		Float dist = GetDistPointVsWall(_vPos, wall);

		if (dist < distMin)
			distMin = dist;
    }

    return distMin;
}

//--------------------------------

Float TopologyAnalyzer_W::GetDistNearestBorderOnGround(const Vec3f& _vPos, S32* _iNbSameDistNeighbors /*= NULL*/) const
{
	for(S32 s=0; s<m_daSurfaces.GetSize();s++)
	{
		const Surface& surface = m_daSurfaces[s];
		if( surface.m_bIsGround )
		{
			for(S32 b=0; b<surface.m_daCells.GetSize(); b++)
			{
				Vec3f pos = GetCellPosition(surface, b);

				if(pos == _vPos)
				{
					if(_iNbSameDistNeighbors != NULL)
					{
						Float dist = surface.m_daCells[b].m_iDistFromBorder;
						DynArray_Z<Vec2i> cellNeighbors;
						GetCellNeighbors(surface, b, cellNeighbors);

						for(S32 i = 0; i < cellNeighbors.GetSize(); ++i)
						{
							const CellInfo* pCellInfo = GetCellInSurface(cellNeighbors[i], surface);
							if(pCellInfo != NULL && pCellInfo->m_iDistFromBorder == dist)
							{
								(*_iNbSameDistNeighbors)++;
							}
						}
					}
					return surface.m_daCells[b].m_iDistFromBorder;
				}
			}
		}
	}

	return -1.f;
}

//--------------------------------

Bool TopologyAnalyzer_W::FindAlignedRectangles(const FloatDA& _widths, const FloatDA& _lengths, Vec3fDA& _outPos, Vec3f& _outDir) const
{
    ASSERT_Z(_lengths.GetSize() > 0);

	S32 lengthsSize = _lengths.GetSize();

    _outPos.SetSize(lengthsSize);

    DynArray_Z<Vec3fDA> poss; poss.SetSize(lengthsSize);
    DynArray_Z<Vec3fDA> dirs; dirs.SetSize(lengthsSize);
    S32DA toTry; toTry.SetSize(lengthsSize);

    for (S32 i = 0; i < lengthsSize; ++i)
    {
        GetAllRectanglePosOnFloor(_widths[i], _lengths[i], poss[i], dirs[i]);
        toTry[i] = 0;

        if (poss[i].GetSize() == 0)
            return FALSE;
    }

    S32 toStartVerif = 1;
    while (TRUE)
    {
        S32 toChange;
        Bool thisTryOk = TRUE;
        for (S32 i = toStartVerif; i < lengthsSize; ++i)
        {
            const Vec3f& previousPos = poss[i - 1][toTry[i - 1]];
            const Vec3f& previousDir = dirs[i - 1][toTry[i - 1]];
            const Vec3f previousDirPer = VEC3F_DOWN ^ previousDir;

            const Vec3f& pos = poss[i][toTry[i]];
            const Vec3f& dir = dirs[i][toTry[i]];

            if (Abs(1.f - previousDir * dir) <= 0.01f &&
                Abs(previousDirPer * (pos - previousPos)) <= 0.01f &&
                previousDir * (pos - previousPos) >= _lengths[i - 1] * 0.5f + _lengths[i] * 0.5f)
                continue;
            else
            {
                toChange = i;
                thisTryOk = FALSE;
                break;
            }
        }

        if (thisTryOk)
        {
            for (S32 i = 0; i < lengthsSize; ++i)
				_outPos[i] = poss[i][toTry[i]];
            
            _outDir = dirs[0][toTry[0]];

            return TRUE;
        }

        // Not good, find next try
        for (S32 i = toChange + 1; i < lengthsSize; ++i)
            toTry[i] = 0;

        for (S32 i = toChange; i >= 0; --i)
        {
            toTry[i] = (toTry[i] + 1) % poss[i].GetSize();

            if (toTry[i] != 0)
            {
                toStartVerif = (i > 0 ? i : 1);
                break;
            }

            if (i == 0)
                return FALSE;
        }
    }
}

//--------------------------------

Bool TopologyAnalyzer_W::FindRectanglesSequence(const FloatDA& _widths, const FloatDA& _lengths, Vec3fDA& _outPos, Vec3fDA& _outDirs) const
{
    ASSERT_Z(_lengths.GetSize() > 0);

    _outPos.SetSize(_lengths.GetSize());
    _outDirs.SetSize(_lengths.GetSize());

    DynArray_Z<Vec3fDA> poss; poss.SetSize(_lengths.GetSize());
    DynArray_Z<Vec3fDA> dirs; dirs.SetSize(_lengths.GetSize());
    S32DA toTry; toTry.SetSize(_lengths.GetSize());

    for (S32 i = 0; i < _lengths.GetSize(); ++i)
    {
        GetAllRectanglePosOnFloor(_widths[i], _lengths[i], poss[i], dirs[i]);
        toTry[i] = 0;

        if (poss[i].GetSize() == 0)
            return FALSE;
    }

    S32 toStartVerif = 1;
    while (TRUE)
    {
        S32 toChange;
        Bool thisTryOk = TRUE;
        for (S32 i = toStartVerif; i < _lengths.GetSize(); ++i)
        {
            const Vec3f& previousPos = poss[i - 1][toTry[i - 1]];
            const Vec3f& previousDir = dirs[i - 1][toTry[i - 1]];
            const Vec3f previousDirPer = VEC3F_DOWN ^ previousDir;

            const Vec3f& pos = poss[i][toTry[i]];
            const Vec3f& dir = dirs[i][toTry[i]];
            
            if ((Abs(1.f - previousDir * dir) <= 0.01f &&
                 Abs(previousDirPer * (pos - previousPos)) <= 0.01f &&
                 previousDir * (pos - previousPos) >= _lengths[i - 1] * .5f + _lengths[i] * .5f)
                ||
                (Abs(previousDir * dir) <= 0.01f &&
                 previousDir * (pos - previousPos) >= _lengths[i - 1] * .5f + _widths[i] * .5f &&
                 -(dir * (previousPos - pos)) >= _lengths[i] * .5f + _widths[i - 1] * .5f))
                 continue;
            else
            {
                toChange = i;
                thisTryOk = FALSE;
                break;
            }
        }

        if (thisTryOk)
        {
            for (S32 i = 0; i < _lengths.GetSize(); ++i)
            {
				_outPos[i] = poss[i][toTry[i]];
                _outDirs[i] = dirs[i][toTry[i]];
            }

            return TRUE;
        }

        // Not good, find next try
        for (S32 i = toChange + 1; i < _lengths.GetSize(); ++i)
            toTry[i] = 0;

        for (S32 i = toChange; i >= 0; --i)
        {
            toTry[i] = (toTry[i] + 1) % poss[i].GetSize();

            if (toTry[i] != 0)
            {
                toStartVerif = (i > 0 ? i : 1);
                break;
            }

            if (i == 0)
                return FALSE;
        }
    }
}

//--------------------------------

Bool TopologyAnalyzer_W::GroupsIsAwayFromWalls(const DynArray_Z<const Surface*>& _group)
{
    for (S32 s = 0; s < _group.GetSize(); s++)
    {
	    const Surface& surface = *_group[s];
        
        // Check if the surface is against a wall
		for (S32 c = 0; c < surface.m_daCells.GetSize(); ++c)
		{
			if (surface.m_daCells[c].m_iDistFromWall == 1)
			{
			    DynArray_Z<Vec2i> cellNeighbors;
				GetCellNeighbors(surface, c, cellNeighbors);

				// Check at least one cell neighbor is in a surface neighbor
				bool ok = false;
				for (S32 cn = 0; cn < cellNeighbors.GetSize(); cn++)
				{
					for (S32 s2 = 0; s2 < _group.GetSize(); s2++)
					{
						if (s2 != s && GetCellInSurface(cellNeighbors[cn], *_group[s2]))
						{
							ok = true;
							break;
						}
					}

					if (ok)
						break;
				}

				if (!ok)
				{
					// The wall is not part of a surface in the group so the group is agains a wall
                    return FALSE;
				}
            }
        }
    }

    return TRUE;
}
