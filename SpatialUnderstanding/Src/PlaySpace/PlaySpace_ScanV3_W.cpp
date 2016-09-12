// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include <pch.h>
#include <PlaySpace\PlaySpace_ScanV3_W.h>
#include <PlaySpace\PlaySpaceInfos_Cst_W.h>
#include <PlaySpace\PlaySpace_ScanMesh_W.h>

/**************************************************************************/

Bool	PlaySpace_CellInfos::CheckIt(S32 _x, S32 _z)
{
	PlaySpace_Surfel	*pCur = pFirst;
	while (pCur)
	{
		if ((pCur->x != _x) || (pCur->z != _z))
			return FALSE;

		PlaySpace_Surfel	*pCur2 = pCur->pNext;
		while (pCur2)
		{
			if (pCur2->y == pCur->y)
				return FALSE;
			pCur2 = pCur2->pNext;
		}

		pCur = pCur->pNext;
	}

	return TRUE;
}

/**************************************************************************/

Bool	PlaySpace_CellInfos::FindBiggestHole(Float &Down, Float &Up)
{
	// On part du plafond.
	PlaySpace_Surfel	*pSurfel = pFirst;
	Up = -1000.f;
	while (pSurfel)
	{
		if (pSurfel->iDir == PlaySpace_Surfel::SURFDIR_DOWN)
			Up = Max(Up, pSurfel->Point.y);

		pSurfel = pSurfel->pNext;
	}
	if (Up < -900.f)
		return FALSE;

	// Cherche la brique juste en dessous.
	for (;;)
	{
		pSurfel = pFirst;
		Down = -1000.f;

		while (pSurfel)
		{
			if ((pSurfel->iDir == PlaySpace_Surfel::SURFDIR_UP) && (pSurfel->Point.y < (Up - 0.08f)))
				Down = Max(pSurfel->Point.y, Down);

			pSurfel = pSurfel->pNext;
		}

		if (Down < -900.f)
			return FALSE;

		if ((Up - Down) > 0.2f)
		{
			Vec3f	a, b;
			a = b = pFirst->Point;
			a.y = Up;
			b.y = Down;
			return TRUE;
		}

		Up = Down;
	}

	return FALSE;
}

/**************************************************************************/

void	PlaySpace_SurfelBoard::Swap(PlaySpace_SurfelBoard &_Surfels)
{
	::Swap(m_SizeX,_Surfels.m_SizeX);
	::Swap(m_SizeY,_Surfels.m_SizeY);
	::Swap(m_SizeH,_Surfels.m_SizeH);

	::Swap(m_Origin,_Surfels.m_Origin);
	::Swap(m_SizeCell,_Surfels.m_SizeCell);

	::Swap(m_pFirstFreeSurfel,_Surfels.m_pFirstFreeSurfel);

	m_SurfelTab.Swap(_Surfels.m_SurfelTab);
	m_CellBoard.Swap(_Surfels.m_CellBoard);
}

/**************************************************************************/

void PlaySpace_SurfelBoard::Empty()
{
	m_pFirstFreeSurfel = NULL;
	m_SurfelTab.Empty();

	S32 nb = m_CellBoard.GetSize();
	for (S32 i = 0; i<nb; i++)
		m_CellBoard[i].pFirst = NULL;
}

/**************************************************************************/

void	PlaySpace_SurfelBoard::Flush(Bool _KeepMemory)
{
	m_SizeX = m_SizeY = m_SizeH = 0;
	m_pFirstFreeSurfel = NULL;
	if (!_KeepMemory)
	{
		m_SurfelTab.Flush();
		m_CellBoard.Flush();
	}
	else
	{
		m_SurfelTab.Empty();
		m_CellBoard.Empty();
	}
}

/**************************************************************************/

void	PlaySpace_SurfelBoard::Init(Vec3f _Origin,Float _CellSize,S32 _SizeX, S32 _SizeY, S32 _SizeH)
{
	m_SizeX = _SizeX;
	m_SizeY = _SizeY;
	m_SizeH = _SizeH;

	m_SizeCell = _CellSize;
	m_Origin = _Origin;

	m_pFirstFreeSurfel = NULL;
	m_SurfelTab.Empty();
	m_CellBoard.SetSize(_SizeX*_SizeY,TRUE);

	// Init Default Value.
	for (S32 y = 0; y < _SizeY; y++)
	{
		PlaySpace_CellInfos *pCellV2 = m_CellBoard.GetArrayPtr() + y * _SizeX;
		Float	CurZ = _Origin.z + (Float)y * _CellSize;
		for (S32 x = 0; x < _SizeX; x++)
		{
			pCellV2->fCornerX = _Origin.x + (Float)x * _CellSize;
			pCellV2->fCornerZ = CurZ;
			pCellV2->pFirst = NULL;
			pCellV2++;
		}
	}
}

/**************************************************************************/

void	PlaySpace_SurfelBoard::ComputeConexity()
{
	S32 NbCell = m_SizeX*m_SizeY;
	PlaySpace_CellInfos *pCell = m_CellBoard.GetArrayPtr();

	// Init ZoneId.
	for (S32 i = 0; i<NbCell; i++)
	{
		// Init.
		PlaySpace_Surfel	*pCur = pCell->pFirst;
		while (pCur)
		{
			pCur->ZoneId = -1;
			pCur = pCur->pNext;
		}

		// Next.
		pCell++;
	}

	// Compute ZoneId.
	PlaySpace_Surfel	*TabLinkSurfel[256];
	PlaySpace_Surfel	*TabSurfel[CONEXITY_MAX_SURF];
	S32					CurGlobalZoneId = 0;
	S32					NbSurfelOnZone = 0;

	Float CosAngleMax = Cos(DegToRad(45.0f));
	Float CosAngleMaxTolerance = Cos(DegToRad(50.0f));

	pCell = m_CellBoard.GetArrayPtr();
	for (S32 i = 0; i<NbCell; i++)
	{
		// Init.
		PlaySpace_Surfel	*pHead = pCell->pFirst;
		while (pHead)
		{
			if ((pHead->ZoneId < 0) && (!pHead->NoGameplay()))
			{
				// compute this Zone...
				S32 NbSurfel = 0;

				pHead->ZoneId = CurGlobalZoneId;	// Dont RE-check this tile.
				NbSurfelOnZone = 1;
				TabSurfel[NbSurfel++] = pHead;

				while (NbSurfel)
				{
					// UnStack.
					NbSurfel--;
					PlaySpace_Surfel *pCur = TabSurfel[NbSurfel];

					// Check Voisins.
					S32 NbLinkedSurfel = 0;
					if ((pCur->Flags & PlaySpace_Surfel::SURFFLAG_VIRTUAL) && (pHead->iDir != PlaySpace_Surfel::SURFDIR_UP) && (pHead->iDir != PlaySpace_Surfel::SURFDIR_DOWN))
						NbLinkedSurfel = GetSurfelByIDistAndDirWall(pCur->x, pCur->y, pCur->z, 1, pHead->Normal, CosAngleMax, CosAngleMaxTolerance, TabLinkSurfel, 256);
					else
						NbLinkedSurfel = GetSurfelByIDistAndIDir(pCur->x, pCur->y, pCur->z, 1, pHead->iDir, TabLinkSurfel, 256);

					for (S32 j = 0; j<NbLinkedSurfel; j++)
					{
						// Same surface ?
						PlaySpace_Surfel *pOther = TabLinkSurfel[j];
						if ((pOther->ZoneId >= 0) || (pOther->NoGameplay()))
							continue;

						// orientation + dist.
						if (!pCur->CanGo(pOther))
							continue;

						// Yes, Add it !
						pOther->ZoneId = CurGlobalZoneId;
						NbSurfelOnZone++;
						TabSurfel[NbSurfel++] = pOther;
						EXCEPTIONC_Z(NbSurfel<CONEXITY_MAX_SURF, "Too Many Surfel in ZONE SCAN");
					}
				}

				// On Surfel for one zone => suppress it !
				if (NbSurfelOnZone < 2)
				{
					// Reset Zone
					pHead->ZoneId = -1;
					CurGlobalZoneId--;
				}
				// Next.
				EXCEPTIONC_Z(CurGlobalZoneId<CONEXITY_MAX_ZONE, "Too Many ZONE ID SCAN");
				CurGlobalZoneId++;
			}

			// Next
			pHead = pHead->pNext;
		}

		// Next.
		pCell++;
	}
}

/**************************************************************************/
//#define DEBUG_CanGoDown	1

Bool PlaySpace_SurfelBoard::CanGoDown(PlaySpace_Surfel * _pSurfel, Vec3i _Dir, S32& _iZoneId)
{
	Vec3f			fDir(_Dir.x,_Dir.y,_Dir.z);
	Bool			firstIter = TRUE;

	Vec3i	iPos(_pSurfel->x,_pSurfel->y,_pSurfel->z);
	PlaySpace_Surfel *pHighestGround = NULL;
	PlaySpace_Surfel *pSlideSurface = NULL;

	PlaySpace_Surfel *pCurPosSurfel = _pSurfel;
	Float	MyCurY = pCurPosSurfel->Point.y;

	for(;;)
	{
		if ((iPos.x < 0) || (iPos.x >= m_SizeX))
			return FALSE;
		if ((iPos.z < 0) || (iPos.z >= m_SizeY))
			return FALSE;
		
		PlaySpace_CellInfos *pNextCell = GetCell(iPos.x,iPos.z);
		PlaySpace_Surfel *pOtherSurfel = pNextCell->pFirst;

		pHighestGround = NULL;
		pSlideSurface = NULL;

		while (pOtherSurfel)
		{
			if (pOtherSurfel == _pSurfel)
			{
				pOtherSurfel = pOtherSurfel->pNext;
				continue;
			}

			if (pOtherSurfel->ZoneId == _pSurfel->ZoneId)
				return FALSE;

			// Ground le plus haut (mais plus bas que le current).
			if( pOtherSurfel->Point.y < (MyCurY + 0.08f))
			{
				if(!firstIter)
				{
					if (pOtherSurfel->iDir == PlaySpace_Surfel::SURFDIR_UP)
					{
						if( !pHighestGround || pOtherSurfel->Point.y > pHighestGround->Point.y )
						{
							// orientation + dist.
							if ((_pSurfel == pCurPosSurfel) && (pOtherSurfel->Point.y >= MyCurY) && !pCurPosSurfel->CanGo(pOtherSurfel))
								return FALSE;
							pHighestGround = pOtherSurfel;
						}
					}
					else if( !pSlideSurface || pOtherSurfel->Point.y > pSlideSurface->Point.y )
					{
						if (   ((_Dir.x == 1) && (pOtherSurfel->iDir == PlaySpace_Surfel::SURFDIR_LEFT))
							|| ((_Dir.x == -1) && (pOtherSurfel->iDir == PlaySpace_Surfel::SURFDIR_RIGHT))
							|| ((_Dir.z == 1) && (pOtherSurfel->iDir == PlaySpace_Surfel::SURFDIR_FRONT))
							|| ((_Dir.z == -1) && (pOtherSurfel->iDir == PlaySpace_Surfel::SURFDIR_BACK))
							)
							pSlideSurface = pOtherSurfel;
					}
				}
			}

			// Est ce que quelque chose bloque ?
			if ((pOtherSurfel->Point.y > MyCurY) && (pOtherSurfel->Point.y < (MyCurY + 0.3f)))
			{
				if (   ((_Dir.x == 1) && (pOtherSurfel->iDir == PlaySpace_Surfel::SURFDIR_RIGHT))
					|| ((_Dir.x == -1) && (pOtherSurfel->iDir == PlaySpace_Surfel::SURFDIR_LEFT))
					|| ((_Dir.z == 1) && (pOtherSurfel->iDir == PlaySpace_Surfel::SURFDIR_BACK))
					|| ((_Dir.z == -1) && (pOtherSurfel->iDir == PlaySpace_Surfel::SURFDIR_FRONT))
					)
				{
#ifdef DEBUG_CanGoDown
					DRAW_DEBUG_LINE3D(_pSurfel->Point + (VEC3F_UP*0.1f), pOtherSurfel->Point+ (VEC3F_UP*0.1f), COLOR_RED, 0.01f, .displayDuration(1000.f));
#endif
					return FALSE;
				}
			}

			// Next.
			pOtherSurfel = pOtherSurfel->pNext;
		}

		// Slide can be a ground.
		if (pSlideSurface && pHighestGround && (pSlideSurface->Point.y > (pHighestGround->Point.y + 0.04f)))
			pHighestGround = NULL;

		// Rien trouvé !!?
		if (pHighestGround && (pHighestGround->ZoneId >= 0))
		{
			_iZoneId = pHighestGround->ZoneId;
#ifdef DEBUG_CanGoDown
			DRAW_DEBUG_LINE3D(_pSurfel->Point + (VEC3F_UP*0.1f), pHighestGround->Point+ (VEC3F_UP*0.1f), COLOR_GREEN, 0.01f, .displayDuration(1000.f));
#endif
			return TRUE;
		}
		if (pSlideSurface)
		{
			// Glisse le long du mur.
			MyCurY = pSlideSurface->Point.y;
			pCurPosSurfel = pSlideSurface;
		}

		iPos += _Dir;
		firstIter = FALSE;
	}

	return FALSE;
}

/**************************************************************************/

void	PlaySpace_SurfelBoard::BasinFilter(S32 _groundZoneId)
{
	//struct
	struct GroundZoneId
	{
		Bool m_bIsGround;
		S32DA m_daReachableZoneId;

		GroundZoneId()
		{
			m_bIsGround = FALSE;
		}
	};
	GroundZoneId GroundZones[CONEXITY_MAX_ZONE];
	S32DA daBasinZoneIds;

	// Setup ground zone id and reachable other zones
	PlaySpace_CellInfos * pCell = m_CellBoard.GetArrayPtr();
	for (S32 z=0 ; z<m_SizeY ; z++)
	{
		for (S32 x=0 ; x<m_SizeX ; x++)
		{
			PlaySpace_Surfel *pSurfel = pCell->pFirst;
			while(pSurfel)
			{
				// ici pb dans la détection de bassin => Toutes les faces n'ont pas de ZoneID.
				if ((pSurfel->iDir == PlaySpace_Surfel::SURFDIR_UP) && (pSurfel->ZoneId >= 0))
				{
					if ((pSurfel->ZoneId >= 0) && (!GroundZones[pSurfel->ZoneId].m_bIsGround))
					{
						GroundZones[pSurfel->ZoneId].m_bIsGround = TRUE;
						daBasinZoneIds.Add(pSurfel->ZoneId);
					}

					S32 zoneId = -1;
					Vec3i	Pos(pSurfel->x,pSurfel->y,pSurfel->z);
					Vec3i	Dir = VEC3I_NULL;	
					Dir.x = -1;
					if(	 x >= 0 && CanGoDown(pSurfel,Dir,zoneId) && GroundZones[pSurfel->ZoneId].m_daReachableZoneId.Contains(zoneId) < 0 )	
						GroundZones[pSurfel->ZoneId].m_daReachableZoneId.Add(zoneId);

					Dir.x = +1;
					if( x < m_SizeX && CanGoDown(pSurfel,Dir,zoneId) && GroundZones[pSurfel->ZoneId].m_daReachableZoneId.Contains(zoneId) < 0 )
						GroundZones[pSurfel->ZoneId].m_daReachableZoneId.Add(zoneId);

					Dir.x = 0;
					Dir.z = -1;

					if( z >= 0 && CanGoDown(pSurfel,Dir,zoneId) && GroundZones[pSurfel->ZoneId].m_daReachableZoneId.Contains(zoneId) < 0 )
						GroundZones[pSurfel->ZoneId].m_daReachableZoneId.Add(zoneId);

					Dir.z = +1;
					if( z < m_SizeY && CanGoDown(pSurfel,Dir,zoneId) && GroundZones[pSurfel->ZoneId].m_daReachableZoneId.Contains(zoneId) < 0 )
						GroundZones[pSurfel->ZoneId].m_daReachableZoneId.Add(zoneId);
				}

				pSurfel = pSurfel->pNext;
			}

			pCell++;
		}
	}

	// Found unreachable ground (remove reachable ids from floor)
	S32DA openList;
	openList.Add(_groundZoneId);
	S32 id = daBasinZoneIds.Contains(_groundZoneId);
	if( id < 0 )
		return;

	daBasinZoneIds.Remove(id);
	while(openList.GetSize() > 0 )
	{
		S32DA nextOpenList;
		for(S32 i=0;i<openList.GetSize();i++)
		{
			S32 zoneId = openList[i];
			for(S32 j=daBasinZoneIds.GetSize()-1; j>=0; j--)
			{
				S32 basinZoneId = daBasinZoneIds[j];
				if( GroundZones[basinZoneId].m_daReachableZoneId.Contains(zoneId) >= 0 )
				{
					nextOpenList.Add(basinZoneId);
					daBasinZoneIds.Remove(j);
				}
			}
		}

		openList = nextOpenList;
	}

	// Flag all surfel
	pCell = m_CellBoard.GetArrayPtr();
	S32 NbCells = m_SizeY * m_SizeX;
	for (S32 i=0 ; i<NbCells ; i++)
	{
		PlaySpace_Surfel *pSurfel = pCell->pFirst;
		while(pSurfel)
		{
			if( GroundZones[pSurfel->ZoneId].m_bIsGround && daBasinZoneIds.Contains(pSurfel->ZoneId) >= 0 )
			{
				pSurfel->SetBasin(TRUE);
				pSurfel->SetNoGameplay(TRUE);
			}

			pSurfel = pSurfel->pNext;
		}
		pCell++;
	}
}

/**************************************************************************/

void	PlaySpace_SurfelBoard::FilterSurfel(Float _EyePos,Float _hGround,Float _hCeilling)
{
	// Compute space availability on the surfel
	PlaySpace_CellInfos * pCell = m_CellBoard.GetArrayPtr();
	for (S32 z=0 ; z<m_SizeY ; z++)
	{
		for (S32 x=0 ; x<m_SizeX ; x++)
		{
			PlaySpace_Surfel *pSurfel = pCell->pFirst;
			
			// find local ceilling
			PlaySpace_Surfel *pLocalCeillingSurfel = NULL;
			while (pSurfel)
			{
				// check ceilling
				if( pSurfel->iDir == PlaySpace_Surfel::SURFDIR_DOWN 
					&& ( !pLocalCeillingSurfel || pSurfel->Point.y > pLocalCeillingSurfel->Point.y ))
				{
					pLocalCeillingSurfel = pSurfel;
				}

				// Next.
				pSurfel = pSurfel->pNext;
			}
			
			// surfel flags
			pSurfel = pCell->pFirst;
			while (pSurfel)
			{
				switch(pSurfel->iDir)
				{
				case PlaySpace_Surfel::SURFDIR_UP :
					{
						FilterUpSurfel(pCell,pSurfel,pLocalCeillingSurfel,_EyePos);
						break;
					}
				case PlaySpace_Surfel::SURFDIR_DOWN :
					{
						FilterDownSurfel(pCell,pSurfel);
						break;
					}
				default :
					{
						FilterSideSurfel(x,z,pSurfel,_EyePos,_hGround,_hCeilling);
						break;
					}
				}
				
				// Next.
				pSurfel = pSurfel->pNext;
			}
			// Next.
			pCell++;
		}
	}
}

/**************************************************************************/

void	PlaySpace_SurfelBoard::FilterUpSurfel(PlaySpace_CellInfos * _pCell, PlaySpace_Surfel * _pSurfel, PlaySpace_Surfel * _pLocalCeillingSurfel, Float _fEyeLimit)
{
	if( _pSurfel->Point.y > _fEyeLimit )
	{
		_pSurfel->SetNoGameplay(TRUE);
	}
	else
	{
		// check all other surfel to find an upper one
		PlaySpace_Surfel *pOtherSurfel = _pCell->pFirst;
		while(pOtherSurfel)
		{
			if( _pSurfel != pOtherSurfel )
			{

				Bool bIsUnderGround = FALSE;

				switch(pOtherSurfel->iDir)
				{
				case PlaySpace_Surfel::SURFDIR_UP :
					{
						if( pOtherSurfel != _pLocalCeillingSurfel 
							&& pOtherSurfel->Point.y < _fEyeLimit
							&& _pSurfel->Point.y < pOtherSurfel->Point.y )
						{
							bIsUnderGround = TRUE;
						}
						break;
					}
				case PlaySpace_Surfel::SURFDIR_DOWN :
				{
					if (   (_pSurfel->Point.y < pOtherSurfel->Point.y)
						&& ((pOtherSurfel->Point.y - _pSurfel->Point.y) < 0.3f)
						)
					{
						bIsUnderGround = TRUE;
					}
					break;
				}
				default :
					{
/*						// Already done at creation
						if( _pSurfel->Point.y < pOtherSurfel->Point.y
							&& pOtherSurfel->Point.y - _pSurfel->Point.y < m_SizeVoxel )
						{
							// compute occupation tolerance
							Float distToOtherSurfel = (_pSurfel->Point - pOtherSurfel->Point) * pOtherSurfel->Normal;
							if( distToOtherSurfel < m_SizeVoxel * .25f )
							{
								bIsUnderGround = TRUE;
							}
						}*/
						break;
					}
				}

				if( bIsUnderGround )
				{
					_pSurfel->SetNoGameplay(TRUE);
					break;
				}
			}

			pOtherSurfel = pOtherSurfel->pNext;
		}
	}
}

/**************************************************************************/

void	PlaySpace_SurfelBoard::FilterDownSurfel(PlaySpace_CellInfos * _pCell, PlaySpace_Surfel * _pSurfel)
{
	// check all other surfel to find an upper one
	PlaySpace_Surfel *pOtherSurfel = _pCell->pFirst;
	while(pOtherSurfel)
	{
		if( _pSurfel != pOtherSurfel
				&& pOtherSurfel->iDir == PlaySpace_Surfel::SURFDIR_DOWN
				&& _pSurfel->Point.y < pOtherSurfel->Point.y )
		{
			_pSurfel->SetNoGameplay(TRUE);
			break;
		}

		pOtherSurfel = pOtherSurfel->pNext;
	}
}

/**************************************************************************/

void	PlaySpace_SurfelBoard::FilterSideSurfel(S32 _x, S32 _z, PlaySpace_Surfel * _pSurfel, Float _fEyeLimit,Float _hGround,Float _hCeilling)
{
	// Note: Les filter ne fonctionnent pas bien avec des surfaces penchées. :(
	// Procéder zone par zone pour les murs ?
	// Regarder les liasons entre zones ?
	// + Filtres classiques... bien sûr.
	static S32 nbCellMaxToUnderground = 6;

	// Other Filters.

	switch(_pSurfel->iDir)
	{
	case PlaySpace_Surfel::SURFDIR_LEFT :
		{
			DoFilterSideSurfel(_pSurfel,_fEyeLimit,_z * m_SizeX + _x - 1,_x,PlaySpace_Surfel::SURFDIR_RIGHT,-1);
			DoFilterSideUndergroundSurfel(_pSurfel,_z * m_SizeX + _x + 1,Min(m_SizeX - _x - 1,nbCellMaxToUnderground),PlaySpace_Surfel::SURFDIR_RIGHT,1);
			DoFilterSideProximitySurfel(_pSurfel, _z * m_SizeX + _x + 1, m_SizeX - _x - 1, 1,_hGround,_hCeilling);
			break;
		}
	case PlaySpace_Surfel::SURFDIR_RIGHT :
		{
			DoFilterSideSurfel(_pSurfel,_fEyeLimit,_z * m_SizeX + _x + 1,m_SizeX - _x - 1,PlaySpace_Surfel::SURFDIR_LEFT,1);
			DoFilterSideUndergroundSurfel(_pSurfel,_z * m_SizeX + _x - 1,Min(_x,nbCellMaxToUnderground),PlaySpace_Surfel::SURFDIR_LEFT,-1);
			DoFilterSideProximitySurfel(_pSurfel, _z * m_SizeX + _x - 1, _x, -1,_hGround,_hCeilling);
			break;
		}
	case PlaySpace_Surfel::SURFDIR_FRONT :
		{
			DoFilterSideSurfel(_pSurfel,_fEyeLimit,_x + m_SizeX * (_z - 1),_z,PlaySpace_Surfel::SURFDIR_BACK,-m_SizeX);
			DoFilterSideUndergroundSurfel(_pSurfel, _x + m_SizeX * (_z + 1), Min(m_SizeY - _z - 1, nbCellMaxToUnderground), PlaySpace_Surfel::SURFDIR_BACK, m_SizeX);
			DoFilterSideProximitySurfel(_pSurfel, _x + m_SizeX * (_z + 1), m_SizeY - _z - 1, m_SizeX,_hGround,_hCeilling);
			break;
		}
	case PlaySpace_Surfel::SURFDIR_BACK :
		{
			DoFilterSideSurfel(_pSurfel,_fEyeLimit,_x + m_SizeX * (_z + 1),m_SizeY - _z - 1,PlaySpace_Surfel::SURFDIR_FRONT,m_SizeX);
			DoFilterSideUndergroundSurfel(_pSurfel, _x + m_SizeX * (_z - 1), Min(_z, nbCellMaxToUnderground), PlaySpace_Surfel::SURFDIR_FRONT, -m_SizeX);
			DoFilterSideProximitySurfel(_pSurfel, _x + m_SizeX * (_z - 1), _z, -m_SizeX,_hGround,_hCeilling);
			break;
		}
	}
}

/**************************************************************************/

void	PlaySpace_SurfelBoard::DoFilterSideSurfel(PlaySpace_Surfel * _pSurfel, Float _fEyeLimit, S32 _startIdx, S32 _nbCell, S8 _reverseDir, S32 _delta)
{
	const Vec3f &CurDir = PlaySpace_Surfel::iDir2Normal(_pSurfel->iDir);
	PlaySpace_CellInfos * pCell = m_CellBoard.GetArrayPtr() + _startIdx;
	Bool bIsSide = FALSE;
	for(S32 i=0;i<_nbCell && !bIsSide;i++)
	{
		PlaySpace_Surfel *pOtherSurfel = pCell->pFirst;
		while(pOtherSurfel)
		{
			Vec3f vSurfelToOther = pOtherSurfel->Point - _pSurfel->Point;
			vSurfelToOther.y = 0;
			Float dist = vSurfelToOther.GetNorm();
			if( dist > m_SizeCell * .5f )
			{
				if( ( pOtherSurfel->iDir == _pSurfel->iDir ||  pOtherSurfel->iDir == _reverseDir )
					&& _pSurfel->Point.y <= pOtherSurfel->Point.y 
					&& (_pSurfel->ZoneId != pOtherSurfel->ZoneId)
					)
				{
					if (Abs((CurDir*_pSurfel->Point) - (CurDir*pOtherSurfel->Point)) > 0.12f)
					{
						//DRAW_DEBUG_SPHERE3D(_pSurfel->Point, COLOR_RED, 0.02f, .displayDuration(100000.f));
						//DRAW_DEBUG_LINE3D(_pSurfel->Point,pOtherSurfel->Point,COLOR_RED,0.005f,.displayDuration(100000.f));
						bIsSide = TRUE;
						break;
					}
				}
				else if( pOtherSurfel->iDir == PlaySpace_Surfel::SURFDIR_UP || pOtherSurfel->iDir == PlaySpace_Surfel::SURFDIR_DOWN )
				{
					if (pOtherSurfel->Point.y < _fEyeLimit && pOtherSurfel->Point.y >= _pSurfel->Point.y && (_pSurfel->ZoneId != pOtherSurfel->ZoneId))
					{
						//DRAW_DEBUG_SPHERE3D(_pSurfel->Point, COLOR_GREEN, 0.02f, .displayDuration(100000.f));
						//DRAW_DEBUG_LINE3D(_pSurfel->Point, pOtherSurfel->Point, COLOR_GREEN, 0.005f, .displayDuration(100000.f));
						bIsSide = TRUE;
						break;
					}
				}
			}

			pOtherSurfel = pOtherSurfel->pNext;
		}

		pCell+=_delta;
	}

	if( bIsSide )
	{
		_pSurfel->SetNoGameplay(TRUE);
	}
}

/**************************************************************************/

void	PlaySpace_SurfelBoard::DoFilterSideProximitySurfel(PlaySpace_Surfel * _pSurfel, S32 _startIdx, S32 _nbCell, S32 _delta,Float _hGround,Float _hCeilling)
{
	if (!_nbCell)
		return;

	const Float	DistMax = 0.4f;

	// Search Up Down.
	PlaySpace_CellInfos * pCell = m_CellBoard.GetArrayPtr() + _startIdx;

	Float MyY = _pSurfel->Point.y;

	if ((_hCeilling - MyY) < DistMax)
	{
		_pSurfel->SetNoGameplay(TRUE);
		return;
	}
	if ((MyY - _hGround) < DistMax)
	{
		_pSurfel->SetNoGameplay(TRUE);
		return;
	}

	PlaySpace_Surfel *pOtherSurfel = pCell->pFirst;
	Float MyGround = -1e8f;
	Float MyCeiling = 1e8f;
	for (S32 i = 0; i<_nbCell ; i++)
	{
		PlaySpace_Surfel *pOtherSurfel = pCell->pFirst;
		while (pOtherSurfel)
		{
			if (	(pOtherSurfel->iDir == PlaySpace_Surfel::SURFDIR_UP)
				||  (pOtherSurfel->iDir == PlaySpace_Surfel::SURFDIR_DOWN)
				)
			{
				if (pOtherSurfel->Point.y < MyY)
				{
					// Ground.
					if (pOtherSurfel->Point.y > MyGround)
					{
						MyGround = pOtherSurfel->Point.y;
						if ((MyY - MyGround) < DistMax)
						{
							_pSurfel->SetNoGameplay(TRUE);
							return;
						}
					}
				}
				else
				{
					// Ceiling.
					if (pOtherSurfel->Point.y < MyCeiling)
					{
						MyCeiling = pOtherSurfel->Point.y;
						if ((MyCeiling - MyY) < DistMax)
						{
							_pSurfel->SetNoGameplay(TRUE);
							return;
						}

						// Trié donc rien au dessus de plus bas.
						break;
					}
				}
			}
			pOtherSurfel = pOtherSurfel->pNext;
		}

		// Done ?
		if (MyGround > -1e7f)
			break;
		if (MyCeiling < 1e7f)
			break;

		// Next Column.
		pCell += _delta;
	}
}

/**************************************************************************/

void	PlaySpace_SurfelBoard::DoFilterSideUndergroundSurfel(PlaySpace_Surfel * _pSurfel, S32 _startIdx, S32 _nbCell, S8 _reverseDir, S32 _delta)
{
	PlaySpace_CellInfos * pCell = m_CellBoard.GetArrayPtr() + _startIdx;
	Bool bIsUnderground = FALSE;
	for(S32 i=0;i<_nbCell && !bIsUnderground;i++)
	{
		PlaySpace_Surfel *pOtherSurfel = pCell->pFirst;
		while(pOtherSurfel)
		{
			if( ( (pOtherSurfel->iDir == _pSurfel->iDir || pOtherSurfel->iDir == _reverseDir)
				&& POS_Z( _pSurfel->Point.y - pOtherSurfel->Point.y ) < m_SizeCell * .5f ) 
				&& (_pSurfel->ZoneId != pOtherSurfel->ZoneId)
				)
			{
				bIsUnderground = TRUE;
				break;
			}

			pOtherSurfel = pOtherSurfel->pNext;
		}

		pCell+=_delta;
	}

	if( bIsUnderground )
	{
		_pSurfel->SetNoGameplay(TRUE);
	}
}

/**************************************************************************/

S32		PlaySpace_SurfelBoard::GetSurfelByIDistAndIDir(S32 _x, S32 _y, S32 _z, S32 _NbCell, S32 _iDir, PlaySpace_Surfel **_TabResult, S32 _NbMax)
{
	S32	MinX = _x - _NbCell;
	S32	MinY = _y - _NbCell;
	S32	MinZ = _z - _NbCell;

	S32	MaxX = _x + _NbCell;
	S32	MaxY = _y + _NbCell;
	S32	MaxZ = _z + _NbCell;

	MinX = Max(MinX, (S32)0);
	MaxX = Min(MaxX, m_SizeX - 1);

	MinZ = Max(MinZ, (S32)0);
	MaxZ = Min(MaxZ, m_SizeY - 1);

	S32		CurNb = 0;
	S32 DeltaPos = MinZ * m_SizeX;
	for (S32 z = MinZ; z <= MaxZ; z++)
	{
		PlaySpace_CellInfos	*pCurCell = &(m_CellBoard[DeltaPos + MinX]);
		for (S32 x = MinX; x <= MaxX; x++)
		{
			PlaySpace_Surfel	*pCur = pCurCell->pFirst;
			while (pCur)
			{
				if ((pCur->y >= MinY) && (pCur->y <= MaxY) && (pCur->iDir == _iDir))
				{
					_TabResult[CurNb++] = pCur;
					if (CurNb >= _NbMax)
						return CurNb;
				}
				pCur = pCur->pNext;
			}
			pCurCell++;
		}
		DeltaPos += m_SizeX;
	}

	return CurNb;
}

/**************************************************************************/

S32		PlaySpace_SurfelBoard::GetSurfelByIDistAndDirWall(S32 _x, S32 _y, S32 _z, S32 _NbCell, Vec3f& _norm, Float _cosMax, Float _cosMaxTolerance, PlaySpace_Surfel **_TabResult, S32 _NbMax)
{
	S32	MinX = _x - _NbCell;
	S32	MinY = _y - _NbCell;
	S32	MinZ = _z - _NbCell;

	S32	MaxX = _x + _NbCell;
	S32	MaxY = _y + _NbCell;
	S32	MaxZ = _z + _NbCell;

	MinX = Max(MinX, (S32)0);
	MaxX = Min(MaxX, m_SizeX - 1);

	MinZ = Max(MinZ, (S32)0);
	MaxZ = Min(MaxZ, m_SizeY - 1);

	S32		CurNb = 0;
	S32 DeltaPos = MinZ * m_SizeX;
	for (S32 z = MinZ; z <= MaxZ; z++)
	{
		PlaySpace_CellInfos	*pCurCell = &(m_CellBoard[DeltaPos + MinX]);
		for (S32 x = MinX; x <= MaxX; x++)
		{
			PlaySpace_Surfel	*pCur = pCurCell->pFirst;
			S32 CurNbSaved = CurNb;
			Bool isValid = FALSE;
			while (pCur)
			{
				if ((pCur->y >= MinY) && (pCur->y <= MaxY))
				{
					Float dot = pCur->Normal * _norm;
					if (dot > _cosMaxTolerance)
					{
						if (dot > _cosMax)
							isValid = TRUE;

						EXCEPTIONC_Z(CurNb < _NbMax, "Big Badaboum");
						if (CurNb < _NbMax)
							_TabResult[CurNb++] = pCur;
					}
				}
				pCur = pCur->pNext;
			}
			if (!isValid)
				CurNb = CurNbSaved;

			pCurCell++;
		}
		DeltaPos += m_SizeX;
	}

	return CurNb;
}

/**************************************************************************/

S32		PlaySpace_SurfelBoard::GetNbZoneId()
{
	S32	ZoneMax = 0;
	S32 NbCell = m_SizeX*m_SizeY;
	PlaySpace_CellInfos *pCell = m_CellBoard.GetArrayPtr();
	for (S32 i = 0; i<NbCell; i++)
	{
		// Init.
		PlaySpace_Surfel	*pCur = pCell->pFirst;
		while (pCur)
		{
			ZoneMax = Max(ZoneMax, (S32)pCur->ZoneId);
			pCur = pCur->pNext;
		}

		// Next.
		pCell++;
	}
	ZoneMax++;
	return ZoneMax;
}

/**************************************************************************/

S32		PlaySpace_SurfelBoard::SearchHorizontalLimit(Float &_Height, S32 &_Nb, Bool _Up)
{
	Float				ZoneHeight[CONEXITY_MAX_ZONE];
	S32					ZoneNb[CONEXITY_MAX_ZONE];

	// Init.
	for (S32 i = 0; i<CONEXITY_MAX_ZONE; i++)
	{
		ZoneHeight[i] = 0.f;
		ZoneNb[i] = 0;
	}

	// Compute Height.
	PlaySpace_Surfel::SurfDir	Dir;
	if (_Up)
		Dir = PlaySpace_Surfel::SURFDIR_DOWN;
	else
		Dir = PlaySpace_Surfel::SURFDIR_UP;

	S32	ZoneMax = -1;
	S32 NbCell = m_SizeX*m_SizeY;
	PlaySpace_CellInfos *pCell = m_CellBoard.GetArrayPtr();
	for (S32 i = 0; i<NbCell; i++)
	{
		// Init.
		PlaySpace_Surfel	*pCur = pCell->pFirst;
		while (pCur)
		{
			if (	(pCur->ZoneId >= 0)
				&& (pCur->iDir == Dir)
				&& (!(pCur->Flags & PlaySpace_Surfel::SURFFLAG_VIRTUAL))
				)
			{
				ZoneHeight[pCur->ZoneId] += pCur->Point.y;
				ZoneNb[pCur->ZoneId]++;

				if (pCur->ZoneId > ZoneMax)
					ZoneMax = pCur->ZoneId;
			}

			// Next
			pCur = pCur->pNext;
		}

		// Next.
		pCell++;
	}

	// Compute Extrema.
	Float	Extrema = -1e6f;
	if (_Up)
		Extrema = -1e6f;
	else
		Extrema = 1e6f;

	for (S32 i = 0; i<=ZoneMax; i++)
	{
		if (!ZoneNb[i])
			continue;

		Float H = ZoneHeight[i] / ZoneNb[i];
		ZoneHeight[i] = H;

		if (ZoneNb[i] < 100)
			continue;

		if (_Up)
			Extrema = Max(Extrema,H);
		else
			Extrema = Min(Extrema, H);
	}

	// Compute Ground Or Ceiling.
	S32		GroupId = -1;
	Float	BestH = -1e6f;
	S32		BestNb = -1;

	for (S32 i = 0; i<=ZoneMax; i++)
	{
		if (ZoneNb[i] < 100)
			continue;
		if (_Up)
		{
			if (ZoneHeight[i] < (Extrema - 1.f))
				continue;
		}
		else
		{
			if (ZoneHeight[i] > (Extrema + 1.f))
				continue;
		}
		// Set New Result.
		if (ZoneNb[i] > BestNb)
		{
			GroupId = i;
			BestH = ZoneHeight[i];
			BestNb = ZoneNb[i];
		}
	}

	if (GroupId >= 0)
	{
		_Height = BestH;
		_Nb = BestNb;
	}
	return GroupId;
}

/**************************************************************************/

void	PlaySpace_SurfelBoard::ComputeConexityOld(Bool _FilterZonePlane)
{
	S32 NbCell = m_SizeX*m_SizeY;
	PlaySpace_CellInfos *pCell = m_CellBoard.GetArrayPtr();

	// Init ZoneId.
	for (S32 i = 0; i<NbCell; i++)
	{
		// Init.
		PlaySpace_Surfel	*pCur = pCell->pFirst;
		while (pCur)
		{
			pCur->ZoneId = -1;
			pCur = pCur->pNext;
		}

		// Next.
		pCell++;
	}

	// Compute ZoneId.
	PlaySpace_Surfel	*TabLinkSurfel[256];
	PlaySpace_Surfel	*TabSurfel[CONEXITY_MAX_SURF];
	Vec3f				ZoneNormal[CONEXITY_MAX_ZONE];
	Float				ZoneDist[CONEXITY_MAX_ZONE];
	S32					ZoneNb[CONEXITY_MAX_ZONE];
	S32					CurGlobalZoneId = 0;

	pCell = m_CellBoard.GetArrayPtr();
	for (S32 i = 0; i<NbCell; i++)
	{
		// Init.
		PlaySpace_Surfel	*pHead = pCell->pFirst;
		while (pHead)
		{
			if (pHead->ZoneId < 0)
			{
				// compute this Zone...
				S32 NbSurfel = 0;

				pHead->ZoneId = CurGlobalZoneId;	// Dont RE-check this tile.
				TabSurfel[NbSurfel++] = pHead;

				Vec3f	SumNormals = pHead->Normal;
				Float	SumDelta = pHead->Normal * pHead->Point;
				S32		NbCellInZone = 1;

				Vec3f	CurNormal = SumNormals;
				Float	CurDelta = SumDelta;

				while (NbSurfel)
				{
					// UnStack.
					NbSurfel--;
					PlaySpace_Surfel *pCur = TabSurfel[NbSurfel];

					// Check Voisins.
					S32 NbLinkedSurfel = GetSurfelByIDistAndIDir(pCur->x, pCur->y, pCur->z, 1, pCur->iDir, TabLinkSurfel, 256);

					for (S32 j = 0; j<NbLinkedSurfel; j++)
					{
						// Same surface ?
						PlaySpace_Surfel *pOther = TabLinkSurfel[j];
						if (pOther->ZoneId >= 0)
							continue;

						// orientation 
						Float DotNormal = CurNormal * pOther->Normal;
						if (DotNormal < 0.9f)
							continue;

						Float OtherDelta = CurNormal * pOther->Point;
						if (Abs(OtherDelta - CurDelta) >= 0.15f)
							continue;

						// Delta.
						SumNormals += pOther->Normal;
						SumDelta += OtherDelta;
						NbCellInZone++;

						CurNormal = SumNormals;
						CurDelta = SumDelta;

						CurNormal.CNormalize();
						CurDelta /= (Float)NbCellInZone;

						// Yes, Add it !
						pOther->ZoneId = CurGlobalZoneId;
						TabSurfel[NbSurfel++] = pOther;
						EXCEPTIONC_Z(NbSurfel<CONEXITY_MAX_SURF, "Too Many Surfel in ZONE SCAN");
					}
				}

				// Next.
				EXCEPTIONC_Z(CurGlobalZoneId<CONEXITY_MAX_ZONE, "Too Many ZONE ID SCAN");
				ZoneNormal[CurGlobalZoneId] = CurNormal;
				CurGlobalZoneId++;
			}

			// Next
			pHead = pHead->pNext;
		}

		// Next.
		pCell++;
	}

	// Calcul du point le plus bas de chaque zone.

	// Filtering Zone => Plane.
	if (_FilterZonePlane)
	{
		// Init.
		for (S32 i = 0; i<CurGlobalZoneId; i++)
		{
			ZoneDist[i] = 0.f;
			ZoneNb[i] = 0;
		}

		// Compute Plane.
		pCell = m_CellBoard.GetArrayPtr();
		for (S32 i = 0; i<NbCell; i++)
		{
			// Init.
			PlaySpace_Surfel	*pCur = pCell->pFirst;
			while (pCur)
			{
				if (pCur->ZoneId < 0)
					continue;

				ZoneDist[pCur->ZoneId] += pCur->Point * ZoneNormal[pCur->ZoneId];
				ZoneNb[pCur->ZoneId]++;

				// Next
				pCur = pCur->pNext;
			}

			// Next.
			pCell++;
		}

		// Average Dist.
		for (S32 i = 0; i<CurGlobalZoneId; i++)
		{
			ZoneDist[i] /= (Float)ZoneNb[i];
		}

		// Set Point !
		pCell = m_CellBoard.GetArrayPtr();
		for (S32 i = 0; i<NbCell; i++)
		{
			// Init.
			PlaySpace_Surfel	*pCur = pCell->pFirst;
			while (pCur)
			{
				if (pCur->ZoneId < 0)
					continue;

				Float	DistToPlan = ZoneDist[pCur->ZoneId] - (ZoneNormal[pCur->ZoneId] * pCur->Point);
				pCur->Point += ZoneNormal[pCur->ZoneId] * DistToPlan;
				pCur->Normal = ZoneNormal[pCur->ZoneId];

				// Next
				pCur = pCur->pNext;
			}

			// Next.
			pCell++;
		}
	}
}

/**************************************************************************/

void PlaySpace_SurfelBoard::CreateSurfelsFromMesh(Playspace_Mesh &_Mesh)
{
	_Mesh.ComputePointsLinks();
	_Mesh.ComputeFacesToolNormal();
	_Mesh.ComputePointsToolNormal();

	Empty();

	S32 nb, NbFaces = _Mesh.m_TabQuad.GetSize();
	Vec3f *TabPoints = _Mesh.m_TabPoints.GetArrayPtr();
	Playspace_Mesh::ToolFaceNormal *TabFaceNormal = _Mesh.m_TabFaceToolNormal.GetArrayPtr();
	Playspace_Mesh::Face *curFace = _Mesh.m_TabQuad.GetArrayPtr();

	Vec3f	pMiddle, vNormal, p[4], center;
	Float	invSize = 1.0f / m_SizeCell;
	Float	CellTri = (m_SizeCell * m_SizeCell)*0.4f;
	Float	CellQuad = (m_SizeCell * m_SizeCell)*0.8;
	Float	CellBig = (m_SizeCell * m_SizeCell)*1.1f;

	for (S32 i = 0; i < NbFaces; i++)
	{
		Bool	IsOkToAdd = FALSE;
		Bool	NeedSubdivide = FALSE;
		Bool	bNoGamePlay = FALSE;
		U8		noGPReason = 0;

		Playspace_Mesh::ToolFaceNormal &FInfos = TabFaceNormal[i];
		if (FInfos.Surface < 0.0005f)
		{
			curFace++;
			continue;
		}

		vNormal = FInfos.Normal;
		p[0] = TabPoints[curFace->TabPoints[0]];
		p[1] = TabPoints[curFace->TabPoints[1]];
		p[2] = TabPoints[curFace->TabPoints[2]];
		nb = (curFace->IsTri ? 3 : 4);
		if (curFace->IsTri)
			pMiddle = (p[0] + p[1] + p[2]) * 0.333333333f;
		else
		{
			p[3] = TabPoints[curFace->TabPoints[3]];
			pMiddle = (p[0] + p[1] + p[2] + p[3]) * 0.25f;
		}
		center.x = (FLOOR((pMiddle.x - m_Origin.x) * invSize) + 0.5f) * m_SizeCell + m_Origin.x;
		center.y = (FLOOR((pMiddle.y - m_Origin.y) * invSize) + 0.5f) * m_SizeCell + m_Origin.y;
		center.z = (FLOOR((pMiddle.z - m_Origin.z) * invSize) + 0.5f) * m_SizeCell + m_Origin.z;

		if (((curFace->IsTri) && (FInfos.Surface >= CellTri)) || (FInfos.Surface >= CellQuad))
		{
			IsOkToAdd = TRUE;
			if (FInfos.Surface > CellBig)
				NeedSubdivide = TRUE;
		}
		else
			noGPReason = ResolveFaceInCell(_Mesh,i, center, IsOkToAdd, bNoGamePlay);

		if (IsOkToAdd)
		{
			Float Delta = (center - pMiddle) * vNormal;
			Vec3f ProjPos = center - vNormal*Delta;			// Project center of cell on Surfel.

			Vec3i SurfeliPos;
			SurfeliPos.x = (S32)((ProjPos.x - m_Origin.x) * invSize);
			SurfeliPos.y = (S32)((ProjPos.y - m_Origin.y) * invSize);
			SurfeliPos.z = (S32)((ProjPos.z - m_Origin.z) * invSize);

			SurfeliPos.x = Min<S32>(SurfeliPos.x, m_SizeX - 1);
			SurfeliPos.x = Max<S32>(SurfeliPos.x, 0);
			SurfeliPos.y = Min<S32>(SurfeliPos.y, m_SizeH - 1);
			SurfeliPos.y = Max<S32>(SurfeliPos.y, 0);
			SurfeliPos.z = Min<S32>(SurfeliPos.z, m_SizeY - 1);
			SurfeliPos.z = Max<S32>(SurfeliPos.z, 0);

			S32 iDir = PlaySpace_Surfel::Normal2iDir(vNormal);

			if (curFace->IsPaintMode == 1)
				bNoGamePlay = TRUE;
			TryAddSurfel(SurfeliPos, iDir, ProjPos, vNormal, curFace->IsVirtual, bNoGamePlay, FALSE, curFace->IsExternal);
			if (NeedSubdivide)
			{
				for (S32 j = 0; j < nb; j++)
				{
					Vec3f pMiddle2;
					if (curFace->IsTri)
						pMiddle2 = (pMiddle + p[j]) * 0.5f;
					else
						pMiddle2 = (2.0f * p[j] + pMiddle) * 0.333333333f;

					Vec3f	SnapPos2;
					SnapPos2.x = (FLOOR((pMiddle2.x - m_Origin.x) * invSize) + 0.5f) * m_SizeCell + m_Origin.x;		// get the center of the voxel
					SnapPos2.y = (FLOOR((pMiddle2.y - m_Origin.y) * invSize) + 0.5f) * m_SizeCell + m_Origin.y;
					SnapPos2.z = (FLOOR((pMiddle2.z - m_Origin.z) * invSize) + 0.5f) * m_SizeCell + m_Origin.z;

					Delta = (SnapPos2 - pMiddle2) * vNormal;		// project the center of the voxel onto current face
					ProjPos = SnapPos2 - vNormal * Delta;

					Vec3i SurfeliPos2;
					SurfeliPos2.x = FLOORINT((ProjPos.x - m_Origin.x) * invSize);
					SurfeliPos2.y = FLOORINT((ProjPos.y - m_Origin.y) * invSize);
					SurfeliPos2.z = FLOORINT((ProjPos.z - m_Origin.z) * invSize);

					SurfeliPos2.x = Min<S32>(SurfeliPos2.x, m_SizeX - 1);
					SurfeliPos2.x = Max<S32>(SurfeliPos2.x, 0);
					SurfeliPos2.y = Min<S32>(SurfeliPos2.y, m_SizeH - 1);
					SurfeliPos2.y = Max<S32>(SurfeliPos2.y, 0);
					SurfeliPos2.z = Min<S32>(SurfeliPos2.z, m_SizeY - 1);
					SurfeliPos2.z = Max<S32>(SurfeliPos2.z, 0);

					if ((SurfeliPos2.x != SurfeliPos.x) || (SurfeliPos2.y != SurfeliPos.y) || (SurfeliPos2.z != SurfeliPos.z))
					{
						noGPReason = ResolveFaceInCell(_Mesh,i, SnapPos2, IsOkToAdd, bNoGamePlay);
						if (IsOkToAdd)
						{
							if (curFace->IsPaintMode == 1)
								bNoGamePlay = TRUE;
							TryAddSurfel(SurfeliPos2, iDir, ProjPos, vNormal, curFace->IsVirtual, bNoGamePlay, FALSE, curFace->IsExternal);
						}
					}
				}
			}
		}
		curFace++;		// Next face
	}
}

/**************************************************************************/

void PlaySpace_SurfelBoard::CreateSurfelsFromMesh_fast(Playspace_Mesh &_Mesh)
{
	Float	CellSurface = m_SizeCell * m_SizeCell * 1.02f;

	Empty();
	_Mesh.ComputeFacesToolNormal();

	S32 NbFaces = _Mesh.m_TabQuad.GetSize();
	Vec3f *TabPoints = _Mesh.m_TabPoints.GetArrayPtr();
	Playspace_Mesh::ToolFaceNormal *pFaceNormal = _Mesh.m_TabFaceToolNormal.GetArrayPtr();
	Playspace_Mesh::Face *pFaces = _Mesh.m_TabQuad.GetArrayPtr();

	Vec3i SurfeliPos;
	Vec3f	SnapPos2;
	Vec3i otherSurfeliPos[4];
	Vec3i* pSurfeliPos2;

	Vec3f	pMiddle, cellCenter, vNormal;
	Float	invSize = 1.0f / m_SizeCell;

	for (S32 i = 0; i < NbFaces; i++)
	{
		if (pFaceNormal->Surface < 4.e-4f)
		{
			pFaceNormal++;
			pFaces++;
			continue;
		}

		vNormal = pFaceNormal->Normal;
		pMiddle = pFaceNormal->Center;
		S32 nb = pFaces->IsTri ? 3 : 4;

		SurfeliPos.x = (S32)((pMiddle.x - m_Origin.x) * invSize);
		SurfeliPos.y = (S32)((pMiddle.y - m_Origin.y) * invSize);
		SurfeliPos.z = (S32)((pMiddle.z - m_Origin.z) * invSize);

		if (SurfeliPos.x < 0)
			SurfeliPos.x = 0;
		else if (SurfeliPos.x >= m_SizeX)
			SurfeliPos.x = m_SizeX - 1;

		if (SurfeliPos.y < 0)
			SurfeliPos.y = 0;
		else if (SurfeliPos.y >= m_SizeH)
			SurfeliPos.y = m_SizeH - 1;

		if (SurfeliPos.z < 0)
			SurfeliPos.z = 0;
		else if (SurfeliPos.z >= m_SizeY)
			SurfeliPos.z = m_SizeY - 1;

		cellCenter.x = (SurfeliPos.x + 0.5f) * m_SizeCell + m_Origin.x;
		cellCenter.y = (SurfeliPos.y + 0.5f) * m_SizeCell + m_Origin.y;
		cellCenter.z = (SurfeliPos.z + 0.5f) * m_SizeCell + m_Origin.z;

		Float Delta = (cellCenter - pMiddle) * vNormal;
		Vec3f ProjPos = cellCenter - (vNormal * Delta);			// Project cellCenter on Surfel.

		S32 iDir = PlaySpace_Surfel::Normal2iDir(vNormal);
		TryAddSurfel(SurfeliPos, iDir, ProjPos, vNormal, pFaces->IsVirtual, (pFaces->IsPaintMode == 1), FALSE, pFaces->IsExternal);

		if (pFaceNormal->Surface > CellSurface)
		{
			pSurfeliPos2 = otherSurfeliPos;
			for (S32 j = 0; j < nb; j++)
			{
				Vec3f p = TabPoints[pFaces->TabPoints[j]];
				Vec3f pMiddle2 = ((7.0f * p) + pMiddle) * 0.125f;

				pSurfeliPos2->x = (S32)((pMiddle2.x - m_Origin.x) * invSize);
				pSurfeliPos2->y = (S32)((pMiddle2.y - m_Origin.y) * invSize);
				pSurfeliPos2->z = (S32)((pMiddle2.z - m_Origin.z) * invSize);

				if (pSurfeliPos2->x < 0)
					pSurfeliPos2->x = 0;
				else if (pSurfeliPos2->x >= m_SizeX)
					pSurfeliPos2->x = m_SizeX - 1;

				if (pSurfeliPos2->y < 0)
					pSurfeliPos2->y = 0;
				else if (pSurfeliPos2->y >= m_SizeH)
					pSurfeliPos2->y = m_SizeH - 1;

				if (pSurfeliPos2->z < 0)
					pSurfeliPos2->z = 0;
				else if (pSurfeliPos2->z >= m_SizeY)
					pSurfeliPos2->z = m_SizeY - 1;

				if ((pSurfeliPos2->x != SurfeliPos.x) || (pSurfeliPos2->y != SurfeliPos.y) || (pSurfeliPos2->z != SurfeliPos.z))
				{
					Bool alreadyDone = FALSE;
					Vec3i* pSurfeliPos3 = otherSurfeliPos;
					for (S32 k = 0; k < j; k++)
					{
						if ((pSurfeliPos2->x == pSurfeliPos3->x) && (pSurfeliPos2->y == pSurfeliPos3->y) && (pSurfeliPos2->z == pSurfeliPos3->z))
						{
							alreadyDone = TRUE;
							break;
						}
						pSurfeliPos3++;
					}
					if (!alreadyDone)
					{
						SnapPos2.x = (pSurfeliPos2->x + 0.5f) * m_SizeCell + m_Origin.x;		// get the center of the voxel
						SnapPos2.y = (pSurfeliPos2->y + 0.5f) * m_SizeCell + m_Origin.y;
						SnapPos2.z = (pSurfeliPos2->z + 0.5f) * m_SizeCell + m_Origin.z;

						Delta = (SnapPos2 - pMiddle2) * vNormal;		// project the center of the voxel onto current face
						ProjPos = SnapPos2 - (vNormal * Delta);

						TryAddSurfel(*pSurfeliPos2, iDir, ProjPos, vNormal, pFaces->IsVirtual, (pFaces->IsPaintMode == 1), FALSE, pFaces->IsExternal);
					}
				}
				pSurfeliPos2++;
			}
		}
		pFaceNormal++;
		pFaces++;		// Next face
	}
}

/**************************************************************************/

S32 PlaySpace_SurfelBoard::ComputePolyScissorByPlane(S32 nbPoints, Vec3f* points, Vec3f& planeNormal, Vec3f& planePoint, Vec3f* output, S32 maxSizeOutput)
{
	S8 flag = 0, flag_p0;
	S32 idx = 0;
	Float dot_p0, dot;

	dot_p0 = (points[0] - planePoint) * planeNormal;
	if (dot_p0 > 1.0e-3f)
	{
		if (idx >= maxSizeOutput)
		{
			EXCEPTIONC_Z(FALSE, "Error insufficient allocation");
			return idx;
		}
		output[idx++] = points[0];
		flag = 1;
	}
	else if (dot_p0 < -1.0e-3f)
		flag = 2;
	else
	{
		if (idx >= maxSizeOutput)
		{
			EXCEPTIONC_Z(FALSE, "Error insufficient allocation");
			return idx;
		}
		output[idx++] = points[0];
	}

	flag_p0 = flag;

	for (S32 i = 1; i < nbPoints; i++)
	{
		dot = (points[i] - planePoint) * planeNormal;
		if (dot > 1.0e-3f)
		{
			if (flag == 2)
			{
				Vec3f v = points[i] - points[i - 1];
				Float delta = v * planeNormal;
				if (idx >= maxSizeOutput)
				{
					EXCEPTIONC_Z(FALSE, "Error insufficient allocation");
					return idx;
				}
				output[idx++] = points[i] - (v * (dot / delta));
			}
			if (idx >= maxSizeOutput)
			{
				EXCEPTIONC_Z(FALSE, "Error insufficient allocation");
				return idx;
			}
			output[idx++] = points[i];
			flag = 1;
		}
		else if (dot < -1.0e-3f)
		{
			if (flag == 1)
			{
				Vec3f v = points[i] - points[i - 1];
				Float delta = v * planeNormal;
				if (idx >= maxSizeOutput)
				{
					EXCEPTIONC_Z(FALSE, "Error insufficient allocation");
					return idx;
				}
				output[idx++] = points[i] - (v * (dot / delta));
			}
			flag = 2;
		}
		else
		{
			if (idx >= maxSizeOutput)
			{
				EXCEPTIONC_Z(FALSE, "Error insufficient allocation");
				return idx;
			}
			output[idx++] = points[i];
			flag = 0;
		}
	}

	if ((flag == 1) && (flag_p0 == 2))
	{
		Vec3f v = points[0] - points[nbPoints - 1];
		Float delta = v * planeNormal;
		if (idx >= maxSizeOutput)
		{
			EXCEPTIONC_Z(FALSE, "Error insufficient allocation");
			return idx;
		}
		output[idx++] = points[0] - (v * (dot_p0 / delta));
	}
	else if ((flag == 2) && (flag_p0 == 1))
	{
		Vec3f v = points[0] - points[nbPoints - 1];
		Float delta = v * planeNormal;
		if (idx >= maxSizeOutput)
		{
			EXCEPTIONC_Z(FALSE, "Error insufficient allocation");
			return idx;
		}
		output[idx++] = points[0] - (v * (dot_p0 / delta));
	}

	return idx;
}

/**************************************************************************/
Float PlaySpace_SurfelBoard::ComputeTriAreaInAABox(S32 nbPts, Vec3f* p, Vec3f& center, Float demiSize)
{
	S32 nb1, nb2;
	Vec2f tab3[10];
	Vec3f u, v, n, tab1[10], tab2[10];

	v = center + Vec3f(demiSize, 0.0f, 0.0f);
	n = Vec3f(-1.0f, 0.0f, 0.0f);
	nb1 = ComputePolyScissorByPlane(nbPts, p, n, v, tab1, 10);
	if (nb1 < 3)
		return 0.0f;

	v = center + Vec3f(-demiSize, 0.0f, 0.0f);
	n = Vec3f(1.0f, 0.0f, 0.0f);
	nb2 = ComputePolyScissorByPlane(nb1, tab1, n, v, tab2, 10);
	if (nb2 < 3)
		return 0.0f;

	v = center + Vec3f(0.0f, demiSize, 0.0f);
	n = Vec3f(0.0f, -1.0f, 0.0f);
	nb1 = ComputePolyScissorByPlane(nb2, tab2, n, v, tab1, 10);
	if (nb1 < 3)
		return 0.0f;

	v = center + Vec3f(0.0f, -demiSize, 0.0f);
	n = Vec3f(0.0f, 1.0f, 0.0f);
	nb2 = ComputePolyScissorByPlane(nb1, tab1, n, v, tab2, 10);
	if (nb2 < 3)
		return 0.0f;

	v = center + Vec3f(0.0f, 0.0f, demiSize);
	n = Vec3f(0.0f, 0.0f, -1.0f);
	nb1 = ComputePolyScissorByPlane(nb2, tab2, n, v, tab1, 10);
	if (nb1 < 3)
		return 0.0f;

	v = center + Vec3f(0.0f, 0.0f, -demiSize);
	n = Vec3f(0.0f, 0.0f, 1.0f);
	nb2 = ComputePolyScissorByPlane(nb1, tab1, n, v, tab2, 10);
	if (nb2 < 3)
		return 0.0f;

	u = p[1] - p[0];
	u.Normalize();

	v = (u ^ (p[2] - p[0])) ^ (-u);
	v.Normalize();

	for (S32 i = 0; i < nb2; i++)
	{
		tab3[i].x = u * tab2[i];
		tab3[i].y = v * tab2[i];
	}

	Float area = CalcArea2D(tab3, nb2);

	return area;
}

/**************************************************************************/

U8 PlaySpace_SurfelBoard::ResolveFaceInCell(Playspace_Mesh &_Mesh,S32 face, Vec3f& cellCenter, Bool& IsOkToAdd, Bool& bNoGamePlay)
{
	Bool toInsert;
	S32 nb, top = 0;
	SafeArray_Z<S32,128,FALSE,FALSE>	faces;
	Float area = 0.0f;
	const Float demiSize = m_SizeCell * 0.5f;
	const Float smallSize = demiSize * 0.5f;						// to check if the intersection of the planes is far from the center of the cell
	const Float smallSize2 = smallSize * smallSize;
	const Float bigSize = demiSize * 1.02f;							// to check if the intersection of the planes is far from the center of the cell
	const Float MinLength2 = (m_SizeCell * m_SizeCell)*0.16f;		// to check if the other face (not parallel) is far from the center of the cell
	const Float MinSurf = (m_SizeCell * m_SizeCell)*0.25f;			// minimal surface parallel with the face and inside the cell for taking it into account
	const Float MinSurfGP = (m_SizeCell * m_SizeCell)*0.5f;			// minimal surface parallel with the face and inside the cell for taking it into account and GamePlay
	
	Vec3f *TabPoints = _Mesh.m_TabPoints.GetArrayPtr();
	Playspace_Mesh::Face *TabFace = _Mesh.m_TabQuad.GetArrayPtr();
	Playspace_Mesh::ToolFaceNormal *TabFaceNormal = _Mesh.m_TabFaceToolNormal.GetArrayPtr();
	Playspace_Mesh::PointsLinks *TabPointsLinks = _Mesh.m_TabPointsLinks.GetArrayPtr();
	Playspace_Mesh::Face *curFace = &TabFace[face];
	Vec3f vNormal = TabFaceNormal[face].Normal;
	Vec3f middle = TabFaceNormal[face].Center;
	bNoGamePlay = FALSE;
	IsOkToAdd = FALSE;
	U8 reason = 0;
	nb = (curFace->IsTri ? 3 : 4);

	for (S32 j = 0; j < nb; j++)
	{
		S32 nbFaces = TabPointsLinks[curFace->TabPoints[j]].GetNbFaces();		// number of faces
		for (S32 k = 0; k < nbFaces; k++)
		{
			S32 faceIdx = TabPointsLinks[curFace->TabPoints[j]].GetFace(k);
			toInsert = TRUE;
			for (S32 m = 0; m < top; m++)
			{
				if (faces[m] == faceIdx)
				{
					toInsert = FALSE;
					break;
				}
			}
			if (toInsert)
			{
				faces[top++] = faceIdx;

				Bool isCommonEdge = FALSE;
				Vec3f otherPoint;
				S32 nb2 = (TabFace[faceIdx].IsTri ? 3 : 4);
				for (S32 m = 0; m < nb2; m++)
				{
					for (S32 n = 0; n < nb; n++)
					{
						if ((n != j) && (curFace->TabPoints[n] == TabFace[faceIdx].TabPoints[m]))
						{
							isCommonEdge = TRUE;
							otherPoint = TabPoints[curFace->TabPoints[n]];
						}
					}
				}

				Vec3f pts[4];
				pts[0] = TabPoints[TabFace[faceIdx].TabPoints[0]];
				pts[1] = TabPoints[TabFace[faceIdx].TabPoints[1]];
				pts[2] = TabPoints[TabFace[faceIdx].TabPoints[2]];
				if (!TabFace[faceIdx].IsTri)
					pts[3] = TabPoints[TabFace[faceIdx].TabPoints[3]];

				Vec3f OtherCenter = TabFaceNormal[faceIdx].Center;
				Vec3f OtherNormal = TabFaceNormal[faceIdx].Normal;

				Float cosAngle = vNormal * OtherNormal;
				if (cosAngle > 0.97f)
				{
					if (TabFace[faceIdx].IsPaintMode == 1)
						bNoGamePlay = TRUE;	

					if (TabFace[faceIdx].IsTri)
						area += ComputeTriAreaInAABox(3, pts, cellCenter, demiSize);
					else
						area += ComputeTriAreaInAABox(4, pts, cellCenter, demiSize);
				}
				else
				{
					if (isCommonEdge)
					{
						// project the center of the voxel onto current face => CenterSurfel
						Float Delta = (cellCenter - middle) * vNormal;		
						Vec3f CenterSurfel = cellCenter - vNormal * Delta;

						// Compute Intersect Line between 2 faces : LineDir and LineProj.
						Vec3f LineDir = vNormal ^ OtherNormal;
						LineDir.CNormalize();	// Because vNormal and OtherNorma are not perp
						Vec3f vec = LineDir ^ vNormal; // No Normalize...
						Float DistMiddleFromOtherFace = ((middle - OtherCenter) * OtherNormal);
						Vec3f LineProj = middle - (vec * (DistMiddleFromOtherFace / (vec * OtherNormal)));

						// Compute Delta Surfel Center To Line.
						//vec = cellCenter - LineProj;
						vec = CenterSurfel - LineProj;
						Vec3f DeltaCL = vec - LineDir * (vec * LineDir);

						if ((DeltaCL.x * DeltaCL.x < smallSize2)
							&& (DeltaCL.y * DeltaCL.y < smallSize2)
							&& (DeltaCL.z * DeltaCL.z < smallSize2)
							)
						{
							// Compute Other Voxel Center.
							Vec3f	cellCenterOther;
							Float invSize = 1.f / m_SizeCell;
							cellCenterOther.x = (FLOOR((OtherCenter.x - m_Origin.x) * invSize) + 0.5f) * m_SizeCell + m_Origin.x;
							cellCenterOther.y = (FLOOR((OtherCenter.y - m_Origin.y) * invSize) + 0.5f) * m_SizeCell + m_Origin.y;
							cellCenterOther.z = (FLOOR((OtherCenter.z - m_Origin.z) * invSize) + 0.5f) * m_SizeCell + m_Origin.z;

							// project the center of the voxel onto other face => CenterSurfel2
							Delta = (cellCenterOther - OtherCenter) * OtherNormal;
							Vec3f CenterSurfel2 = cellCenterOther - OtherNormal * Delta;

							Float DistOtherFromMe = (CenterSurfel2 - CenterSurfel) * vNormal;
							if (DistOtherFromMe > 0.f)
							{
								// Other is upper from me => block my zone
								bNoGamePlay = TRUE;
								reason = 1;
							}
							else
							{
								// Compute Delta Surfel Center2 To Line.
								vec = CenterSurfel2 - LineProj;
								Vec3f DeltaCL2 = vec - (LineDir * (vec * LineDir));

								if ((DeltaCL2.x * DeltaCL2.x < smallSize2)
									&& (DeltaCL2.y * DeltaCL2.y < smallSize2)
									&& (DeltaCL2.z * DeltaCL2.z < smallSize2)
									)
								{
									bNoGamePlay = TRUE;				// when this projection is 'clearly' inside the cell => NoGamePlay
									reason = 2;
								}
							}
						}
					}
					else
					{
						Vec3f p = TabPoints[curFace->TabPoints[j]];
						if ((p.x > (cellCenter.x - smallSize)) && (p.x < (cellCenter.x + smallSize)) &&
							(p.y >(cellCenter.y - smallSize)) && (p.y < (cellCenter.y + smallSize)) &&
							(p.z >(cellCenter.z - smallSize)) && (p.z < (cellCenter.z + smallSize)))
						{
							bNoGamePlay = TRUE;				// when the only common point is 'clearly' inside the cell => NoGamePlay
							reason = 3;
						}
					}
				}
			}
		}
	}
	if (area > MinSurf)		// if parallel area inside the cell is insufficient => do not add this face
	{
		IsOkToAdd = TRUE;
		if (area < MinSurfGP)
		{
			bNoGamePlay = TRUE;
			reason = 4;
		}
	}
	return reason;
}

/**************************************************************************/

PlaySpace_SurfaceInfosV3::PlaySpace_SurfaceInfosV3()
{
	m_MeshCycleID = 0;
	m_IsInRayMode = TRUE; // par défaut, on est en Ray Mode...
}


/**************************************************************************/

PlaySpace_SurfaceInfosV3::~PlaySpace_SurfaceInfosV3()
{
}


/**************************************************************************/
void	PlaySpace_SurfaceInfosV3::ReInitScan(Playspace_Area &_Area, Float _YGround, Float _YCeiling)
{
	m_MeshCycleID = 0;
	m_MapMesh3D.Init(_Area,_YGround, _YCeiling);
}

/**************************************************************************/

void	PlaySpace_SurfaceInfosV3::ReInitScan(Vec3f &_Min, Vec3f &_Max, Float _YGround, Float _YCeiling, Float _CellSize, S32 _SizeX, S32 _SizeH, S32 _SizeY)
{
	Playspace_Area Area;
	Area.Min = _Min;
	Area.Max = _Max;
	Area.Center = (_Min+_Max) * 0.5f;
	Area.NbCellX = _SizeX;
	Area.NbCellY = _SizeY;
	Area.NbCellH = _SizeH;
	Area.SizeVoxel = _CellSize;
	m_MapMesh3D.Init(Area, _YGround, _YCeiling);
}

/**************************************************************************/

void PlaySpace_SurfaceInfosV3::CreateMesh(Playspace_Mesh &_Mesh,Bool _FastMode,Bool _KeepMode)
{
	m_MapMesh3D.CreateMesh(_Mesh,_FastMode,_KeepMode);
}

/**************************************************************************/
