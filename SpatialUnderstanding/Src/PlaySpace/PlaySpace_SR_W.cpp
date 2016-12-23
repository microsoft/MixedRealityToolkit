// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include <pch.h>
#include <PlaySpace\PlaySpace_SR_W.h>
#include <PlaySpace\PlaySpaceInfos_W.h>

#define		QUALITY_BLIND_1		12
#define		QUALITY_BLIND_2		24

Float		Playspace_SR_W::m_SnapGranularity = 0.08f;

FINLINE_Z Bool		Playspace_SR_W::SRSphereVsCone(const Vec3f &_Center, Float R)
{
	Vec3f	Delta = _Center - m_FilterConePos;
	Float	Dist2 = Delta.GetNorm2();
	
	// Touche l'origine ?
	if (Dist2 < R*R)
		return TRUE;

	// Derrière ?
	Float	DistProj = Delta*m_FilterConeDir;
	if (DistProj < 0.f)
		return FALSE;

	// Trop loin ?
	if (DistProj > (m_FilterConeMax + R))
		return FALSE;

	// Angle...
	Float CorrectR = R * m_FilterConeInvCos;
	Float PerpDistMax = DistProj * m_FilterConeTan + CorrectR;

	Float RealPerpDist2 = Dist2 - (DistProj*DistProj);
	if (RealPerpDist2 > (PerpDistMax*PerpDistMax))
		return FALSE;

	return	TRUE;
}

/**************************************************************************/
/* SR BlindMap                                                            */
/**************************************************************************/

Playspace_SR_BlindMap::Playspace_SR_BlindMap()
{
}

/**************************************************************************/

Playspace_SR_BlindMap::~Playspace_SR_BlindMap()
{
	m_BlindMap.Flush();
}

/**************************************************************************/

void	Playspace_SR_BlindMap::Init(Vec3f _Center)
{
	m_fCenter = _Center;

	Float DeltaCenter = (Float)(SR_BLINDRAY_MAP_SIZE >> 1) * SR_BLINDRAY_MAP_TO_WORLD;
	m_fOrigin = m_fCenter - Vec3f(DeltaCenter,DeltaCenter,DeltaCenter);

	Float HalfCell = SR_BLINDRAY_MAP_TO_WORLD * 0.5f;
	m_fOrigin_Half = m_fOrigin + Vec3f(HalfCell,HalfCell,HalfCell);

	m_BlindMap.SetSize(SR_BLINDRAY_MAP_SIZE*SR_BLINDRAY_MAP_SIZE*SR_BLINDRAY_MAP_SIZE);
	Clear();
}

/**************************************************************************/

void	Playspace_SR_BlindMap::Clear()
{
	m_BlindMap.Null();
	m_StackPoint.Flush();
	m_StackVertex.Flush();
}

/**************************************************************************/

void	Playspace_SR_BlindMap::Flush()
{
	m_BlindMap.SetSize(0,FALSE);
	m_StackPoint.Flush();
	m_StackVertex.Flush();
}


/**************************************************************************/

S32		Playspace_SR_BlindMap::GetSeenZoneList(Float _x,Float _z,Float _yMin,Float _yMax,Float *_pTabDest,S32 _NbMax)
{
	EXCEPTIONC_Z((_NbMax & 0x1) == 0,"BAD GetSeenZoneList nbMax");

	S32 x = (S32)((_x- m_fOrigin.x + 0.01f) * SR_BLINDRAY_MAP_TO_CELL);
	if ((x < 0) || (x >= SR_BLINDRAY_MAP_SIZE))
		return 0;
	S32 z = (S32)((_z- m_fOrigin.z + 0.01f) * SR_BLINDRAY_MAP_TO_CELL);
	if ((z < 0) || (z >= SR_BLINDRAY_MAP_SIZE))
		return 0;

	S32 yMin = (S32)((_yMin- m_fOrigin.y + 0.01f) * SR_BLINDRAY_MAP_TO_CELL);
	if (yMin >= SR_BLINDRAY_MAP_SIZE)
		return 0;
	if (yMin < 0)
		yMin = 0;

	S32 yMax = (S32)((_yMax- m_fOrigin.y + 0.01f) * SR_BLINDRAY_MAP_TO_CELL);
	if (yMax < 0)
		return 0;
	if (yMax > SR_BLINDRAY_MAP_SIZE)
		yMax = SR_BLINDRAY_MAP_SIZE;

	PSR_BlindMap_Voxel	*pVoxel = &m_BlindMap[(((z << SR_BLINDRAY_MAP_SHIFT) + x) << SR_BLINDRAY_MAP_SHIFT)];

	S32	NbValZone = 0;
	for (S32 y=yMin ; y<yMax ; y++)
	{
		if (NbValZone & 0x1)
		{
			// Zone Started.
			if (!(pVoxel[y].Flags & PSR_BlindMap_Voxel::IsSeen))
			{
				// End Seen Zone.
				*_pTabDest++ = (Float)(y)*SR_BLINDRAY_MAP_TO_WORLD + m_fOrigin.y;
				NbValZone++;
				if (NbValZone >= _NbMax)
					return NbValZone;
			}
		}
		else
		{
			// Search start zone.
			if (pVoxel[y].Flags & PSR_BlindMap_Voxel::IsSeen)
			{
				// New Seen Zone.
				*_pTabDest++ = (Float)(y)*SR_BLINDRAY_MAP_TO_WORLD + m_fOrigin.y;
				NbValZone++;
			}
		}
	}
	if (NbValZone & 0x1)
	{
		// End.
		*_pTabDest++ = (Float)(yMax)*SR_BLINDRAY_MAP_TO_WORLD + m_fOrigin.y;
	}
	return NbValZone;
}


/**************************************************************************/
void	Playspace_SR_BlindMap::AddMesh(Playspace_Mesh &_Mesh,PSR_BlindMap_Voxel::Flag _Flag)
{
	AddMesh((U32*)_Mesh.m_TabQuad[0].TabPoints,_Mesh.m_TabQuad.GetSize(),sizeof(Playspace_Mesh::Face), _Mesh.m_TabPoints.GetArrayPtr(), _Mesh.m_TabPoints.GetSize(), _Flag);
	AddMeshEnd();
}

/**************************************************************************/
void	Playspace_SR_BlindMap::AddMeshSlow(U32 *_pTabTriIdx, S32 _NbTriIdx,U32 _ModuloTri, Vec3f *_pTabVtx, S32 _NbVtx, PSR_BlindMap_Voxel::Flag _Flag)
{
	if (!IsInit())
		return;

	// First : Add Points.
	HU8DA		PointInside;
	SafeArray_Z<Vec3f,1024,FALSE,FALSE> StackPoint;
	SafeArray_Z<U32,1024,FALSE,FALSE> StackVertex;

	PointInside.SetSize(_NbVtx);
	PointInside.Null();

	for (S32 i = 0; i<_NbVtx ; i++)
	{
		Vec3f pos = _pTabVtx[i];
		S32 x = (S32)((pos.x - m_fOrigin.x) * SR_BLINDRAY_MAP_TO_CELL);
		if ((x < 0) || (x >= SR_BLINDRAY_MAP_SIZE))
			continue;
		S32 z = (S32)((pos.z - m_fOrigin.z) * SR_BLINDRAY_MAP_TO_CELL);
		if ((z < 0) || (z >= SR_BLINDRAY_MAP_SIZE))
			continue;
		S32 y = (S32)((pos.y - m_fOrigin.y) * SR_BLINDRAY_MAP_TO_CELL);
		if ((y < 0) || (y >= SR_BLINDRAY_MAP_SIZE))
			continue;
		U8	&Flags = m_BlindMap[(((z << SR_BLINDRAY_MAP_SHIFT) + x) << SR_BLINDRAY_MAP_SHIFT) + y].Flags;
		Flags |= _Flag;
		PointInside[i] = 1;
	}

	// Subdivide faces if needed.
	Float MaxDist = SR_BLINDRAY_MAP_TO_WORLD / Sqrt(2.f);
	Float MaxDist2 = MaxDist*MaxDist;
	U32	*pCurFace = _pTabTriIdx;
	_ModuloTri >>= 2;

	for (S32 i = 0; i<_NbTriIdx ; i++,pCurFace+=_ModuloTri)
	{
		// Inside Manage.
		S32 Id0 = pCurFace[0];
		S32 Id1 = pCurFace[1];
		S32 Id2 = pCurFace[2];

		U8	NbInside = PointInside[Id0] + PointInside[Id1] + PointInside[Id2];
		if (!NbInside)
			continue;

		// Get Dist.
		S32 StackPtTop = 3;
		S32 StackVtxTop = 3;

		StackVertex[0] = 0;
		StackPoint[0] = _pTabVtx[Id0];

		StackVertex[1] = 1;
		StackPoint[1] = _pTabVtx[Id1];

		StackVertex[2] = 2;
		StackPoint[2] = _pTabVtx[Id2];

		while (StackVtxTop)
		{
			// Unstack.
			StackVtxTop -= 3;
			Id0 = StackVertex[StackVtxTop];
			Id1 = StackVertex[StackVtxTop+1];
			Id2 = StackVertex[StackVtxTop+2];
			Vec3f p0 = StackPoint[Id0];
			Vec3f p1 = StackPoint[Id1];
			Vec3f p2 = StackPoint[Id2];

			S32	MaskBig = 0;
			S32 NbBiggest = 0;
			if ((p1-p0).GetNorm2() > MaxDist2)
			{
				MaskBig |= 0x1;
				NbBiggest++;
			}
			if ((p2-p1).GetNorm2() > MaxDist2)
			{
				MaskBig |= 0x2;
				NbBiggest++;
			}
			if ((p0-p2).GetNorm2() > MaxDist2)
			{
				MaskBig |= 0x4;
				NbBiggest++;
			}

			// Nothing to do ?
			if (NbBiggest < 2)
				continue;

			if (NbBiggest == 3)
			{
				// Add Vertex.
				S32 Id01 = StackPtTop;
				S32 Id12 = StackPtTop+1;
				S32 Id20 = StackPtTop+2;

				Vec3f p01 = (p0 + p1) * 0.5f;
				Vec3f p12 = (p1 + p2) * 0.5f;
				Vec3f p20 = (p2 + p0) * 0.5f;
				StackPoint[Id01] = p01;
				StackPoint[Id12] = p12;
				StackPoint[Id20] = p20;
				StackPtTop += 3;

				// Add Ref Tri.
				StackVertex[StackVtxTop++] = Id0;
				StackVertex[StackVtxTop++] = Id01;
				StackVertex[StackVtxTop++] = Id20;

				StackVertex[StackVtxTop++] = Id01;
				StackVertex[StackVtxTop++] = Id1;
				StackVertex[StackVtxTop++] = Id12;

				StackVertex[StackVtxTop++] = Id20;
				StackVertex[StackVtxTop++] = Id12;
				StackVertex[StackVtxTop++] = Id2;

				StackVertex[StackVtxTop++] = Id12;
				StackVertex[StackVtxTop++] = Id20;
				StackVertex[StackVtxTop++] = Id01;
			}
			else
			{
				// One is OK.
				Vec3f	pM1,pM0;
				if (!(MaskBig & 0x1))
				{
					// 0 - 1 is little.
					pM1 = (p0 + p1) * 0.5f;
					pM0 = p2;
				}
				else if (!(MaskBig & 0x2))
				{
					// 1 - 2 is little.
					pM1 = (p1 + p2) * 0.5f;
					pM0 = p0;
				}
				else
				{
					// 2 - 0 is little.
					pM1 = (p2 + p0) * 0.5f;
					pM0 = p1;
				}

				Vec3f	delta = pM1-pM0;
				Float	d2 = delta.GetNorm2();
				if (d2 > MaxDist2)
				{
					// Subdivide.
					Float d = Sqrt(d2);
					S32 Nb = d / MaxDist;
					delta *= MaxDist / d;
					while (Nb)
					{
						pM0 += delta;
						StackPoint[StackPtTop] = pM0;
						StackPtTop++;
						Nb--;
					}
				}
			}
		}

		// Insert New Points.
		Vec3f *pStart = StackPoint.GetArrayPtr();
		Vec3f *pEnd = pStart + StackPtTop;
		pStart+=3;	// Don't Insert 3 first.

		if (NbInside == 3)
		{
			while (pStart < pEnd)
			{
				S32 x = (S32)((pStart->x - m_fOrigin.x) * SR_BLINDRAY_MAP_TO_CELL);
				S32 y = (S32)((pStart->y - m_fOrigin.y) * SR_BLINDRAY_MAP_TO_CELL);
				S32 z = (S32)((pStart->z - m_fOrigin.z) * SR_BLINDRAY_MAP_TO_CELL);
				U8	&Flags = m_BlindMap[(((z << SR_BLINDRAY_MAP_SHIFT) + x) << SR_BLINDRAY_MAP_SHIFT) + y].Flags;
				Flags |= _Flag;
				pStart++;
			}
		}
		else
		{
			while (pStart < pEnd)
			{
				S32 x = (S32)((pStart->x - m_fOrigin.x) * SR_BLINDRAY_MAP_TO_CELL);
				S32 y = (S32)((pStart->y - m_fOrigin.y) * SR_BLINDRAY_MAP_TO_CELL);
				S32 z = (S32)((pStart->z - m_fOrigin.z) * SR_BLINDRAY_MAP_TO_CELL);
				if (   ((x < 0) || (x >= SR_BLINDRAY_MAP_SIZE))
					|| ((y < 0) || (y >= SR_BLINDRAY_MAP_SIZE))
					|| ((z < 0) || (z >= SR_BLINDRAY_MAP_SIZE))
					)
					{
						pStart++;
						continue;
					}
				U8	&Flags = m_BlindMap[(((z << SR_BLINDRAY_MAP_SHIFT) + x) << SR_BLINDRAY_MAP_SHIFT) + y].Flags;
				Flags |= _Flag;
				pStart++;
			}
		}
	}
//	TimeToalAddMesh += GetAbsoluteTime() - TimeLocal;
}

/**************************************************************************/

void	Playspace_SR_BlindMap::AddMeshEnd()
{
	m_StackPoint.Flush();
	m_StackVertex.Flush();
}

void	Playspace_SR_BlindMap::AddMesh(U32 *_pTabTriIdx, S32 _NbTriIdx,U32 _ModuloTri, Vec3f *_pTabVtx, S32 _NbVtx, PSR_BlindMap_Voxel::Flag _Flag)
{
	// On ne passe que 10% dabs la partie qui subdivise les faces...
	// => AddMesh New semble plus lent...
	if (!IsInit())
		return;
	if (_NbVtx <= 0)
		return;

	// First : Add Points.
	HU8DA		PointInside;
	SafeArray_Z<U32,32,FALSE,FALSE>				StackOrder;

	PointInside.SetSize(_NbVtx);
	PointInside.Null();
	U8 *pFastPointInisde = PointInside.GetArrayPtr();

	for (S32 i = 0; i<_NbVtx ; i++)
	{
		Vec3f pos = _pTabVtx[i];
		S32 x = (S32)((pos.x - m_fOrigin.x) * SR_BLINDRAY_MAP_TO_CELL);
		if ((x < 0) || (x >= SR_BLINDRAY_MAP_SIZE))
			continue;
		S32 z = (S32)((pos.z - m_fOrigin.z) * SR_BLINDRAY_MAP_TO_CELL);
		if ((z < 0) || (z >= SR_BLINDRAY_MAP_SIZE))
			continue;
		S32 y = (S32)((pos.y - m_fOrigin.y) * SR_BLINDRAY_MAP_TO_CELL);
		if ((y < 0) || (y >= SR_BLINDRAY_MAP_SIZE))
			continue;
		U8	&Flags = m_BlindMap[(((z << SR_BLINDRAY_MAP_SHIFT) + x) << SR_BLINDRAY_MAP_SHIFT) + y].Flags;
		Flags |= _Flag;
		pFastPointInisde[i] = 1;
	}

	// Subdivide faces if needed.
	Float MaxDist = SR_BLINDRAY_MAP_TO_WORLD / Sqrt(2.f);
	Float MaxDist2 = MaxDist*MaxDist;
	U32	*pCurFace = _pTabTriIdx;
	_ModuloTri >>= 2;

	for (S32 i = 0; i<_NbTriIdx ; i++,pCurFace+=_ModuloTri)
	{
		// Inside Manage.
		U32 Id0 = pCurFace[0];
		U32 Id1 = pCurFace[1];
		U32 Id2 = pCurFace[2];

		EXCEPTIONC_Z((Id0 < (U32)_NbVtx) && (Id1 < (U32)_NbVtx) && (Id2 < (U32)_NbVtx),"Playspace_SR_BlindMap::AddMesh : BAD VERTEX !!!");
		U8	NbInside = pFastPointInisde[Id0] + pFastPointInisde[Id1] + pFastPointInisde[Id2];
		if (!NbInside)
			continue;

		// Test Point.
		Vec3f p0 = _pTabVtx[Id0];
		Vec3f p1 = _pTabVtx[Id1];
		Vec3f p2 = _pTabVtx[Id2];

		Float Size01 = (p1-p0).GetNorm2();
		Float Size12 = (p2-p1).GetNorm2();
		Float Size20 = (p0-p2).GetNorm2();

		S32 NbBiggest = 0;
		if (Size01 > MaxDist2)
			NbBiggest++;
		if (Size12 > MaxDist2)
			NbBiggest++;
		if (Size20 > MaxDist2)
			NbBiggest++;

		// Nothing to do ?
		if (NbBiggest < 2)
			continue;

		S32 StackPtTop = 3;
		S32 StackVtxTop = 3;

		m_StackPoint[0] = p0;
		m_StackPoint[1] = p1;
		m_StackPoint[2] = p2;

		// Sort Points.
		Float MinSize2;
		Float MiddleSize2;
		if (Size01 < Size20)
		{
			if (Size01 < Size12)
			{
				// Min is Size01
				MinSize2 = Size01;
				if (Size12 < Size20)
				{
					// 01 - 12 - 20 => 021
					MiddleSize2 = Size12;
					m_StackVertex[0] = 0;
					m_StackVertex[1] = 2;
					m_StackVertex[2] = 1;
				}
				else
				{
					// 01 - 20 - 12 => 120
					MiddleSize2 = Size20;
					m_StackVertex[0] = 1;
					m_StackVertex[1] = 2;
					m_StackVertex[2] = 0;
				}
			}
			else
			{
				// Min is Size12
				MinSize2 = Size12;
				// 12 - 01 - 20 => 201
				MiddleSize2 = Size01;
				m_StackVertex[0] = 2;
				m_StackVertex[1] = 0;
				m_StackVertex[2] = 1;
			}
		}
		else if (Size12 < Size20)
		{
			// Min is Size12
			MinSize2 = Size12;
			// 12 - 20 - 01 => 102
			MiddleSize2 = Size20;
			m_StackVertex[0] = 1;
			m_StackVertex[1] = 0;
			m_StackVertex[2] = 2;
		}
		else
		{
			// Min is Size20
			MinSize2 = Size20;
			if (Size01 < Size12)
			{
				// 20 - 01 - 12 => 210
				MiddleSize2 = Size01;
				m_StackVertex[0] = 2;
				m_StackVertex[1] = 1;
				m_StackVertex[2] = 0;
			}
			else
			{
				// 20 - 12 - 01 => 012
				MiddleSize2 = Size12;
				m_StackVertex[0] = 0;
				m_StackVertex[1] = 1;
				m_StackVertex[2] = 2;
			}
		}

		// Coompute max order.
		S32 NbSubdivide = 0;
		while (MinSize2 > MaxDist2)
		{
			NbSubdivide++;
			MinSize2 *=  0.25f;
			MiddleSize2 *= 0.25f;
		}

		Bool Subdivide2Side = TRUE;
		if (MiddleSize2 < MaxDist2)
			Subdivide2Side = FALSE;

		// Get Dist.
		S32 NumOrder = 0;
		StackOrder[0] = 1; // 1 triangle.
		
		while (StackVtxTop > 0)
		{
			// Unstack.
			StackVtxTop -= 3;
			Id0 = m_StackVertex[StackVtxTop];
			Id1 = m_StackVertex[StackVtxTop+1];
			Id2 = m_StackVertex[StackVtxTop+2];
			Vec3f p0 = m_StackPoint[Id0];
			Vec3f p1 = m_StackPoint[Id1];
			Vec3f p2 = m_StackPoint[Id2];

			StackOrder[NumOrder]--;

			if (NumOrder == NbSubdivide)
			{
				// Update Order.
				do {
					NumOrder--;
				} while ((NumOrder>0) && !StackOrder[NumOrder]);	// Down the order until nothing more in current order.

				// One Is Ok => Direct subdivide.
				if (!Subdivide2Side)
				{
					StackVtxTop-=9;
					continue;
				}

				Vec3f pM1 = (p2 + p0) * 0.5f;
				Vec3f pM0 = p1;

				Vec3f	delta = pM1-pM0;
				Float	d2 = delta.GetNorm2();
				if (d2 <= MaxDist2)
				{
					// One is bad...  ALL is bad !
					Subdivide2Side = FALSE;
					StackVtxTop-=9;
					continue;
				}

				// Subdivide.
				Float d = Sqrt(d2);
				S32 Nb = d / MaxDist;
				S32 SaveNb = Nb;
				delta *= MaxDist / d;
				while (Nb)
				{
					pM0 += delta;
					m_StackPoint[StackPtTop] = pM0;
					StackPtTop++;
					Nb--;
				}

				if (StackVtxTop > 0)
				{
					// There is 3 mirrored triangle on stack...
					for (S32 mirror=0 ; mirror<3 ; mirror++)
					{
						StackVtxTop-=3;
						Id0 = m_StackVertex[StackVtxTop];
						Id1 = m_StackVertex[StackVtxTop+1];
						Id2 = m_StackVertex[StackVtxTop+2];
							
						pM1 = (m_StackPoint[Id2] + m_StackPoint[Id0]) * 0.5f;
						pM0 = m_StackPoint[Id1];

						Nb = SaveNb;
						while (Nb)
						{
							pM0 -= delta;
							m_StackPoint[StackPtTop] = pM0;
							StackPtTop++;
							Nb--;
						}
					}
				}
			}
			else
			{
				// Subdivide in 4 identical triangles.
				// Add Vertex.
				S32 Id01 = StackPtTop;
				S32 Id12 = StackPtTop+1;
				S32 Id20 = StackPtTop+2;

				Vec3f p01 = (p0 + p1) * 0.5f;
				Vec3f p12 = (p1 + p2) * 0.5f;
				Vec3f p20 = (p2 + p0) * 0.5f;
				m_StackPoint[Id01] = p01;
				m_StackPoint[Id12] = p12;
				m_StackPoint[Id20] = p20;
				StackPtTop += 3;

				// Add Ref Tri => LAST is mirrored one (first poped).
				if (Subdivide2Side || (NumOrder < NbSubdivide))
				{
					NumOrder++;
					StackOrder[NumOrder] = 4; // 4 triangle.

					m_StackVertex[StackVtxTop++] = Id0;
					m_StackVertex[StackVtxTop++] = Id01;
					m_StackVertex[StackVtxTop++] = Id20;

					m_StackVertex[StackVtxTop++] = Id01;
					m_StackVertex[StackVtxTop++] = Id1;
					m_StackVertex[StackVtxTop++] = Id12;

					m_StackVertex[StackVtxTop++] = Id20;
					m_StackVertex[StackVtxTop++] = Id12;
					m_StackVertex[StackVtxTop++] = Id2;

					m_StackVertex[StackVtxTop++] = Id12;
					m_StackVertex[StackVtxTop++] = Id20;
					m_StackVertex[StackVtxTop++] = Id01;
				}
			}
		}

		// Insert New Points.
		Vec3f *pStart = m_StackPoint.GetArrayPtr();
		Vec3f *pEnd = pStart + StackPtTop;
		pStart+=3;	// Don't Insert 3 first.

		if (NbInside == 3)
		{
			while (pStart < pEnd)
			{
				S32 x = (S32)((pStart->x - m_fOrigin.x) * SR_BLINDRAY_MAP_TO_CELL);
				S32 y = (S32)((pStart->y - m_fOrigin.y) * SR_BLINDRAY_MAP_TO_CELL);
				S32 z = (S32)((pStart->z - m_fOrigin.z) * SR_BLINDRAY_MAP_TO_CELL);
				U8	&Flags = m_BlindMap[(((z << SR_BLINDRAY_MAP_SHIFT) + x) << SR_BLINDRAY_MAP_SHIFT) + y].Flags;
				Flags |= _Flag;
				pStart++;
			}
		}
		else
		{
			while (pStart < pEnd)
			{
				S32 x = (S32)((pStart->x - m_fOrigin.x) * SR_BLINDRAY_MAP_TO_CELL);
				S32 y = (S32)((pStart->y - m_fOrigin.y) * SR_BLINDRAY_MAP_TO_CELL);
				S32 z = (S32)((pStart->z - m_fOrigin.z) * SR_BLINDRAY_MAP_TO_CELL);
				if (   ((x < 0) || (x >= SR_BLINDRAY_MAP_SIZE))
					|| ((y < 0) || (y >= SR_BLINDRAY_MAP_SIZE))
					|| ((z < 0) || (z >= SR_BLINDRAY_MAP_SIZE))
					)
					{
						pStart++;
						continue;
					}
				U8	&Flags = m_BlindMap[(((z << SR_BLINDRAY_MAP_SHIFT) + x) << SR_BLINDRAY_MAP_SHIFT) + y].Flags;
				Flags |= _Flag;
				pStart++;
			}
		}
	}
}

/**************************************************************************/

void	Playspace_SR_BlindMap::AddMeshNew(U32 *_pTabTriIdx, S32 _NbTriIdx,U32 _ModuloTri, Vec3f *_pTabVtx, S32 _NbVtx, PSR_BlindMap_Voxel::Flag _Flag)
{
	if (!IsInit())
		return;
	if (_NbVtx <= 0)
		return;

	// First : Add Points.
	HU8DA		PointInside;
	PointInside.SetSize(_NbVtx);
	PointInside.Null();
	U8 *pFastPointInisde = PointInside.GetArrayPtr();

	for (S32 i = 0; i<_NbVtx ; i++)
	{
		Vec3f pos = _pTabVtx[i];
		S32 x = (S32)((pos.x - m_fOrigin.x) * SR_BLINDRAY_MAP_TO_CELL);
		if ((x < 0) || (x >= SR_BLINDRAY_MAP_SIZE))
			continue;
		S32 z = (S32)((pos.z - m_fOrigin.z) * SR_BLINDRAY_MAP_TO_CELL);
		if ((z < 0) || (z >= SR_BLINDRAY_MAP_SIZE))
			continue;
		S32 y = (S32)((pos.y - m_fOrigin.y) * SR_BLINDRAY_MAP_TO_CELL);
		if ((y < 0) || (y >= SR_BLINDRAY_MAP_SIZE))
			continue;
		U8	&Flags = m_BlindMap[(((z << SR_BLINDRAY_MAP_SHIFT) + x) << SR_BLINDRAY_MAP_SHIFT) + y].Flags;
		Flags |= _Flag;
		pFastPointInisde[i] = 1;
	}

	// Subdivide faces if needed.
	Float MaxDist = SR_BLINDRAY_MAP_TO_WORLD / Sqrt(2.f);
	Float MaxDist2 = MaxDist*MaxDist;
	U32	*pCurFace = _pTabTriIdx;
	_ModuloTri >>= 2;

	for (S32 i = 0; i<_NbTriIdx ; i++,pCurFace+=_ModuloTri)
	{
		// Inside Manage.
		U32 Id0 = pCurFace[0];
		U32 Id1 = pCurFace[1];
		U32 Id2 = pCurFace[2];

		EXCEPTIONC_Z((Id0 < (U32)_NbVtx) && (Id1 < (U32)_NbVtx) && (Id2 < (U32)_NbVtx),"Playspace_SR_BlindMap::AddMesh : BAD VERTEX !!!");
		U8	NbInside = pFastPointInisde[Id0] + pFastPointInisde[Id1] + pFastPointInisde[Id2];
		if (!NbInside)
			continue;

		// Test Point.
		Vec3f p0 = _pTabVtx[Id0];
		Vec3f p1 = _pTabVtx[Id1];
		Vec3f p2 = _pTabVtx[Id2];

		Float Size01 = (p1-p0).GetNorm2();
		Float Size12 = (p2-p1).GetNorm2();
		Float Size20 = (p0-p2).GetNorm2();

		S32	MaskBig = 0;
		S32 NbBiggest = 0;
		if (Size01 > MaxDist2)
		{
			MaskBig |= 0x1;
			NbBiggest++;
		}
		if (Size12 > MaxDist2)
		{
			MaskBig |= 0x2;
			NbBiggest++;
		}
		if (Size20 > MaxDist2)
		{
			MaskBig |= 0x4;
			NbBiggest++;
		}

		// Nothing to do ?
		if (NbBiggest < 2)
			continue;

		// Max Size.
		Float MaxSize = Size01;
		if (MaxSize < Size20)
			MaxSize = Size20;
		if (MaxSize < Size12)
			MaxSize = Size12;

		// Coompute max subidivide.
		S32 NbSubdivide = 1;
		while (MaxSize > MaxDist2)
		{
			NbSubdivide += NbSubdivide;
			MaxSize *=  0.25f;
		}
		Float DivNbSeg = 1.f / (Float)NbSubdivide;
		Vec3f Delta1 = (p0-p1)* DivNbSeg;
		Vec3f Delta2 = (p0-p2)* DivNbSeg;
		Vec3f StartP1 = p1;
		Vec3f StartP2 = p2;

		if (NbInside == 3)
		{
			for (S32 i = 0 ; i<=NbSubdivide ; i++)
			{
				Vec3f CurP = StartP1;
				Vec3f Delta = (p2-p1)* DivNbSeg;
				for (S32 j = i ; j<=NbSubdivide ; j++)
				{
					S32 x = (S32)((CurP.x - m_fOrigin.x) * SR_BLINDRAY_MAP_TO_CELL);
					S32 y = (S32)((CurP.y - m_fOrigin.y) * SR_BLINDRAY_MAP_TO_CELL);
					S32 z = (S32)((CurP.z - m_fOrigin.z) * SR_BLINDRAY_MAP_TO_CELL);
					U8	&Flags = m_BlindMap[(((z << SR_BLINDRAY_MAP_SHIFT) + x) << SR_BLINDRAY_MAP_SHIFT) + y].Flags;
					Flags |= _Flag;
					CurP += Delta;
				}

				StartP1 += Delta1;
				StartP2 += Delta2;
			}
		}
		else
		{
			for (S32 i = 0 ; i<=NbSubdivide ; i++)
			{
				Vec3f CurP = StartP1;
				Vec3f Delta = (p2-p1)* DivNbSeg;
				for (S32 j = i ; j<=NbSubdivide ; j++)
				{
					S32 x = (S32)((CurP.x - m_fOrigin.x) * SR_BLINDRAY_MAP_TO_CELL);
					S32 y = (S32)((CurP.y - m_fOrigin.y) * SR_BLINDRAY_MAP_TO_CELL);
					S32 z = (S32)((CurP.z - m_fOrigin.z) * SR_BLINDRAY_MAP_TO_CELL);
					if (   ((x >= 0) && (x < SR_BLINDRAY_MAP_SIZE))
						&& ((y >= 0) && (y < SR_BLINDRAY_MAP_SIZE))
						&& ((z >= 0) && (z < SR_BLINDRAY_MAP_SIZE))
						)
					{
						U8	&Flags = m_BlindMap[(((z << SR_BLINDRAY_MAP_SHIFT) + x) << SR_BLINDRAY_MAP_SHIFT) + y].Flags;
						Flags |= _Flag;
					}
					CurP += Delta;
				}

				StartP1 += Delta1;
				StartP2 += Delta2;
			}
		}
	}
}

/**************************************************************************/

void	Playspace_SR_BlindMap::AddMesh2(U32 *_pTabTriIdx, S32 _NbTriIdx, U32 _ModuloTri, Vec3f *_pTabVtx, S32 _NbVtx, PSR_BlindMap_Voxel::Flag _Flag)
{
	if (!IsInit())
		return;
	//Float TimeLocal = GetAbsoluteTime();
	// First : Add Points.
	HU8DA	PointInside;
	PointInside.SetSize(_NbVtx);
	PointInside.Null();

	for (S32 i = 0; i<_NbVtx; i++)
	{
		Vec3f pos = _pTabVtx[i];
		S32 x = (S32)((pos.x - m_fOrigin.x) * SR_BLINDRAY_MAP_TO_CELL);
		if ((x < 0) || (x >= SR_BLINDRAY_MAP_SIZE))
			continue;
		S32 z = (S32)((pos.z - m_fOrigin.z) * SR_BLINDRAY_MAP_TO_CELL);
		if ((z < 0) || (z >= SR_BLINDRAY_MAP_SIZE))
			continue;
		S32 y = (S32)((pos.y - m_fOrigin.y) * SR_BLINDRAY_MAP_TO_CELL);
		if ((y < 0) || (y >= SR_BLINDRAY_MAP_SIZE))
			continue;

		m_BlindMap[(((z << SR_BLINDRAY_MAP_SHIFT) + x) << SR_BLINDRAY_MAP_SHIFT) + y].Flags |= _Flag;
		PointInside[i] = 1;
	}

	// Subdivide faces if needed.
	Float MaxDist = SR_BLINDRAY_MAP_TO_WORLD / Sqrt(2.f);
	Float MaxDist2 = MaxDist * MaxDist;
	Float InvMaxDist = 1.0f / MaxDist;
	U32	*pCurFace = _pTabTriIdx;
	_ModuloTri >>= 2;

	for (S32 i = 0; i < _NbTriIdx; i++, pCurFace += _ModuloTri)
	{
		// Inside Manage.
		S32 Id0 = pCurFace[0];
		S32 Id1 = pCurFace[1];
		S32 Id2 = pCurFace[2];

		U8	NbInside = PointInside[Id0] + PointInside[Id1] + PointInside[Id2];
		if (!NbInside)
			continue;

		Vec3f p0 = _pTabVtx[Id0];
		Vec3f p1 = _pTabVtx[Id1];
		Vec3f p2 = _pTabVtx[Id2];

		Vec3f v1 = p1 - p0;		// p0 p1
		Vec3f v2 = p2 - p1;		// p1 p2
		Vec3f v3 = p0 - p2;		// p2 p0

		Float d1 = v1.GetNorm2();
		Float d2 = v2.GetNorm2();
		Float d3 = v3.GetNorm2();

		S32 nbLow = 0;
		if (d1 < MaxDist2)
			nbLow++;
		if (d2 < MaxDist2)
			nbLow++;
		if (d3 < MaxDist2)
			nbLow++;

		if (nbLow > 1)
			continue;

		Float factor = 1.0f;

		if (d1 < d2)
		{
			if (d2 < d3)		// d1 < d2 < d3
			{
				d3 = d1;
				v1 = v3;
				p1 = p2;
				factor = -1.0f;
			}
			else if (d1 < d3)	// d1 < d3 < d2
			{
				d3 = d1;
				v1 = v2;
				v2 = v3;
				p1 = p2;
			}
			else				// d3 < d1 < d2
			{
				Swap<Vec3f>(v1, v2);
				factor = -1.0f;
			}
		}
		else if (d1 < d3)		// d2 < d1 < d3
		{
			d3 = d2;
			v2 = v1;
			v1 = v3;
			p1 = p0;
		}
		else if (d2 < d3)		// d2 < d3 < d1
		{
			d3 = d2;
			v2 = v3;
			p1 = p0;
			factor = -1.0f;
		}

		if (nbLow == 1)
		{
			v1 *= factor;
			v2 *= factor;
			Vec3f v4 = v2 - v1;
			Float d4 = v4.GetNorm();
			S32 nb4 = S32(d4 * InvMaxDist);
			if (nb4 < 2)
				continue;

			v4 /= nb4;
			p1 += v4;
			S32 nbIter = ((nb4 + 1) >> 1) - 1;
			if (NbInside == 3)
			{
				for (S32 k = 0; k < nbIter; k++)
				{
					S32 x = (S32)((p1.x - m_fOrigin.x) * SR_BLINDRAY_MAP_TO_CELL);
					S32 y = (S32)((p1.y - m_fOrigin.y) * SR_BLINDRAY_MAP_TO_CELL);
					S32 z = (S32)((p1.z - m_fOrigin.z) * SR_BLINDRAY_MAP_TO_CELL);
					m_BlindMap[(((z << SR_BLINDRAY_MAP_SHIFT) + x) << SR_BLINDRAY_MAP_SHIFT) + y].Flags |= _Flag;
					p1 += v4;
				}
			}
			else
			{
				for (S32 k = 0; k < nbIter; k++)
				{
					S32 x = (S32)((p1.x - m_fOrigin.x) * SR_BLINDRAY_MAP_TO_CELL);
					S32 y = (S32)((p1.y - m_fOrigin.y) * SR_BLINDRAY_MAP_TO_CELL);
					S32 z = (S32)((p1.z - m_fOrigin.z) * SR_BLINDRAY_MAP_TO_CELL);
					if ((x >= 0) && (x < SR_BLINDRAY_MAP_SIZE) && (y >= 0) && (y < SR_BLINDRAY_MAP_SIZE) && (z >= 0) && (z < SR_BLINDRAY_MAP_SIZE))
					{
						m_BlindMap[(((z << SR_BLINDRAY_MAP_SHIFT) + x) << SR_BLINDRAY_MAP_SHIFT) + y].Flags |= _Flag;
					}
					p1 += v4;
				}
			}
		}
		else
		{
			S32 nb = 1;
			while (d3 >= MaxDist2)
			{
				d3 *= 0.25f;
				factor *= 0.5f;
				nb <<= 1;
			}
			v1 *= factor;
			v2 *= factor;

			Vec3f v4 = v2 - v1;
			Float d4 = v4.GetNorm();
			S32 nb4 = S32(d4 * InvMaxDist);

			v4 /= nb4;
			S32 iter[20];
			for (S32 i = 0; i < nb; i++)
				iter[i] = nb4;

			iter[nb - 1] = (iter[nb - 1] + 1) >> 1;
			iter[0]--;
			iter[nb] = 1;

			Vec3f pt = p1;						// v1 is [p0 p1] so pt goes from p1 to p0
			S32 nb2 = nb - 1;
			S32* iterPtr1 = iter;
			S32 tst = nb4 + 10;
			if (!(nb4 & 1))
				tst = nb4 >> 1;

			Bool firstIter = TRUE;
			if (NbInside == 3)
			{
				for (S32 i = 0; i < nb; i++)
				{
					Vec3f pt2 = pt;
					S32* iterPtr2 = iterPtr1++;
					for (S32 j = 0; j <= nb2; j++)
					{
						S32 nbIter = *iterPtr2++;
						Vec3f pt3 = pt2;
						if (firstIter)
						{
							pt3 += v4;
							tst--;
						}
						for (S32 k = 0; k < nbIter; k++)
						{
							if (k == tst)
							{
								pt3 += v4;
								continue;
							}
							S32 x = (S32)((pt3.x - m_fOrigin.x) * SR_BLINDRAY_MAP_TO_CELL);
							S32 y = (S32)((pt3.y - m_fOrigin.y) * SR_BLINDRAY_MAP_TO_CELL);
							S32 z = (S32)((pt3.z - m_fOrigin.z) * SR_BLINDRAY_MAP_TO_CELL);
							m_BlindMap[(((z << SR_BLINDRAY_MAP_SHIFT) + x) << SR_BLINDRAY_MAP_SHIFT) + y].Flags |= _Flag;
							pt3 += v4;
						}
						pt2 -= v1;
						if (firstIter)
						{
							tst++;
							firstIter = FALSE;
						}
					}
					pt += v2;
					if (i)
						nb2--;
				}
			}
			else
			{
				for (S32 i = 0; i < nb; i++)
				{
					Vec3f pt2 = pt;
					S32* iterPtr2 = iterPtr1++;
					for (S32 j = 0; j <= nb2; j++)
					{
						S32 nbIter = *iterPtr2++;
						Vec3f pt3 = pt2;
						if (firstIter)
						{
							pt3 += v4;
							tst--;
						}
						for (S32 k = 0; k < nbIter; k++)
						{
							if (k == tst)
							{
								pt3 += v4;
								continue;
							}
							S32 x = (S32)((pt3.x - m_fOrigin.x) * SR_BLINDRAY_MAP_TO_CELL);
							S32 y = (S32)((pt3.y - m_fOrigin.y) * SR_BLINDRAY_MAP_TO_CELL);
							S32 z = (S32)((pt3.z - m_fOrigin.z) * SR_BLINDRAY_MAP_TO_CELL);
							if ((x >= 0) && (x < SR_BLINDRAY_MAP_SIZE) && (y >= 0) && (y < SR_BLINDRAY_MAP_SIZE) && (z >= 0) && (z < SR_BLINDRAY_MAP_SIZE))
							{
								m_BlindMap[(((z << SR_BLINDRAY_MAP_SHIFT) + x) << SR_BLINDRAY_MAP_SHIFT) + y].Flags |= _Flag;
							}
							pt3 += v4;
						}
						pt2 -= v1;
						if (firstIter)
						{
							tst++;
							firstIter = FALSE;
						}
					}
					pt += v2;
					if (i)
						nb2--;
				}
			}
		}
	}
	//MESSAGE_Z("AddMesh2 nb max of points: %d", nbMaxPoints);
	//TimeToalAddMesh += GetAbsoluteTime() - TimeLocal;
}

/**************************************************************************/

void	Playspace_SR_BlindMap::ConvertWorkFlag(PSR_BlindMap_Voxel::Flag _Flag)
{
	if (!IsInit())
		return;

	PSR_BlindMap_Voxel *pStart = m_BlindMap.GetArrayPtr();
	PSR_BlindMap_Voxel *pEnd = pStart + m_BlindMap.GetSize();

	U8	ClearMask = 0xFF ^ (_Flag | PSR_BlindMap_Voxel::IsWork);

	while (pStart < pEnd)
	{
		U8 CurFlag = pStart->Flags;
		U8 WorkMask = (U8)(((S8)(CurFlag & PSR_BlindMap_Voxel::IsWork)) >> 7);	// Propagate bit.
		pStart->Flags = (WorkMask & _Flag) | (CurFlag & ClearMask);
		pStart++;
	}
}

/**************************************************************************/

void	Playspace_SR_BlindMap::CleatFlag(PSR_BlindMap_Voxel::Flag _Flag)
{
	if (!IsInit())
		return;

	PSR_BlindMap_Voxel *pStart = m_BlindMap.GetArrayPtr();
	PSR_BlindMap_Voxel *pEnd = pStart + m_BlindMap.GetSize();

	U8	ClearMask = 0xFF ^ _Flag;

	while (pStart < pEnd)
	{
		pStart->Flags &= ClearMask;
		pStart++;
	}
}

/**************************************************************************/

void	Playspace_SR_BlindMap::ProcessConeView(Playspace_SR_BlindMap::Mode _Mode,const Vec3f &_Pos, const Vec3f &_Dir,const Vec3f &_LateralDir, Float _Angle, Float _MaxDist)
{
	if (!IsInit())
		return;

	// Cone Min-Max.
	Vec2f	ConeSinCos;
	SinCos(ConeSinCos, _Angle);

	Float SinAngle = ConeSinCos.x;
	Float CosAngle2 = ConeSinCos.y*ConeSinCos.y;
	Float TanAngle = ConeSinCos.x / ConeSinCos.y;
	Float DeltaR = SinAngle * _MaxDist;
	Float MaxDist2 = _MaxDist*_MaxDist;

	Float MoveDist = (SR_BLINDRAY_MAP_VOXEL_DIAMETER / SinAngle);
	Vec3f ZBufferMarginPos = _Pos - MoveDist * _Dir;
	Float MaxDistMargin2 = _MaxDist + MoveDist*2.f;
	MaxDistMargin2 *= MaxDistMargin2;

	// Compute principal dir and Sign.
	Vec3i	Sign(1,1,1);
	Vec3i	Modulo(SR_BLINDRAY_MAP_SIZE, 1, SR_BLINDRAY_MAP_SIZE*SR_BLINDRAY_MAP_SIZE);
	Vec3f	aDir = _Dir;
	S32		Axis1 = 0;
	S32		Axis2 = 1;
	S32     Axis3 = 2;

	if (_Dir.x < 0.f)
	{
		Sign.x = -1;
		Modulo.x = -Modulo.x;
		aDir.x = -aDir.x;
	}
	if (_Dir.y < 0.f)
	{
		Sign.y = -1;
		Modulo.y = -Modulo.y;
		aDir.y = -aDir.y;
	}
	if (_Dir.z < 0.f)
	{
		Sign.z = -1;
		Modulo.z = -Modulo.z;
		aDir.z = -aDir.z;
	}

	if (aDir[Axis1] < aDir[Axis2])
		Swap(Axis1, Axis2);
	if (aDir[Axis1] < aDir[Axis3])
		Swap(Axis1, Axis3);
	if (aDir[Axis2] < aDir[Axis3])
		Swap(Axis2, Axis3);

	// InitZBuffer.
	m_LastZBuffer.SetSize(SR_BLINDRAY_ZBUFF_SIZE * SR_BLINDRAY_ZBUFF_SIZE);
	U8 *ptrZ = m_LastZBuffer.GetArrayPtr();
	S32 Delta2 = (SR_BLINDRAY_ZBUFF_HMASK - 1) * (SR_BLINDRAY_ZBUFF_HMASK-1);
	for (S32 x = -SR_BLINDRAY_ZBUFF_HMASK; x<(SR_BLINDRAY_ZBUFF_SIZE - SR_BLINDRAY_ZBUFF_HMASK); x++)
	{
		S32 x2 = x*x;
		for (S32 y = -SR_BLINDRAY_ZBUFF_HMASK; y<(SR_BLINDRAY_ZBUFF_SIZE - SR_BLINDRAY_ZBUFF_HMASK); y++)
		{
			if ((x2 + y*y) > Delta2)
				*ptrZ++ = 0;
			else 
				*ptrZ++ = 0xFF;
		}
	}
	EXCEPTIONC_Z(ptrZ == (m_LastZBuffer.GetArrayPtr() + m_LastZBuffer.GetSize()),"ZBuffer init error");

	// Compute Up Left Axis for ZBuffer.
	Vec3f	ZBufferUp;
	Vec3f	ZBufferLeft;
/*	if (Axis1 == 0)
	{
		// Left is biggest
		ZBufferUp = _Dir ^ (VEC3F_UP^_Dir);
		ZBufferLeft = _Dir ^ (VEC3F_FRONT^_Dir);
	}
	else if (Axis1 == 2)
	{
		// Front is biggest
		ZBufferUp = _Dir ^ (VEC3F_UP^_Dir);
		ZBufferLeft = _Dir ^ (VEC3F_LEFT^_Dir);
	}
	else if (Axis1 == 1)
	{
		// Up is biggest.
		ZBufferUp = _Dir ^ (VEC3F_FRONT^_Dir);
		ZBufferLeft = _Dir ^ (VEC3F_LEFT^_Dir);
	}
	ZBufferUp.CNormalize();
	ZBufferLeft.CNormalize();*/
	if (Axis1 == 1)
	{
		// Up is biggest.
		if  (Axis2 == 0)
			// Get Front as reference (because Front is lowest)
			ZBufferUp = VEC3F_FRONT;
		else
			// Get Left as reference (because Left is lowest)
			ZBufferUp = VEC3F_LEFT;
	}
	else
	{
		// Get Up as reference (bacesaue Up is not Biggest)
		ZBufferUp = VEC3F_UP;
	}
	ZBufferLeft = _Dir^ZBufferUp;
	ZBufferLeft.CNormalize();
	ZBufferUp = ZBufferLeft^_Dir;
	ZBufferUp.CNormalize();

	// Compute Optimal size for occlusion mask.
/*	Inutile... change rien.
	Vec3f v1(SR_BLINDRAY_MAP_TO_WORLD,SR_BLINDRAY_MAP_TO_WORLD,SR_BLINDRAY_MAP_TO_WORLD);
	Vec3f v2 = Quat(M_PI_2,VEC3F_UP) * v1;
	Vec3f v3 = Quat(M_PI,VEC3F_UP) * v1;
	Vec3f v4 = Quat(M_PI_2 * 3.f,VEC3F_UP) * v1;

	Float DLeft = Abs(ZBufferLeft * v1);
	DLeft = Max(DLeft,Abs(ZBufferLeft * v2));
	DLeft = Max(DLeft,Abs(ZBufferLeft * v3));
	DLeft = Max(DLeft,Abs(ZBufferLeft * v4));

	Float DUp = Abs(ZBufferUp * v1);
	DUp = Max(DUp,Abs(ZBufferUp * v2));
	DUp = Max(DUp,Abs(ZBufferUp * v3));
	DUp = Max(DUp,Abs(ZBufferUp * v4));*/

	// Compute Min-Max.
	Vec3f vMin = _Pos;
	Vec3f vMax = _Pos;
	Vec3f ConeEnd = _Pos + _Dir*_MaxDist;

	vMin.x = Min(vMin.x, ConeEnd.x - DeltaR);
	vMin.y = Min(vMin.y, ConeEnd.y - DeltaR);
	vMin.z = Min(vMin.z, ConeEnd.z - DeltaR);

	vMax.x = Max(vMax.x, ConeEnd.x + DeltaR);
	vMax.y = Max(vMax.y, ConeEnd.y + DeltaR);
	vMax.z = Max(vMax.z, ConeEnd.z + DeltaR);

	Vec3i Start,End;
	Start.x = (S32)((vMin.x - m_fOrigin.x) * SR_BLINDRAY_MAP_TO_CELL);
	Start.y = (S32)((vMin.y - m_fOrigin.y) * SR_BLINDRAY_MAP_TO_CELL);
	Start.z = (S32)((vMin.z - m_fOrigin.z) * SR_BLINDRAY_MAP_TO_CELL);

	if (Start.x < 0) Start.x = 0;
	if (Start.x >= SR_BLINDRAY_MAP_SIZE) Start.x = SR_BLINDRAY_MAP_SIZE - 1;

	if (Start.y < 0) Start.y = 0;
	if (Start.y >= SR_BLINDRAY_MAP_SIZE) Start.y = SR_BLINDRAY_MAP_SIZE - 1;

	if (Start.z < 0) Start.z = 0;
	if (Start.z >= SR_BLINDRAY_MAP_SIZE) Start.z = SR_BLINDRAY_MAP_SIZE - 1;

	End.x = (S32)((vMax.x - m_fOrigin.x) * SR_BLINDRAY_MAP_TO_CELL);
	End.y = (S32)((vMax.y - m_fOrigin.y) * SR_BLINDRAY_MAP_TO_CELL);
	End.z = (S32)((vMax.z - m_fOrigin.z) * SR_BLINDRAY_MAP_TO_CELL);

	if (End.x < 0) End.x = 0;
	if (End.x >= SR_BLINDRAY_MAP_SIZE) End.x = SR_BLINDRAY_MAP_SIZE - 1;

	if (End.y < 0) End.y = 0;
	if (End.y >= SR_BLINDRAY_MAP_SIZE) End.y = SR_BLINDRAY_MAP_SIZE - 1;

	if (End.z < 0) End.z = 0;
	if (End.z >= SR_BLINDRAY_MAP_SIZE) End.z = SR_BLINDRAY_MAP_SIZE - 1;

	// Invert if needed.
	if (Sign.x < 0) Swap(Start.x, End.x);
	if (Sign.y < 0) Swap(Start.y, End.y);
	if (Sign.z < 0) Swap(Start.z, End.z);
	End += Sign;

	Vec3f	VoxelPos;
	Vec3i	CurPos = Start;
	Float	DeltaAxis2 = (Float)(SR_BLINDRAY_MAP_TO_WORLD * Sign[Axis2]);

	U8		ZBufferVal = 5;	// Max delta = 5.

	U8		RejectMask = PSR_BlindMap_Voxel::IsSeen | PSR_BlindMap_Voxel::IsBlock;
	U8		RejectVal = PSR_BlindMap_Voxel::IsSeen;	// ISeen and !IsBlock
//	S32 SizeMin = 1000;
//	S32 SizeMax = -1000;
	if (_Mode == Playspace_SR_BlindMap::Mode_Paint)
	{
		RejectMask = PSR_BlindMap_Voxel::IsPaint | PSR_BlindMap_Voxel::IsBlock;
		RejectVal = PSR_BlindMap_Voxel::IsPaint; // ISeen and !IsBlock
	}
	else if (_Mode == Playspace_SR_BlindMap::Mode_Clear)
	{
		RejectMask = PSR_BlindMap_Voxel::IsUnPaint | PSR_BlindMap_Voxel::IsBlock;
		RejectVal = PSR_BlindMap_Voxel::IsUnPaint; // IsUnPaint and !IsBlock
	}


	while (CurPos[Axis1] != End[Axis1])
	{
		ZBufferVal++;
		U8 ZBufferTest = ZBufferVal;
		if (_Mode == Playspace_SR_BlindMap::Mode_Clear)
			ZBufferTest -= 2;	// Clear plus loin !

		CurPos[Axis2] = Start[Axis2];
		VoxelPos[Axis1] = (Float)(CurPos[Axis1]) * SR_BLINDRAY_MAP_TO_WORLD + m_fOrigin_Half[Axis1];
		VoxelPos[Axis2] = (Float)(CurPos[Axis2]) * SR_BLINDRAY_MAP_TO_WORLD + m_fOrigin_Half[Axis2];

		while (CurPos[Axis2] != End[Axis2])
		{

			CurPos[Axis3] = Start[Axis3];
			PSR_BlindMap_Voxel *pCur = &m_BlindMap[(((CurPos.z << SR_BLINDRAY_MAP_SHIFT) + CurPos.x) << SR_BLINDRAY_MAP_SHIFT) + CurPos.y];

			while (CurPos[Axis3] != End[Axis3])
			{
				U8 VoxelFlags = pCur->Flags;

				if ((VoxelFlags & RejectMask) == RejectVal)
				{
					if (  (_Mode != Playspace_SR_BlindMap::Mode_Blind)
						|| (pCur->SeenCount > QUALITY_BLIND_2)
						)
					{
						// Next !
						pCur += Modulo[Axis3];
						CurPos[Axis3] += Sign[Axis3];
						continue;
					}
				}

				// Compute pos.
				VoxelPos[Axis3] =(Float)(CurPos[Axis3]) * SR_BLINDRAY_MAP_TO_WORLD + m_fOrigin_Half[Axis3];

				// Is In Cone ?
				Bool DoZBufferTest = FALSE;
				Bool AffectVoxel = FALSE;

				Vec3f Delta = (VoxelPos - _Pos);
				Float fDelta2 = Delta.GetNorm2();
				Float Proj = Delta * _Dir;
				Float CosDelta2 = Proj*Proj;
				if (  (CosDelta2 >= (CosAngle2 * fDelta2))
					&& (fDelta2 < MaxDist2)
					&& (Proj > 0.001f)
					)
				{
					// In the Principal Cone !!
					DoZBufferTest = TRUE;
					AffectVoxel = TRUE;
				}
				else if (  (Proj > 0.001f)
						&& (VoxelFlags & PSR_BlindMap_Voxel::IsBlock)
						)
				{
					// Test if Block touch Margin Cone (For avoid bugs on border).
					Vec3f DeltaMargin = (VoxelPos - ZBufferMarginPos);
					Float fDeltaMargin2 = DeltaMargin.GetNorm2();
					Float ProjMargin = DeltaMargin * _Dir;
					Float CosDeltaMargin2 = ProjMargin*ProjMargin;
					if ((CosDeltaMargin2 >= (CosAngle2 * fDeltaMargin2))
						&& (fDeltaMargin2 < MaxDistMargin2)
						)
					{
						// In the Margin Cone !!
						DoZBufferTest = TRUE;
						AffectVoxel = FALSE;
					}
				}
				if (DoZBufferTest)
				{
					// Compute Occlusion;
					// Now Try détecte ZBuffer.
					Float LocalR = Proj * TanAngle;
					Float iLocalR = SR_BLINDRAY_ZBUFF_FHSIZE / LocalR;
					Vec3f ProjP = _Pos + Proj * _Dir;
					Vec3f LocalDelta = VoxelPos - ProjP;

					Float	fZBx = (LocalDelta * ZBufferLeft) * iLocalR + SR_BLINDRAY_ZBUFF_FHSIZE;
					Float	fZBy = (LocalDelta * ZBufferUp) * iLocalR + SR_BLINDRAY_ZBUFF_FHSIZE;

					Float	fZBl = SR_BLINDRAY_MAP_VOXEL_DIAMETER * 0.5f * iLocalR;

//					Float	fZBlX = DLeft * 0.5f * iLocalR;	=> Inutile... change rien.
//					Float	fZBlY = DUp * 0.5f * iLocalR;

					S32		ZBsx = (S32)(fZBx-fZBl);
					S32		ZBex = (S32)(fZBx+fZBl);
					S32		ZBsy = (S32)(fZBy-fZBl);
					S32		ZBey = (S32)(fZBy+fZBl);

					//SizeMin = Min((S32)(ZBex-ZBsx),SizeMin);
					//SizeMax = Max((S32)(ZBex-ZBsx),SizeMax);

					ZBsx = Max((S32)0, ZBsx);
					ZBsy = Max((S32)0, ZBsy);
					ZBex = Min((S32)SR_BLINDRAY_ZBUFF_MASK, ZBex);
					ZBey = Min((S32)SR_BLINDRAY_ZBUFF_MASK, ZBey);

					Bool IsOccluded = TRUE;
					U8 *pCurYPtr = m_LastZBuffer.GetArrayPtr() + (ZBsy << SR_BLINDRAY_ZBUFF_SHIFT);
					U8 *pEndYPtr = m_LastZBuffer.GetArrayPtr() + (ZBey << SR_BLINDRAY_ZBUFF_SHIFT);


					if (VoxelFlags & PSR_BlindMap_Voxel::IsBlock)
					{
						// contain faces => Mark and test
						while (pCurYPtr <= pEndYPtr)
						{
							U8 *pCurXPos = pCurYPtr + ZBsx;
							U8 *pEndXPos = pCurYPtr + ZBex;
							EXCEPTIONC_Z(pCurXPos >= m_LastZBuffer.GetArrayPtr(), "OUT OF ZBUFFER");
							EXCEPTIONC_Z(pEndXPos < (m_LastZBuffer.GetArrayPtr() + m_LastZBuffer.GetSize()), "OUT OF ZBUFFER");
							while (pCurXPos <= pEndXPos)
							{
								U8 val = *pCurXPos;
								if (ZBufferVal < val)
								{
									*pCurXPos = ZBufferVal;
									IsOccluded = FALSE;
								}
								else if (ZBufferTest < val)
									IsOccluded = FALSE;
								pCurXPos++;
							}
							pCurYPtr += SR_BLINDRAY_ZBUFF_SIZE;
						}
					}
					else
					{
						// Empty => Only test
						while (pCurYPtr <= pEndYPtr)
						{
							U8 *pCurXPos = pCurYPtr + ZBsx;
							U8 *pEndXPos = pCurYPtr + ZBex;
							EXCEPTIONC_Z(pCurXPos >= m_LastZBuffer.GetArrayPtr(), "OUT OF ZBUFFER");
							EXCEPTIONC_Z(pEndXPos < (m_LastZBuffer.GetArrayPtr() + m_LastZBuffer.GetSize()), "OUT OF ZBUFFER");
							while (pCurXPos <= pEndXPos)
							{
								U8 val = *pCurXPos++;
								if (ZBufferTest < val)
								{
									IsOccluded = FALSE;
									pCurYPtr = pEndYPtr+1;
									break;
								}
							}
							pCurYPtr += SR_BLINDRAY_ZBUFF_SIZE;
						}
					}

/*					if (IsOccluded)
					{
						DRAW_DEBUG_SPHERE3D(VoxelPos, COLOR_RED*0.99f, 0.04f);//, .displayDuration(1000.f));
					}
					else
					{
						DRAW_DEBUG_SPHERE3D(VoxelPos, COLOR_GREEN*0.99f, 0.04f);//, .displayDuration(1000.f));
					}*/

					// Modify Voxel if needed.
					if (AffectVoxel)
					{
						if (_Mode != Playspace_SR_BlindMap::Mode_Blind)
						{
							if (_Mode == Playspace_SR_BlindMap::Mode_Paint)
							{
								if (!IsOccluded && (VoxelFlags & PSR_BlindMap_Voxel::IsSeen))
								{
									if ((_LateralDir * Delta) < 0)
									{
										VoxelFlags &= 0xFF ^ PSR_BlindMap_Voxel::IsUnPaint;
										VoxelFlags |= PSR_BlindMap_Voxel::IsPaint;
									}
								}
							}
							else if (_Mode == Playspace_SR_BlindMap::Mode_Clear)
							{
								if (!IsOccluded)
								{
									if (VoxelFlags & PSR_BlindMap_Voxel::IsPaint)
									{
										VoxelFlags &= 0xFF ^ PSR_BlindMap_Voxel::IsPaint;
										VoxelFlags |= PSR_BlindMap_Voxel::IsUnPaint;
									}
									if (pCur->SeenCount)
										pCur->SeenCount = 1;
								}
							}
						}
						else
						{
							if (!IsOccluded)
							{
								VoxelFlags |= PSR_BlindMap_Voxel::IsSeen;
								if ((pCur->SeenCount <= QUALITY_BLIND_2) && !(VoxelFlags & PSR_BlindMap_Voxel::IsUnPaint))
									pCur->SeenCount++;
							}
							VoxelFlags |= PSR_BlindMap_Voxel::IsInCone;
						}
						pCur->Flags = VoxelFlags;
					}
				}

				// Next !
				pCur += Modulo[Axis3];
				CurPos[Axis3] += Sign[Axis3];
			}

			// Next 
			CurPos[Axis2] += Sign[Axis2];
			VoxelPos[Axis2] += DeltaAxis2;
		}

		// Next 
		CurPos[Axis1] += Sign[Axis1];
	}
//MESSAGE_Z("Min Max %d %d",SizeMin,SizeMax);
/*	
	Vec3f VoxelPos;
	while (z <= EndZ)
	{
		VoxelPos.z = (Float)(z) * SR_BLINDRAY_MAP_TO_WORLD + m_fOrigin_Half.z;
		VoxelPos.x = (Float)(x) * SR_BLINDRAY_MAP_TO_WORLD + m_fOrigin_Half.x;

		S32 CurX = x;
		while (CurX <= EndX)
		{
			U8 *pCurY = &m_BlindMap[(((z << SR_BLINDRAY_MAP_SHIFT) + CurX) << SR_BLINDRAY_MAP_SHIFT) + y];
			U8 *pEndY = pCurY + EndY - y;
			S32 CurY = y-1;
			while (pCurY <= pEndY)
			{
				U8 Voxel = *pCurY++;
				CurY++;
				if (Voxel & Playspace_SR_BlindMap::IsSeen)
					continue;

				// Compute pos.
				VoxelPos.y = (Float)(y) * SR_BLINDRAY_MAP_TO_WORLD + m_fOrigin_Half.y;

				// Is In Cone ?
				Vec3f Delta = (VoxelPos - _Pos);
				Float fDelta2 = Delta.GetNorm2();
				Float CosDelta2 = Delta * _Dir;
				CosDelta2 *= CosDelta2;
				if (CosDelta2 < (CosAngle2 * fDelta2))
					continue;
				if (fDelta2 > MaxDist2)
					continue;

				// In the Cone !!
				Voxel |= Playspace_SR_BlindMap::IsSeen;
				pCurY[-1] = Voxel;
			}
			VoxelPos.x  += SR_BLINDRAY_MAP_TO_WORLD;
			CurX++;
		}
		z++;
	}*/
}

/**************************************************************************/

void	Playspace_SR_BlindMap::ProcessConeViewNew(Playspace_SR_BlindMap::Mode _Mode,const Playspace_SR_ConeView &_ConeBlind,const Playspace_SR_ConeView &_ConeMode,const Vec3f &_LateralDir)
{
	if (!IsInit())
		return;
	if (!m_LastZBuffer.GetSize())
		return;
Float t0 = GetAbsoluteTime();
	// Cone Min-Max.
	Vec2f	ConeSinCos;
	SinCos(ConeSinCos, _ConeBlind.Angle);

	Float CosMode2 = Cos(_ConeMode.Angle);
	CosMode2 *= CosMode2;

	Float MaxDist = _ConeBlind.Dist;
	Vec3f DirBlind = _ConeBlind.Dir;
	Vec3f PosBlind = _ConeBlind.Pos;

	Vec3f DirMode = _ConeMode.Dir;
	Vec3f PosMode = _ConeMode.Pos;

	Float SinAngle = ConeSinCos.x;
	Float CosAngle2 = ConeSinCos.y*ConeSinCos.y;
	Float TanAngle = ConeSinCos.x / ConeSinCos.y;
	Float DeltaR = SinAngle * MaxDist;
	Float MaxDist2 = MaxDist*MaxDist;

	Float MoveDist = (SR_BLINDRAY_MAP_VOXEL_DIAMETER / SinAngle);
	Vec3f ZBufferMarginPos = PosBlind - MoveDist * DirBlind;
	Float MaxDistMargin2 = MaxDist + MoveDist*2.f;
	MaxDistMargin2 *= MaxDistMargin2;
//	Vec3i	Modulo(SR_BLINDRAY_MAP_SIZE, 1, SR_BLINDRAY_MAP_SIZE*SR_BLINDRAY_MAP_SIZE);

	// Compute Up Left Axis for ZBuffer.
	Vec3f	ZBufferUp;
	Vec3f	ZBufferLeft;
	Vec3f	ACamDir(Abs(DirBlind.x),Abs(DirBlind.y),Abs(DirBlind.z));
	if ((ACamDir.y > ACamDir.x) && (ACamDir.y > ACamDir.z))
	{
		// Up is biggest.
		if (ACamDir.x > ACamDir.z)
			// Get Front as reference (because Front is lowest)
			ZBufferUp = VEC3F_FRONT;
		else
			// Get Left as reference (because Left is lowest)
			ZBufferUp = VEC3F_LEFT;
	}
	else
	{
		// Get Up as reference (bacesaue Up is not Biggest)
		ZBufferUp = VEC3F_UP;
	}

	ZBufferLeft = DirBlind^ZBufferUp;
	ZBufferLeft.CNormalize();
	ZBufferUp = ZBufferLeft^DirBlind;
	ZBufferUp.CNormalize();

	// Compute Optimal size for occlusion mask.

	// Compute Min-Max.
	Vec3f vMin = PosBlind;
	Vec3f vMax = PosBlind;
	Vec3f ConeEnd = PosBlind + DirBlind*MaxDist;

	vMin.x = Min(vMin.x, ConeEnd.x - DeltaR);
	vMin.y = Min(vMin.y, ConeEnd.y - DeltaR);
	vMin.z = Min(vMin.z, ConeEnd.z - DeltaR);

	vMax.x = Max(vMax.x, ConeEnd.x + DeltaR);
	vMax.y = Max(vMax.y, ConeEnd.y + DeltaR);
	vMax.z = Max(vMax.z, ConeEnd.z + DeltaR);

	Vec3i Start,End;
	Start.x = (S32)((vMin.x - m_fOrigin.x) * SR_BLINDRAY_MAP_TO_CELL);
	Start.y = (S32)((vMin.y - m_fOrigin.y) * SR_BLINDRAY_MAP_TO_CELL);
	Start.z = (S32)((vMin.z - m_fOrigin.z) * SR_BLINDRAY_MAP_TO_CELL);

	if (Start.x < 0) Start.x = 0;
	if (Start.x >= SR_BLINDRAY_MAP_SIZE) Start.x = SR_BLINDRAY_MAP_SIZE - 1;

	if (Start.y < 0) Start.y = 0;
	if (Start.y >= SR_BLINDRAY_MAP_SIZE) Start.y = SR_BLINDRAY_MAP_SIZE - 1;

	if (Start.z < 0) Start.z = 0;
	if (Start.z >= SR_BLINDRAY_MAP_SIZE) Start.z = SR_BLINDRAY_MAP_SIZE - 1;

	End.x = (S32)((vMax.x - m_fOrigin.x) * SR_BLINDRAY_MAP_TO_CELL) + 1;
	End.y = (S32)((vMax.y - m_fOrigin.y) * SR_BLINDRAY_MAP_TO_CELL) + 1;
	End.z = (S32)((vMax.z - m_fOrigin.z) * SR_BLINDRAY_MAP_TO_CELL) + 1;

	if (End.x < 0) End.x = 0;
	if (End.x > SR_BLINDRAY_MAP_SIZE) End.x = SR_BLINDRAY_MAP_SIZE;

	if (End.y < 0) End.y = 0;
	if (End.y > SR_BLINDRAY_MAP_SIZE) End.y = SR_BLINDRAY_MAP_SIZE;

	if (End.z < 0) End.z = 0;
	if (End.z > SR_BLINDRAY_MAP_SIZE) End.z = SR_BLINDRAY_MAP_SIZE;

	// Invert if needed.
//	Float	DeltaAxis2 = (Float)(SR_BLINDRAY_MAP_TO_WORLD * Sign[Axis2]);

	Float	ToleranceMode = 0.f;
	Bool	ModeActivated = TRUE;
	U8		RejectMask = PSR_BlindMap_Voxel::IsSeen;	// already IsSeen
	if (_Mode == Playspace_SR_BlindMap::Mode_Paint)
		RejectMask = PSR_BlindMap_Voxel::IsPaint; // IsPaint (Paint => Seen)
	else if (_Mode == Playspace_SR_BlindMap::Mode_Clear)
	{
		RejectMask = PSR_BlindMap_Voxel::IsUnPaint; // IsUnPaint (IsUnPaint => Seen)
		ToleranceMode = 0.2f; // 20 cm.
	}
	else
		ModeActivated = FALSE;
	S32  iToleranceMode = ToleranceMode * 254.f/ MaxDist;

	S32	CacheSizeX = End.x - Start.x + 1;
	S32	CacheSizeY = End.y - Start.y + 1;
	S32	CacheSizeZ = End.z - Start.z + 1;
	S32 DelteCacheZ = CacheSizeX*CacheSizeY;

	HU8DA	CacheSeen;
	CacheSeen.SetSize(CacheSizeX*CacheSizeY*CacheSizeZ);
	CacheSeen.Null();
	if (!CacheSeen.GetSize())
		return;

#if 0
	// Traitement optimal par Flooding => traite 4x moins de datas ... mais pas plus rapide. :(
	HugeDynArray_Z<Vec3i,32,FALSE,FALSE,4,TRUE>		StackCone;
	StackCone.SetSize(CacheSeen.GetSize());	// Can't Have More val.

	Vec3f StartPointInCone = PosBlind + DirBlind * MaxDist * 0.8f;
	Vec3f EndPointInCone = PosBlind + DirBlind * 0.1f;

	Vec3i FirstPoint;
	Vec3f	VoxelPos;

	for (;;)
	{
		FirstPoint.x = (S32)((StartPointInCone.x - m_fOrigin.x) * SR_BLINDRAY_MAP_TO_CELL);
		FirstPoint.y = (S32)((StartPointInCone.y - m_fOrigin.y) * SR_BLINDRAY_MAP_TO_CELL);
		FirstPoint.z = (S32)((StartPointInCone.z - m_fOrigin.z) * SR_BLINDRAY_MAP_TO_CELL);

		if (   (FirstPoint.x >= 0) && (FirstPoint.x < SR_BLINDRAY_MAP_SIZE)
			&& (FirstPoint.y >= 0) && (FirstPoint.y < SR_BLINDRAY_MAP_SIZE)
			&& (FirstPoint.z >= 0) && (FirstPoint.z < SR_BLINDRAY_MAP_SIZE)
			)
			break;

		StartPointInCone -= DirBlind * 0.08f;
		if (((StartPointInCone-EndPointInCone) * DirBlind) < 0.f)
			return;
	}

	FirstPoint -= Start;

	Vec3f CacheOrigin;
	CacheOrigin.x = (Float)(Start.x) * SR_BLINDRAY_MAP_TO_WORLD + m_fOrigin.x;
	CacheOrigin.y = (Float)(Start.y) * SR_BLINDRAY_MAP_TO_WORLD + m_fOrigin.y;
	CacheOrigin.z = (Float)(Start.z) * SR_BLINDRAY_MAP_TO_WORLD + m_fOrigin.z;

	S32 LastElem = 0;
	StackCone[LastElem] = FirstPoint;
	CacheSeen[((FirstPoint.z *CacheSizeX) + FirstPoint.x)*CacheSizeY + FirstPoint.y] = 0x80; // Set as done.
S32 Nb = 0;

	while (LastElem >= 0)
	{
		// Pop.
		Vec3i CurPos = StackCone[LastElem--];
		
Nb++;
		// Process It.
		VoxelPos.x = (Float)(CurPos.x) * SR_BLINDRAY_MAP_TO_WORLD + CacheOrigin.x;
		VoxelPos.y = (Float)(CurPos.y) * SR_BLINDRAY_MAP_TO_WORLD + CacheOrigin.y;
		VoxelPos.z = (Float)(CurPos.z) * SR_BLINDRAY_MAP_TO_WORLD + CacheOrigin.z;

		// Is In Cone ?
		Vec3f Delta = (VoxelPos - PosBlind);
		Float Proj = Delta * DirBlind;
		if ((Proj < 0.001f) || (Proj >= MaxDist))
			continue;

		Float fDelta2 = Delta*Delta;
		Float CosDelta2 = Proj*Proj;
		if (CosDelta2 < (CosAngle2 * fDelta2))
			continue;

		// I am in the cone.
		// Is it visible ?
		Float LocalR = Proj * TanAngle;
		Float iLocalR = SR_BLINDRAY_ZBUFF_FHSIZE / LocalR;
		Float	fZBx = (Delta * ZBufferLeft) * iLocalR;
		Float	fZBy = (Delta * ZBufferUp) * iLocalR;

		S32 x = (S32)(fZBx+SR_BLINDRAY_ZBUFF_FHSIZE);
		S32 y = (S32)(fZBy+SR_BLINDRAY_ZBUFF_FHSIZE);
		S32 z = (S32)(Proj * 254.f/ MaxDist);

//				if ((x<0) || (x>=128) || (y<0) || (y>=128))
//					int laa = 0;
		S32 ZTest = m_LastZBuffer[(y << SR_BLINDRAY_ZBUFF_SHIFT) + x];

		// Occluded for Z and Mode ?
		U8 *pCache = &CacheSeen[((CurPos.z *CacheSizeX) + CurPos.x)*CacheSizeY + CurPos.y];
		if (z <= (ZTest+iToleranceMode))
		{
			// Set as seen
			if (z <= ZTest)
			{
				// Seen.
				*pCache |= 0x1;
			}

			// Mode modification.
			if (ModeActivated)
			{
				Vec3f DeltaMode = (VoxelPos - PosMode);
				Float ProjMode = DeltaMode * DirMode;
				if ((ProjMode > 0.001f) && (ProjMode < MaxDist))
				{
					Float fDeltaMode2 = DeltaMode*DeltaMode;
					Float CosDeltaMode2 = ProjMode*ProjMode;
					if (CosDeltaMode2 > (CosMode2 * fDeltaMode2))
					{
						if ((_Mode != Playspace_SR_BlindMap::Mode_Paint) || ((_LateralDir * DeltaMode) < 0))
							*pCache |= 0x2;
					}
				}
			}
		}

		// Check NeighBoor.
		if (CurPos.y && !pCache[-1])
		{
			pCache[-1] = 0x80;
			Vec3i &p = StackCone[++LastElem];
			 p.x = CurPos.x;
			 p.y = CurPos.y-1;
			 p.z = CurPos.z;
		}
		if (CurPos.y < (CacheSizeY-1) && !pCache[1])
		{
			pCache[1] = 0x80;
			Vec3i &p = StackCone[++LastElem];
			 p.x = CurPos.x;
			 p.y = CurPos.y+1;
			 p.z = CurPos.z;
		}

		if (CurPos.x && !pCache[-CacheSizeY])
		{
			pCache[-CacheSizeY] = 0x80;
			Vec3i &p = StackCone[++LastElem];
			 p.x = CurPos.x-1;
			 p.y = CurPos.y;
			 p.z = CurPos.z;
		}
		if (CurPos.x < (CacheSizeX-1) && !pCache[CacheSizeY])
		{
			pCache[CacheSizeY] = 0x80;
			Vec3i &p = StackCone[++LastElem];
			 p.x = CurPos.x+1;
			 p.y = CurPos.y;
			 p.z = CurPos.z;
		}

		if (CurPos.z && !pCache[-DelteCacheZ])
		{
			pCache[-DelteCacheZ] = 0x80;
			Vec3i &p = StackCone[++LastElem];
			 p.x = CurPos.x;
			 p.y = CurPos.y;
			 p.z = CurPos.z-1;
		}
		if (CurPos.z < (CacheSizeZ-1) && !pCache[DelteCacheZ])
		{
			pCache[DelteCacheZ] = 0x80;
			Vec3i &p = StackCone[++LastElem];
			 p.x = CurPos.x;
			 p.y = CurPos.y;
			 p.z = CurPos.z+1;
		}
	}
#else
	{
	Vec3f	DeltaPos;
	U8		*pCache = CacheSeen.GetArrayPtr();
	for (S32 CurPosZ = Start.z ; CurPosZ <= End.z ; CurPosZ++)
	{
		S32 CurPosX = Start.x;
		DeltaPos.x = (Float)(CurPosX) * SR_BLINDRAY_MAP_TO_WORLD + m_fOrigin.x - PosBlind.x;
		DeltaPos.z = (Float)(CurPosZ) * SR_BLINDRAY_MAP_TO_WORLD + m_fOrigin.z - PosBlind.z;
		for (; CurPosX <= End.x ; CurPosX++,DeltaPos.x += SR_BLINDRAY_MAP_TO_WORLD)
		{
			DeltaPos.y = (Float)(Start.y) * SR_BLINDRAY_MAP_TO_WORLD + m_fOrigin.y - PosBlind.y;
			Float ProjPos = DeltaPos * DirBlind;
			Float dProjPos = SR_BLINDRAY_MAP_TO_WORLD*DirBlind.y;
			U8	*pCacheEnd = pCache + CacheSizeY;
			for (;pCache < pCacheEnd;pCache++,DeltaPos.y+=SR_BLINDRAY_MAP_TO_WORLD,ProjPos+=dProjPos)
			{
				// Is In Cone ?
				if ((ProjPos < 0.001f) || (ProjPos >= MaxDist))
					continue;
				Float fDelta2 = DeltaPos*DeltaPos;
				Float CosDelta2 = ProjPos*ProjPos;
				if (CosDelta2 < (CosAngle2 * fDelta2))
					continue;

				// I am in the cone.

				// Is it visible ?
				Float LocalR = ProjPos * TanAngle;
				Float iLocalR = SR_BLINDRAY_ZBUFF_FHSIZE / LocalR;
				Float	fZBx = (DeltaPos * ZBufferLeft) * iLocalR;
				Float	fZBy = (DeltaPos * ZBufferUp) * iLocalR;

				S32 x = (S32)(fZBx+SR_BLINDRAY_ZBUFF_FHSIZE);
				S32 y = (S32)(fZBy+SR_BLINDRAY_ZBUFF_FHSIZE);
				S32 z = (S32)(ProjPos * 254.f/ MaxDist);

//				if ((x<0) || (x>=128) || (y<0) || (y>=128))
//					int laa = 0;
				S32 ZTest = m_LastZBuffer[(y << SR_BLINDRAY_ZBUFF_SHIFT) + x];

				// Occluded for Z and Mode ?
				if (z > (ZTest+iToleranceMode))
					continue;

				// Set as seen
				U8	CacheVal = 0;
				if (z <= ZTest)
				{
					// Seen.
					CacheVal = 1;
				}

				// Mode modification.
				if (ModeActivated)
				{
					Bool IsInsideMode = TRUE;
					if (CosDelta2 < (CosMode2 * fDelta2))
						IsInsideMode = FALSE;
					else if ((_Mode == Playspace_SR_BlindMap::Mode_Paint) && ((_LateralDir * DeltaPos) >= 0))
						IsInsideMode = FALSE;
					if (IsInsideMode)
						CacheVal |= 2;
				}
				*pCache = CacheVal;
			}
		}
	}
	EXCEPTIONC_Z(pCache == (CacheSeen.GetArrayPtr() + CacheSeen.GetSize()),"Bad Cache position");
	}
#endif

Float t1 = GetAbsoluteTime();
	// Now report result to BlindMap
	U8 *pCache = CacheSeen.GetArrayPtr();
	for (S32 CurPosZ = Start.z ; CurPosZ < End.z ; CurPosZ++)
	{
		S32 CurPosX = Start.x;
		for (; CurPosX < End.x ; CurPosX++)
		{
			S32 CurPosY = Start.y;
			PSR_BlindMap_Voxel *pCur = &m_BlindMap[(((CurPosZ << SR_BLINDRAY_MAP_SHIFT) + CurPosX) << SR_BLINDRAY_MAP_SHIFT) + CurPosY];
			for (;CurPosY < End.y;pCur++,pCache++,CurPosY++)
			{
				U8 VoxelFlags = pCur->Flags;
				if ((VoxelFlags & RejectMask) && (pCur->SeenCount > QUALITY_BLIND_2))
					continue;

				// Merge neighbors 
				U8 *pCacheZNext = pCache + DelteCacheZ;
				U8 val = pCache[0] | pCache[1] | pCacheZNext[0] | pCacheZNext[1];
				val |= pCache[CacheSizeY] | pCache[CacheSizeY+1] | pCacheZNext[CacheSizeY] | pCacheZNext[CacheSizeY+1];

				// Set as seen
				if (val & 1)
				{
					// Seen.
					VoxelFlags |= PSR_BlindMap_Voxel::IsSeen;
	//				if ((pCur->SeenCount <= QUALITY_BLIND_2) && (VoxelFlags & (PSR_BlindMap_Voxel::IsPaint)))//!(VoxelFlags & (PSR_BlindMap_Voxel::IsUnPaint)))
					if ((pCur->SeenCount <= QUALITY_BLIND_2) && !(VoxelFlags & (PSR_BlindMap_Voxel::IsUnPaint)))
						pCur->SeenCount++;
				}
				VoxelFlags |= PSR_BlindMap_Voxel::IsInCone;

				// Mode modification.
				if (val & 2)
				{
					if (_Mode == Playspace_SR_BlindMap::Mode_Paint)
					{
						if (VoxelFlags & PSR_BlindMap_Voxel::IsSeen)
						{
							VoxelFlags &= 0xFF ^ PSR_BlindMap_Voxel::IsUnPaint;
							VoxelFlags |= PSR_BlindMap_Voxel::IsPaint;
						}
					}
					else if (_Mode == Playspace_SR_BlindMap::Mode_Clear)
					{
						if (VoxelFlags & PSR_BlindMap_Voxel::IsPaint)
						{
							VoxelFlags &= 0xFF ^ PSR_BlindMap_Voxel::IsPaint;
							VoxelFlags |= PSR_BlindMap_Voxel::IsUnPaint;
						}
						if (pCur->SeenCount)
							pCur->SeenCount = 1;
					}
				}
				// Report flags.
				pCur->Flags = VoxelFlags;
			}
			// One more Y
			pCache++;
		}
		// One more X
		pCache+=CacheSizeY;
	}
	pCache+=DelteCacheZ;
	EXCEPTIONC_Z(pCache == (CacheSeen.GetArrayPtr() + CacheSeen.GetSize()),"Bad Cache position");
//ici pas rentable d'utiliser le filtre 19265 / 56700 dans un cas vraiment bon... 25% ! :(
	Float t2 = GetAbsoluteTime();
//MESSAGE_Z("LEEEE %.3f %.3f",t1-t0,t2-t1);
}

/**************************************************************************/

void RasterizeTriangleAverageZ(Vec3i *p0,Vec3i *p1,Vec3i *p2,U8 *_pBuffer,S32 _SizeX,S32 _SizeY)
{
	// Sort by Y.
	if (p0->y > p1->y)
		::Swap(p0,p1);
	if (p0->y > p2->y)
		::Swap(p0,p2);
	if (p1->y > p2->y)
		::Swap(p1,p2);

/*	if ((p0->y == p2->y) && (p0->x == p2->x == p1->x))
	{
		_pBuffer[p0->y * _SizeX + p0->x] = p0->z;
		return;
	}*/

	S32 z = (S32)(p0->z+p1->z+p2->z) / 3;
	U8 U8z = z;

	// Cut P0-P2.
	S32	CurY = p0->y;
	S32 EndY = Min(p1->y,_SizeY-1);
	
	Float d0SX,d0EX;
	Float d1SX,d1EX;

	Float fCurSX,fCurEX;
	if (p0->y == p1->y)
	{
		if (p0->x > p1->x)
			::Swap(p0,p1);

		d0SX = d0EX = 0.f;
		if (p1->y == p2->y)
		{
			// Line
			if (p0->x > p2->x)	// smallest on p0
				::Swap(p0,p2);
			if (p1->x < p2->x)  // biggest on p1
				::Swap(p1,p2);
			d1SX = d1EX = 0.f;
		}
		else
		{
			// Tri : Horizontal start.
			Float dx02 = (Float)(p2->x - p0->x) / (Float)(p2->y - p0->y);
			Float dx12 = (Float)(p2->x - p1->x) / (Float)(p2->y - p1->y);
			d1SX = dx02;
			d1EX = dx12;
		}
		fCurSX = p0->x + 0.5f;
		fCurEX = p1->x + 0.5f;
	}
	else
	{
		Float dx01 = (Float)(p1->x - p0->x) / (Float)(p1->y - p0->y);
		Float dx02 = (Float)(p2->x - p0->x) / (Float)(p2->y - p0->y);	// 2 can't be == 0
		Float dx12 = 0.f;
		if (p1->y != p2->y)
			dx12 = (Float)(p2->x - p1->x) / (Float)(p2->y - p1->y);

		if (dx02 < dx01)
		{
			d0SX = dx02;
			d0EX = dx01;
			d1SX = dx02;
			d1EX = dx12;
		}
		else
		{
			d0SX = dx01;
			d0EX = dx02;
			d1SX = dx12;
			d1EX = dx02;
		}
	
		fCurSX = p0->x + 0.5f;
		fCurEX = fCurSX;
	}

	U8 *pDatas = _pBuffer + CurY * _SizeX;
	if (CurY < 0)
	{
		if (p2->y < 0)
			return;
		// Clip Up.
		S32 Nb = Min(-CurY,EndY-CurY);
		Float fNb = (Float)Nb;
		fCurSX += d0SX * fNb;
		fCurEX += d0EX * fNb;
		CurY+=Nb;
		pDatas += _SizeX*Nb;
	}
	while (CurY < EndY)
	{
		S32 StartX = Max((S32)0,(S32)fCurSX);
		S32 EndX = Min((S32)fCurEX,_SizeX-1);
		for (S32 x=StartX ; x<=EndX ; x++)
		{
			if (pDatas[x] > U8z)
				pDatas[x] = U8z;
		}
		fCurSX += d0SX;
		fCurEX += d0EX;
		CurY++;
		pDatas += _SizeX;
	}
	EndY = Min(p2->y,_SizeY-1);
	if (CurY < 0)
	{
		// Cut Start.
		S32 Nb = Min(-CurY,EndY-CurY);
		Float fNb = (Float)Nb;
		fCurSX += d1SX*fNb;
		fCurEX += d1EX*fNb;
		CurY+=Nb;
		pDatas += _SizeX*Nb;
	}
	while (CurY <= EndY)
	{
		S32 StartX = Max((S32)0,(S32)fCurSX);
		S32 EndX = Min((S32)fCurEX,_SizeX-1);
		for (S32 x=StartX ; x<=EndX ; x++)
		{
			if (pDatas[x] > U8z)
				pDatas[x] = U8z;
		}
		fCurSX += d1SX;
		fCurEX += d1EX;
		CurY++;
		pDatas += _SizeX;
	}
}

void RasterizeTriangleInterpZ(Vec3i *p0,Vec3i *p1,Vec3i *p2,U8 *_pBuffer,S32 _SizeX,S32 _SizeY)
{
	// Sort by Y.
	if (p0->y > p1->y)
		::Swap(p0,p1);
	if (p0->y > p2->y)
		::Swap(p0,p2);
	if (p1->y > p2->y)
		::Swap(p1,p2);

	// Cut P0-P2.
	S32	CurY = p0->y;
	S32 EndY = Min(p1->y,_SizeY-1);
	
	Float d0SX,d0EX;
	Float d1SX,d1EX;

	Float d0SZ,d0EZ;
	Float d1SZ,d1EZ;

	Float dXZ;

	Float fCurSX,fCurEX;
	Float fCurSZ,fCurEZ;

	if (p0->y == p1->y)
	{
		if (p0->x > p1->x)
			::Swap(p0,p1);

		d0SX = d0EX = 0.f;
		d0SZ = d0EZ = 0.f;
		if (p1->y == p2->y)
		{
			// Line
			if (p0->x > p2->x)	// smallest on p0
				::Swap(p0,p2);
			if (p1->x < p2->x)  // biggest on p1
				::Swap(p1,p2);
			d1SX = d1EX = 0.f;
			d1SZ = d1EZ = 0.f;
		}
		else
		{
			// Tri : Horizontal start.
			Float pente02 = 1.f / (Float)(p2->y - p0->y);
			Float pente12 = 1.f / (Float)(p2->y - p1->y);
			Float dx02 = (Float)(p2->x - p0->x) * pente02;
			Float dx12 = (Float)(p2->x - p1->x) * pente12;
			d1SX = dx02;
			d1EX = dx12;

			Float dz02 = (Float)(p2->z - p0->z) * pente02;
			Float dz12 = (Float)(p2->z - p1->z) * pente12;
			d1SZ = dz02;
			d1EZ = dz12;
		}
		dXZ = 0.f;
		if (p0->x != p1->x)
			dXZ = (Float)(p1->z - p0->z) / (Float)(p1->x - p0->x);

		fCurSX = p0->x + 0.5f;
		fCurEX = p1->x + 0.5f;

		fCurSZ = p0->z + 0.5f;
		fCurEZ = p1->z + 0.5f;
	}
	else
	{
		Float pente01 = 1.f / (Float)(p1->y - p0->y);
		Float Deltay02 = (Float)(p2->y - p0->y);
		Float pente02 = 1.f / Deltay02;

		Float dx01 = (Float)(p1->x - p0->x) * pente01;
		Float dx02 = (Float)(p2->x - p0->x) * pente02;	// 2 can't be == 0
		Float dx12 = 0.f;

		Float dz01 = (Float)(p1->z - p0->z) * pente01;
		Float dz02 = (Float)(p2->z - p0->z) * pente02;	// 2 can't be == 0
		Float dz12 = 0.f;

		if (p1->y != p2->y)
		{
			Float pente12 = 1.f / (Float)(p2->y - p1->y);
			dx12 = (Float)(p2->x - p1->x) * pente12;
			dz12 = (Float)(p2->z - p1->z) * pente12;
		}

		if (dx02 < dx01)
		{
			d0SX = dx02;
			d0EX = dx01;
			d1SX = dx02;
			d1EX = dx12;

			d0SZ = dz02;
			d0EZ = dz01;
			d1SZ = dz02;
			d1EZ = dz12;
		}
		else
		{
			d0SX = dx01;
			d0EX = dx02;
			d1SX = dx12;
			d1EX = dx02;

			d0SZ = dz01;
			d0EZ = dz02;
			d1SZ = dz12;
			d1EZ = dz02;
		}
	
		fCurSX = p0->x + 0.5f;
		fCurEX = fCurSX;

		fCurSZ = p0->z + 0.5f;
		fCurEZ = fCurSZ;

		// Compute Delta Z.
		S32	  ExtraX = p0->x + (S32)(dx01 * Deltay02);
		Float ExtraZ = (Float)p0->z + (dz01 * Deltay02);	// PAS DE +0.5 ICI !!!!

		dXZ = 0.f;
		if (p2->x != ExtraX)
			dXZ = (ExtraZ - (Float)p2->z) / (ExtraX - (Float)p2->x);
	}
	
	// Draw first tri part.
	U8 *pDatas = _pBuffer + CurY * _SizeX;

	if (CurY < 0)
	{
		if (p2->y < 0)
			return;
		// Clip Up.
		S32 Nb = Min(-CurY,EndY-CurY);
		Float fNb = (Float)Nb;
		fCurSX += d0SX * fNb;
		fCurEX += d0EX * fNb;
		fCurSZ += d0SZ * fNb;
		fCurEZ += d0EZ * fNb;
		CurY+=Nb;
		pDatas += _SizeX*Nb;
	}

	while (CurY < EndY)
	{
		Float CurZ = fCurSZ;
		S32 StartX = (S32)fCurSX;
		if (StartX < 0)
		{
			CurZ += dXZ * (Float)(-StartX);
			StartX = 0;
		}
		S32 EndX = Min((S32)fCurEX,_SizeX-1);
		for (S32 x=StartX ; x<=EndX ; x++)
		{
//				if (pDatas <_pBuffer || pDatas>=(_pBuffer+_SizeX*_SizeY))
//					int la = 0;
			U8 U8z = (U8)CurZ;
			if (pDatas[x] > U8z)
				pDatas[x] = U8z;
			CurZ += dXZ;
		}
		fCurSX += d0SX;
		fCurEX += d0EX;
		fCurSZ += d0SZ;
		fCurEZ += d0EZ;
		CurY++;
		pDatas += _SizeX;
	}

	// Draw second tri part.
	EndY = Min(p2->y,_SizeY-1);
	if (CurY < 0)
	{
		// Cut Start.
		S32 Nb = Min(-CurY,EndY-CurY);
		Float fNb = (Float)Nb;
		fCurSX += d1SX*fNb;
		fCurEX += d1EX*fNb;
		fCurSZ += d1SZ*fNb;
		fCurEZ += d1EZ*fNb;
		CurY+=Nb;
		pDatas += _SizeX*Nb;
	}
	while (CurY <= EndY)
	{
		Float CurZ = fCurSZ;
		S32 StartX = (S32)fCurSX;
		if (StartX < 0)
		{
			CurZ += dXZ * (Float)(-StartX);
			StartX = 0;
		}
		S32 EndX = Min((S32)fCurEX,_SizeX-1);
		for (S32 x=StartX ; x<=EndX ; x++)
		{
//				if (pDatas <_pBuffer || pDatas>=(_pBuffer+_SizeX*_SizeY))
//					int la = 0;
			U8 U8z = (U8)CurZ;
			if (pDatas[x] > U8z)
				pDatas[x] = U8z;
			CurZ += dXZ;
		}
		fCurSX += d1SX;
		fCurEX += d1EX;
		fCurSZ += d1SZ;
		fCurEZ += d1EZ;
		CurY++;
		pDatas += _SizeX;
	}
}

class RasterPt
{
public:
	Vec3i	pos;
	U8		IsComputed;
	U8		IsVisible;
};

static Float RSTimeT1 = 0;
static Float RSTimeT2 = 0;

void	Playspace_SR_BlindMap::AddMeshToZBuffer(HU8DA &_ZBuffer,U32 *_pTabTriIdx, S32 _NbTriIdx, U32 _ModuloTri, Vec3f *_pTabVtx, S32 _NbVtx,const Vec3f &_Pos, const Vec3f &_Dir, Float _Angle, Float _MaxDist)
{
	// Attention la rasterization utilise le Z pour aller plus vite pas le 1/Z.
	// => DONC Interpolation sur triangle est fausse si le triangle est grand

	if (!IsInit())
		return;
	if (!_NbTriIdx || !_NbVtx)
		return;

	Float t0 = GetAbsoluteTime();

	// Compute Up Left Axis for ZBuffer.
	Vec3f	ZBufferUp;
	Vec3f	ZBufferLeft;
	Vec3f	ACamDir(Abs(_Dir.x),Abs(_Dir.y),Abs(_Dir.z));
	if ((ACamDir.y > ACamDir.x) && (ACamDir.y > ACamDir.z))
	{
		// Up is biggest.
		if (ACamDir.x > ACamDir.z)
			// Get Front as reference (because Front is lowest)
			ZBufferUp = VEC3F_FRONT;
		else
			// Get Left as reference (because Left is lowest)
			ZBufferUp = VEC3F_LEFT;
	}
	else
	{
		// Get Up as reference (bacesaue Up is not Biggest)
		ZBufferUp = VEC3F_UP;
	}

	ZBufferLeft = _Dir^ZBufferUp;
	ZBufferLeft.CNormalize();
	ZBufferUp = ZBufferLeft^_Dir;
	ZBufferUp.CNormalize();

	// Compute Angles.
	Vec2f	ConeSinCos;
	SinCos(ConeSinCos, _Angle);

	Float SinAngle = ConeSinCos.x;
	Float CosAngle2 = ConeSinCos.y*ConeSinCos.y;
	Float TanAngle = ConeSinCos.x / ConeSinCos.y;
	Float DeltaR = SinAngle * _MaxDist;

	Float MaxDist2 = _MaxDist*_MaxDist;

	// Compute Min-Max.
	Vec3f vMin = _Pos;
	Vec3f vMax = _Pos;
	Vec3f ConeEnd = _Pos + _Dir*_MaxDist;

	vMin.x = Min(vMin.x, ConeEnd.x - DeltaR);
	vMin.y = Min(vMin.y, ConeEnd.y - DeltaR);
	vMin.z = Min(vMin.z, ConeEnd.z - DeltaR);

	vMax.x = Max(vMax.x, ConeEnd.x + DeltaR);
	vMax.y = Max(vMax.y, ConeEnd.y + DeltaR);
	vMax.z = Max(vMax.z, ConeEnd.z + DeltaR);

	// First : Add Points.
	HugeDynArray_Z<RasterPt,32,FALSE,FALSE,4,TRUE>		TabRasterPoints;
	TabRasterPoints.SetSize(_NbVtx);
	TabRasterPoints.Null();
	S32 NbPoints = _NbVtx;

	Bool OneIsVisible = FALSE;

	for (S32 i = 0; i<NbPoints ; i++)
	{
		Vec3f pos = _pTabVtx[i];
		if (   (pos.x < vMin.x) || (pos.x > vMax.x)
			|| (pos.z < vMin.z) || (pos.z > vMax.z)
			|| (pos.y < vMin.y) || (pos.y > vMax.y)
			)
			continue;	// Sur PC, plus rapide... sur le kit ?

		// Is In Cone ?
		Vec3f Delta = pos - _Pos;
		Float Proj = Delta * _Dir;
		if ((Proj < 0.001f) || (Proj >= _MaxDist))
			continue;

		Float fDelta2 = Delta * Delta;
		if (Proj*Proj < (CosAngle2 * fDelta2))
			continue;

		OneIsVisible = TRUE;

		// Project !
		RasterPt &RasterInfo = TabRasterPoints[i];

		Float LocalR = Proj * TanAngle;
		Float iLocalR = SR_BLINDRAY_ZBUFF_FHSIZE / LocalR;
		Float	fZBx = (Delta * ZBufferLeft) * iLocalR;
		Float	fZBy = (Delta * ZBufferUp) * iLocalR;

		// Init Point.
		RasterInfo.IsVisible = 1;
		RasterInfo.IsComputed = 1;
		Vec3i &ScanPos = RasterInfo.pos;
		ScanPos.x = (S32)(fZBx+SR_BLINDRAY_ZBUFF_FHSIZE);
		ScanPos.y = (S32)(fZBy+SR_BLINDRAY_ZBUFF_FHSIZE);
		ScanPos.z = (S32)(Proj * 254.f/ _MaxDist);
	}

	Float t1 = GetAbsoluteTime();
	RSTimeT1 += t1 - t0;

	// Nothing visible
	if (!OneIsVisible)
		return;

	// Rasterize Faces.
	S32 NbFaces = _NbTriIdx;
	U32 *pCurFaceIdx = _pTabTriIdx;
	S32 ModuloTriU32 = _ModuloTri >> 2;
	U8 *ZBuffer = _ZBuffer.GetArrayPtr();

	for (S32 i = 0; i<NbFaces ; i++,pCurFaceIdx+=ModuloTriU32)
	{
		// Access Face.
		U32 Id0 = pCurFaceIdx[0];
		U32 Id1 = pCurFaceIdx[1];
		U32 Id2 = pCurFaceIdx[2];

		RasterPt &rp0 = TabRasterPoints[Id0];
		RasterPt &rp1 = TabRasterPoints[Id1];
		RasterPt &rp2 = TabRasterPoints[Id2];

		S32 NbVisible = rp0.IsVisible + rp1.IsVisible + rp2.IsVisible;
		if (!NbVisible)
			continue;
		if ((NbVisible != 3) && ((rp0.IsComputed + rp1.IsComputed + rp2.IsComputed) != 3))
		{
			// Compute needed.
			for (S32 j = 0; j<3 ; j++)
			{
				S32 CurId = pCurFaceIdx[j];
				RasterPt &RasterInfo = TabRasterPoints[CurId];
				if (RasterInfo.IsComputed)
					continue;

				// Project !
				Vec3f pos = _pTabVtx[CurId];
				Vec3f Delta = (pos - _Pos);
				Float Proj = Delta * _Dir;
				if (Proj < 0.01f)
					Proj = 0.01f;

				Float LocalR = Proj * TanAngle;
				Float iLocalR = SR_BLINDRAY_ZBUFF_FHSIZE / LocalR;
				Float	fZBx = (Delta * ZBufferLeft) * iLocalR;
				Float	fZBy = (Delta * ZBufferUp) * iLocalR;

				// Init Point.
				RasterInfo.IsComputed = 1;
				Vec3i &ScanPos = RasterInfo.pos;
				ScanPos.x = (S32)(fZBx+SR_BLINDRAY_ZBUFF_FHSIZE);
				ScanPos.y = (S32)(fZBy+SR_BLINDRAY_ZBUFF_FHSIZE);
				if (Proj >= _MaxDist)
					ScanPos.z = 254;
				else
					ScanPos.z = (S32)(Proj * 254.f/ _MaxDist);
			}
		}
		// Rasterize.
		RasterizeTriangleInterpZ(&rp0.pos,&rp1.pos,&rp2.pos,ZBuffer,SR_BLINDRAY_ZBUFF_SIZE,SR_BLINDRAY_ZBUFF_SIZE);
	}
	Float t2 = GetAbsoluteTime();
	RSTimeT2 += t2 - t1;
}

/**************************************************************************/

void	Playspace_SR_BlindMap::AddMeshToZBuffer(HU8DA	&_ZBuffer,Playspace_Mesh &_Mesh,const Vec3f &_Pos, const Vec3f &_Dir, Float _Angle, Float _MaxDist)
{
	AddMeshToZBuffer(_ZBuffer,(U32*)_Mesh.m_TabQuad[0].TabPoints,_Mesh.m_TabQuad.GetSize(),sizeof(Playspace_Mesh::Face), _Mesh.m_TabPoints.GetArrayPtr(), _Mesh.m_TabPoints.GetSize(),_Pos,_Dir,_Angle,_MaxDist);
}

/**************************************************************************/

void	Playspace_SR_BlindMap::ResetZBuffer(HU8DA	&_ZBuffer)
{
	if (!IsInit())
		return;
	_ZBuffer.SetSize(SR_BLINDRAY_ZBUFF_SIZE * SR_BLINDRAY_ZBUFF_SIZE);
	memset(_ZBuffer.GetArrayPtr(),-1,SR_BLINDRAY_ZBUFF_SIZE * SR_BLINDRAY_ZBUFF_SIZE);
}

/**************************************************************************/

void	Playspace_SR_BlindMap::GetPointInfos(Vec3f *pTabPoints, S32 _NbPoints, PSR_BlindMap_VoxelHUDA &_TabVoxels)
{
	_TabVoxels.SetSize(_NbPoints,TRUE);
	for (S32 i = 0; i<_NbPoints; i++)
	{
		Vec3f pos = *pTabPoints++;
		S32 x = (S32)((pos.x - m_fOrigin.x) * SR_BLINDRAY_MAP_TO_CELL);
		S32 y = (S32)((pos.y - m_fOrigin.y) * SR_BLINDRAY_MAP_TO_CELL);
		S32 z = (S32)((pos.z - m_fOrigin.z) * SR_BLINDRAY_MAP_TO_CELL);
		if (((x < 0) || (x >= SR_BLINDRAY_MAP_SIZE))
			|| ((y < 0) || (y >= SR_BLINDRAY_MAP_SIZE))
			|| ((z < 0) || (z >= SR_BLINDRAY_MAP_SIZE))
			)
		{
			_TabVoxels[i].Flags = PSR_BlindMap_Voxel::IsOut;
			_TabVoxels[i].SeenCount;
			continue;
		}

		_TabVoxels[i] = m_BlindMap[(((z << SR_BLINDRAY_MAP_SHIFT) + x) << SR_BLINDRAY_MAP_SHIFT) + y];
	}
}

/**************************************************************************/

void	Playspace_SR_BlindMap::GetPointNormalInfos(Vec3f *pTabPoints, Playspace_Mesh::ToolPointNormal *pTabNormal, S32 _NbPoints, Float _fDeltaNormal, PSR_BlindMap_VoxelHUDA &_TabVoxels)
{
	_TabVoxels.SetSize(_NbPoints, TRUE);
	for (S32 i = 0; i<_NbPoints; i++)
	{
		Vec3f pos = *pTabPoints++;
		Vec3f Normal = pTabNormal->Normal; pTabNormal++;

		pos += Normal*_fDeltaNormal;

		S32 x = (S32)((pos.x - m_fOrigin.x) * SR_BLINDRAY_MAP_TO_CELL);
		S32 y = (S32)((pos.y - m_fOrigin.y) * SR_BLINDRAY_MAP_TO_CELL);
		S32 z = (S32)((pos.z - m_fOrigin.z) * SR_BLINDRAY_MAP_TO_CELL);
		if (((x < 0) || (x >= SR_BLINDRAY_MAP_SIZE))
			|| ((y < 0) || (y >= SR_BLINDRAY_MAP_SIZE))
			|| ((z < 0) || (z >= SR_BLINDRAY_MAP_SIZE))
			)
		{
			_TabVoxels[i].Flags = PSR_BlindMap_Voxel::IsOut;
			_TabVoxels[i].SeenCount;
			continue;
		}

		_TabVoxels[i] = m_BlindMap[(((z << SR_BLINDRAY_MAP_SHIFT) + x) << SR_BLINDRAY_MAP_SHIFT) + y];
	}
}

/**************************************************************************/

void	Playspace_SR_BlindMap::RefreshMergePaintToMesh(Playspace_Mesh &_Mesh, Float _fUseDeltaNormal)
{
	S32		NbPoint = _Mesh.m_TabPoints.GetSize();
	if (!NbPoint)
		return;
	if (!IsInit())
		return;

	// Get Points infos.
	PSR_BlindMap_VoxelHUDA	TabSeen;

	if (_fUseDeltaNormal > 1e-6f)
	{
		EXCEPTIONC_Z(_Mesh.m_TabPointsToolNormal.GetArrayPtr() != 0,"RefreshMergePaintToMesh => Need normal in this mode");
		GetPointNormalInfos(_Mesh.m_TabPoints.GetArrayPtr(), _Mesh.m_TabPointsToolNormal.GetArrayPtr(), NbPoint, _fUseDeltaNormal, TabSeen);
	}
	else
	{
		GetPointInfos(_Mesh.m_TabPoints.GetArrayPtr(), NbPoint, TabSeen);
	}

	// Refresh Face Visibility.
	S32						NbFaces = _Mesh.m_TabQuad.GetSize();
	Playspace_Mesh::Face	*pFaces = _Mesh.m_TabQuad.GetArrayPtr();
	for (S32 i = 0; i<NbFaces; i++)
	{
		PSR_BlindMap_Voxel &v0 = TabSeen[pFaces->TabPoints[0]];
		PSR_BlindMap_Voxel &v1 = TabSeen[pFaces->TabPoints[1]];
		PSR_BlindMap_Voxel &v2 = TabSeen[pFaces->TabPoints[2]];
		PSR_BlindMap_Voxel &v3 = TabSeen[pFaces->TabPoints[3]];

		U8	CountMax = Max(Max(Max(v0.SeenCount,v1.SeenCount),v2.SeenCount),v3.SeenCount);
		if (CountMax < QUALITY_BLIND_1)
			pFaces->IsSeenQuality = Max(pFaces->IsSeenQuality,(U8)1);
		else if (CountMax < QUALITY_BLIND_2)
			pFaces->IsSeenQuality = Max(pFaces->IsSeenQuality,(U8)2);
		else
			pFaces->IsSeenQuality = Max(pFaces->IsSeenQuality,(U8)3);

		U8	OrFlags = (v0.Flags | v1.Flags | v2.Flags | v3.Flags);
		if (OrFlags & PSR_BlindMap_Voxel::IsPaint)
			pFaces->IsPaintMode = Max(pFaces->IsPaintMode,(U8)2);
		else if (OrFlags & PSR_BlindMap_Voxel::IsUnPaint)
			pFaces->IsPaintMode = Min(pFaces->IsPaintMode,(U8)1);

		pFaces++;
	}
}

/**************************************************************************/

void	Playspace_SR_BlindMap::ApplyQuatAndMinMax(const Quat &_Transfo,const Vec3f &_Min,const Vec3f &_Max)
{
	// No Apply quat if no BlindMap.
	if (!m_BlindMap.GetSize())
		return;

	PSR_BlindMap_VoxelHUDA	CopyBlindMap;

	// Swap with work Map.
	CopyBlindMap.SetSize(m_BlindMap.GetSize());
	CopyBlindMap.Swap(m_BlindMap);
	m_BlindMap.Null();

	// New Origin.
	Vec3f m_OldfOrigin = m_fOrigin;

	m_fCenter = _Transfo * m_fCenter;
	Playspace_SR_W::SnapPos(m_fCenter);

	Float DeltaCenter = (Float)(SR_BLINDRAY_MAP_SIZE >> 1) * SR_BLINDRAY_MAP_TO_WORLD;
	m_fOrigin = m_fCenter - Vec3f(DeltaCenter,DeltaCenter,DeltaCenter);

	Float HalfCell = SR_BLINDRAY_MAP_TO_WORLD * 0.5f;
	m_fOrigin_Half = m_fOrigin + Vec3f(HalfCell,HalfCell,HalfCell);

	// Transfert information.
	U8		ClearMask = 0xFF ^ (PSR_BlindMap_Voxel::IsOut | PSR_BlindMap_Voxel::IsBlock);
	Quat	QuatInvTransfo = -_Transfo;
	Mat4x4	MatInvTranfo;
	QuatInvTransfo.GetMatrix(MatInvTranfo);

	Vec4f	Pos;
	Pos.w = 1.f;
	for (S32 z = 0; z<SR_BLINDRAY_MAP_SIZE ; z++)
	{
		Pos.z = (Float)(z) * SR_BLINDRAY_MAP_TO_WORLD + m_fOrigin_Half.z;
		for (S32 x = 0; x<SR_BLINDRAY_MAP_SIZE; x++)
		{
			PSR_BlindMap_Voxel *pCur = &m_BlindMap[(((z << SR_BLINDRAY_MAP_SHIFT) + x) << SR_BLINDRAY_MAP_SHIFT)];
			Pos.x = (Float)(x) * SR_BLINDRAY_MAP_TO_WORLD + m_fOrigin_Half.x;

			for (S32 y = 0; y<SR_BLINDRAY_MAP_SIZE; y++,pCur++)
			{
				Pos.y = (Float)(y) * SR_BLINDRAY_MAP_TO_WORLD + m_fOrigin_Half.y;
				Vec4f PrevPos = VecFloat4x4Transform3(MatInvTranfo,Pos);

				S32 ox = (S32)((PrevPos.x - m_OldfOrigin.x) * SR_BLINDRAY_MAP_TO_CELL);
				if ((ox < 0) || (ox >= SR_BLINDRAY_MAP_SIZE))
					continue;
				S32 oz = (S32)((PrevPos.z - m_OldfOrigin.z) * SR_BLINDRAY_MAP_TO_CELL);
				if ((oz < 0) || (oz >= SR_BLINDRAY_MAP_SIZE))
					continue;
				S32 oy = (S32)((PrevPos.y - m_OldfOrigin.y) * SR_BLINDRAY_MAP_TO_CELL);
				if ((oy < 0) || (oy >= SR_BLINDRAY_MAP_SIZE))
					continue;

				PSR_BlindMap_Voxel &CurVoxel = CopyBlindMap[(((oz << SR_BLINDRAY_MAP_SHIFT) + ox) << SR_BLINDRAY_MAP_SHIFT) + oy];
				pCur->Flags = CurVoxel.Flags & ClearMask;
				pCur->SeenCount = CurVoxel.SeenCount;
			}
		}
	}

	// And Relock Min Max.
	LockMinMax(_Min, _Max);

}

/**************************************************************************/

Bool	Playspace_SR_BlindMap::ApplyMoveXZAndMinMax(Vec3f _NewCenter, const Vec3f &_Min, const Vec3f &_Max)
{
	// No Move if no BlindMap.
	if (!m_BlindMap.GetSize())
		return FALSE;

	Vec2f	Delta2D(_NewCenter.x - m_fCenter.x,_NewCenter.z - m_fCenter.z);

	Delta2D *= SR_BLINDRAY_MAP_TO_CELL;
	S32 dx = FLOOR(Delta2D.x + 0.5f);
	S32 dy = FLOOR(Delta2D.y + 0.5f);
	EXCEPTIONC_Z(Abs(Delta2D.x - dx) < 0.001f,"No Alignement Playspace_SR_BlindMap:MoveXZ");
	EXCEPTIONC_Z(Abs(Delta2D.y - dy) < 0.001f,"No Alignement Playspace_SR_BlindMap:MoveXZ");

	if (!dx && !dy)
		return FALSE;


	// Compute Deltas
	S32 NbCellX;
	S32 DeltaSrcX;
	S32 DeltaDstX;
	if (dx > 0)
	{
		DeltaSrcX = dx;
		DeltaDstX = 0;
		NbCellX = SR_BLINDRAY_MAP_SIZE - dx;
	}
	else
	{
		DeltaSrcX = 0;
		DeltaDstX = -dx;
		NbCellX = SR_BLINDRAY_MAP_SIZE + dx;
	}

	S32 NbCellY;
	S32 DeltaSrcY;
	S32 DeltaDstY;
	if (dy > 0)
	{
		DeltaSrcY = dy;
		DeltaDstY = 0;
		NbCellY = SR_BLINDRAY_MAP_SIZE - dy;
	}
	else
	{
		DeltaSrcY = 0;
		DeltaDstY = -dy;
		NbCellY = SR_BLINDRAY_MAP_SIZE + dy;
	}

	PSR_BlindMap_VoxelHUDA	CopyBlindMap;

	// Swap with work Map.
	CopyBlindMap.SetSize(m_BlindMap.GetSize());
	CopyBlindMap.Swap(m_BlindMap);
	m_BlindMap.Null();

	// New Origin.
	Vec3f m_OldfOrigin = m_fOrigin;

	m_fCenter = _NewCenter;
	Float DeltaCenter = (Float)(SR_BLINDRAY_MAP_SIZE >> 1) * SR_BLINDRAY_MAP_TO_WORLD;
	m_fOrigin = m_fCenter - Vec3f(DeltaCenter,DeltaCenter,DeltaCenter);
	Float HalfCell = SR_BLINDRAY_MAP_TO_WORLD * 0.5f;
	m_fOrigin_Half = m_fOrigin + Vec3f(HalfCell,HalfCell,HalfCell);

	// Transfert information.
	U8		ClearMask = 0xFF ^ (PSR_BlindMap_Voxel::IsOut | PSR_BlindMap_Voxel::IsBlock);
	for (S32 z = 0; z<NbCellY ; z++)
	{
		PSR_BlindMap_Voxel *pSrc = CopyBlindMap.GetArrayPtr() + ((((z+DeltaSrcY)<<SR_BLINDRAY_MAP_SHIFT) + DeltaSrcX)<<SR_BLINDRAY_MAP_SHIFT);
		PSR_BlindMap_Voxel *pDst = m_BlindMap.GetArrayPtr() + ((((z+DeltaDstY)<<SR_BLINDRAY_MAP_SHIFT) + DeltaDstX)<<SR_BLINDRAY_MAP_SHIFT);

		for (S32 x = 0; x<NbCellX; x++)
		{
			for (S32 y = 0; y<SR_BLINDRAY_MAP_SIZE; y++)
			{
				pDst->Flags = pSrc->Flags & ClearMask;
				pDst->SeenCount = pSrc->SeenCount;
				pSrc++;
				pDst++;
			}
		}
	}

	// And Relock Min Max.
	LockMinMax(_Min, _Max);

	return TRUE;
}

/**************************************************************************/

void	Playspace_SR_BlindMap::LockMinMax(const Vec3f &_Min,const Vec3f &_Max)
{
	// First, All is OUT.
	S32 Size = m_BlindMap.GetSize();
	if (!Size)
		return;
	EXCEPTIONC_Z((Size & 0x3) == 0,"BAD SIZE Playspace_SR_BlindMap");

	PSR_BlindMap_Voxel *pReset = m_BlindMap.GetArrayPtr();
	PSR_BlindMap_Voxel *pResetEnd = pReset + m_BlindMap.GetSize();
	while (pReset<pResetEnd)
	{
		pReset->Flags |= PSR_BlindMap_Voxel::IsOut;
		pReset++;
	}

	EXCEPTIONC_Z(pReset == (m_BlindMap.GetArrayPtr() + m_BlindMap.GetSize()), "BAD Reset Playspace_SR_BlindMap");

	// And Set what is OK.
	Vec3i Start, End;
	Start.x = (S32)((_Min.x - m_fOrigin.x) * SR_BLINDRAY_MAP_TO_CELL + 0.5f);
	Start.y = (S32)((_Min.y - m_fOrigin.y) * SR_BLINDRAY_MAP_TO_CELL + 0.5f);
	Start.z = (S32)((_Min.z - m_fOrigin.z) * SR_BLINDRAY_MAP_TO_CELL + 0.5f);

	if (Start.x < 0) Start.x = 0;
	if (Start.x >= SR_BLINDRAY_MAP_SIZE) Start.x = SR_BLINDRAY_MAP_SIZE - 1;

	if (Start.y < 0) Start.y = 0;
	if (Start.y >= SR_BLINDRAY_MAP_SIZE) Start.y = SR_BLINDRAY_MAP_SIZE - 1;

	if (Start.z < 0) Start.z = 0;
	if (Start.z >= SR_BLINDRAY_MAP_SIZE) Start.z = SR_BLINDRAY_MAP_SIZE - 1;

	End.x = (S32)((_Max.x - m_fOrigin.x) * SR_BLINDRAY_MAP_TO_CELL + 0.5f);
	End.y = (S32)((_Max.y - m_fOrigin.y) * SR_BLINDRAY_MAP_TO_CELL + 0.5f);
	End.z = (S32)((_Max.z - m_fOrigin.z) * SR_BLINDRAY_MAP_TO_CELL + 0.5f);

	if (End.x < 0) End.x = 0;
	if (End.x >= SR_BLINDRAY_MAP_SIZE) End.x = SR_BLINDRAY_MAP_SIZE - 1;

	if (End.y < 0) End.y = 0;
	if (End.y >= SR_BLINDRAY_MAP_SIZE) End.y = SR_BLINDRAY_MAP_SIZE - 1;

	if (End.z < 0) End.z = 0;
	if (End.z >= SR_BLINDRAY_MAP_SIZE) End.z = SR_BLINDRAY_MAP_SIZE - 1;

	S32 CurZ = Start.z;
	U8 ClearMask = 0xFF ^ PSR_BlindMap_Voxel::IsOut;
	while (CurZ <= End.z)
	{
		S32 CurX = Start.x;
		while (CurX <= End.x)
		{
			PSR_BlindMap_Voxel *pCurY = &m_BlindMap[(((CurZ << SR_BLINDRAY_MAP_SHIFT) + CurX) << SR_BLINDRAY_MAP_SHIFT) + Start.y];
			PSR_BlindMap_Voxel *pEndY = pCurY + End.y - Start.y;

			EXCEPTIONC_Z(pCurY >= m_BlindMap.GetArrayPtr(), "OUT OF Playspace_SR_BlindMap::LockMinMax");
			EXCEPTIONC_Z(pEndY < (m_BlindMap.GetArrayPtr() + m_BlindMap.GetSize()), "OUT OF Playspace_SR_BlindMap::LockMinMax");

			while (pCurY <= pEndY)
			{
				pCurY->Flags &= ClearMask;
				pCurY++;
			}
			CurX++;
		}
		CurZ++;
	}
}


/**************************************************************************/

void	Playspace_SR_BlindMap::Draw()
{
	if (!m_BlindMap.GetSize())
		return;
	Segment_Z RefView;
	Util_L::GetViewSegment(&RefView);
	Vec3f	vCam = RefView.Org;
	Vec3f	vDir = RefView.Dir;
	Float ConeCos2 = Cos(DegToRad(15.f));
	ConeCos2*=ConeCos2;
	Float DistMax2 = 2.f*2.f;

	Vec3f	Pos;
	for (S32 z = 0; z<SR_BLINDRAY_MAP_SIZE; z++)
	{
		Pos.z = (Float)(z) * SR_BLINDRAY_MAP_TO_WORLD + m_fOrigin_Half.z;
		for (S32 x = 0; x<SR_BLINDRAY_MAP_SIZE; x++)
		{
			PSR_BlindMap_Voxel *pCur = &m_BlindMap[(((z << SR_BLINDRAY_MAP_SHIFT) + x) << SR_BLINDRAY_MAP_SHIFT)];
			Pos.x = (Float)(x) * SR_BLINDRAY_MAP_TO_WORLD + m_fOrigin_Half.x;

			for (S32 y = 0; y<SR_BLINDRAY_MAP_SIZE; y++, pCur++)
			{
//				if (*pCur & Playspace_SR_BlindMap::IsOut)
//					continue;
//				if (!(*pCur & Playspace_SR_BlindMap::IsInCone))
//					continue;

				Pos.y = (Float)(y) * SR_BLINDRAY_MAP_TO_WORLD + m_fOrigin_Half.y;

				Vec3f Delta = Pos - vCam;
				Float ProjDist = Delta * vDir;
				if (ProjDist < 0.f)
					continue;
				if (ProjDist*ProjDist < (Delta.GetNorm2()*ConeCos2))
					continue;
				if (Delta.GetNorm2() >= DistMax2)
					continue;

				if (pCur->Flags & PSR_BlindMap_Voxel::IsBlock)
				{
					if (pCur->Flags & PSR_BlindMap_Voxel::IsSeen)
					{
						if (pCur->SeenCount < QUALITY_BLIND_1)
						{
							DRAW_DEBUG_SPHERE3D(Pos, COLOR_RED, 0.04f);
						}
						else if (pCur->SeenCount < QUALITY_BLIND_2)
						{
							DRAW_DEBUG_SPHERE3D(Pos, COLOR_ORANGE, 0.04f);
						}
						else
						{
							DRAW_DEBUG_SPHERE3D(Pos, COLOR_GREEN, 0.04f);
						}
					}
/*					if (pCur->Flags & PSR_BlindMap_Voxel::IsPaint)
					{
						DRAW_DEBUG_SPHERE3D(Pos, COLOR_GREEN, 0.04f);
					}
					else if (pCur->Flags & PSR_BlindMap_Voxel::IsUnPaint)
					{
						DRAW_DEBUG_SPHERE3D(Pos, COLOR_RED, 0.04f);
					}
					else if (pCur->SeenCount > 0)
					{
						DRAW_DEBUG_SPHERE3D(Pos, COLOR_BLUE, 0.04f);
					}*/
				}
			}
		}
	}
}

/**************************************************************************/
/* SR UPDATER                                                             */
/**************************************************************************/

Playspace_SR_W::Playspace_SR_W()
{
	pFreeBlock = NULL;
	m_LastFrameUpdate = 0;
	m_TypeRefresh = REFRESH_SR_NONE;
	m_Min = Vec3f(-1e5f, -1e5f, -1e5f);
	m_Max = Vec3f(+1e5f, +1e5f, +1e5f);
	m_SpeedCheckTime30FPS = 0;
	m_IdFirstCone = m_IdLastCone = 0;
	InvalidateConeView();
	Playspace_SR_ConeView	TabCone[32];
	m_SnapGranularity = 0.08f;
}

/**************************************************************************/

Playspace_SR_W::~Playspace_SR_W()
{
}

/**************************************************************************/

Playspace_SR_Status	Playspace_SR_W::GetRefreshStatus()
{
	return Playspace_SR_Status::STATUS_OK;
}

/**************************************************************************/

void	Playspace_SR_W::Flush(Bool _OnlyIfDevice)
{
	m_Min = Vec3f(-1e5f, -1e5f, -1e5f);
	m_Max = Vec3f(+1e5f, +1e5f, +1e5f);

	m_SpeedCheckTime30FPS = 0;
	m_IdFirstCone = m_IdLastCone = 0;
	InvalidateConeView();

	FlushBlind();
	if (_OnlyIfDevice)
		return;
	FlushBlocks();
	m_TypeRefresh = REFRESH_SR_NONE;
}

/**************************************************************************/

void		Playspace_SR_W::FlushBlocks()
{
	pFreeBlock = NULL;
	m_TabBlocks.Flush();
}

/**************************************************************************/

void		Playspace_SR_W::MarkBlindMapFromSR(Bool _FullRefresh,PSR_BlindMap_Voxel::Flag _Flag)
{
	S32		NbBlock = m_TabBlocks.GetSize();

Float t0 = GetAbsoluteTime();
	for (S32 i = 0; i < NbBlock; i++)
	{
		Playspace_SR_Block	&CurBlock = m_TabBlocks[i];
		if (!CurBlock.m_BlockHdl.IsValid())
			continue;
		S32 NbVertex = CurBlock.m_TabVtx.GetSize();
		if (!NbVertex)
			continue;
		if (!_FullRefresh && !CurBlock.m_NeedBlindRefresh)
			continue;
		m_BlindMap.AddMesh(CurBlock.m_TabFaceIdx.GetArrayPtr(),CurBlock.m_TabFaceIdx.GetSize()/3,3*sizeof(U32),CurBlock.m_TabVtx.GetArrayPtr(), NbVertex,_Flag);
		CurBlock.m_NeedBlindRefresh = FALSE;
//		m_BlindMap.AddPointList(CurBlock.m_TabVtx.GetArrayPtr(), NbVertex,_Flag);
	}
	m_BlindMap.AddMeshEnd();

Float t1 = GetAbsoluteTime();
//	MESSAGE_Z("STATS AddMesh %.3f ");

	
	//MESSAGE_Z("AddMesh : %.3f", t1 - t0);
}

/**************************************************************************/

void		Playspace_SR_W::ReleaseBlock(Playspace_SR_Block *_pBlock)
{
	_pBlock->pNext = pFreeBlock;
	pFreeBlock = _pBlock;
	_pBlock->m_BlockHdl.Init();
	_pBlock->m_TabVtx.Flush();
	_pBlock->m_TabFaceIdx.Flush();
}

/**************************************************************************/

Playspace_SR_Block	*Playspace_SR_W::GetNewBlock()
{
	Playspace_SR_Block *pNewBlock;
	if (pFreeBlock)
	{
		pNewBlock = pFreeBlock;
		pFreeBlock = pFreeBlock->pNext;
	}
	else
	{
		pNewBlock = &(m_TabBlocks.Add());
	}

	// Init
	pNewBlock->m_FrameRefresh = 0;
	pNewBlock->pNext = NULL;
	return pNewBlock;
}

/**************************************************************************/

Playspace_SR_Block	*Playspace_SR_W::GetBlock(Playspace_SR_BlockHdl &_BlockHdl, Bool _CreateIfNeeded)
{
	S32 NbBlocks = m_TabBlocks.GetSize();
	for (S32 i = 0; i<NbBlocks ; i++)
	{
		if (m_TabBlocks[i].m_BlockHdl == _BlockHdl)
			return &m_TabBlocks[i];
	}
	if (!_CreateIfNeeded)
		return NULL;

	Playspace_SR_Block *pNewBlock = GetNewBlock();
	pNewBlock->m_BlockHdl = _BlockHdl;
	return pNewBlock;
}


/**************************************************************************/

Playspace_SR_Block	*Playspace_SR_W::GetBlockByCenterPos(Vec3f &_Pos, Float _DistMax2)
{
	S32		NbBlocks = m_TabBlocks.GetSize();
	Float	BetterDist2 = _DistMax2;
	S32		BetterBlock = -1;
	for (S32 i = 0; i<NbBlocks; i++)
	{
		Float Dist2 = (m_TabBlocks[i].m_Center - _Pos).GetNorm2();
		if (Dist2 < BetterDist2)
		{
			BetterDist2 = Dist2;
			BetterBlock = i;
		}
	}
	if (BetterBlock < 0)
		return NULL;

	return &m_TabBlocks[BetterBlock];
}

/**************************************************************************/
Bool	LockCone= FALSE;
void	Playspace_SR_W::AddConeView(const Vec3f &_Pos, const Vec3f &_Dir, Float _DegDemiAngle, Float _MaxDist)
{
	if (LockCone)
		return;

	Playspace_SR_Status Status = GetRefreshStatus();
	if (Status != Playspace_SR_Status::STATUS_OK)
	{
		if (Status == Playspace_SR_Status::STATUS_FREEZE)
		{
			// Flush Cone manage.
			m_SpeedCheckTime30FPS = 0;
			m_IdFirstCone = m_IdLastCone = 0;
			InvalidateConeView();
		}
		return;
	}

	// Check Speed..
	U32 CurFrame30FPS = (U32)(GetAbsoluteTime() * 30.f);

	Bool IsSpeedOK = FALSE;

	if (m_SpeedCheckTime30FPS && (CurFrame30FPS > m_SpeedCheckTime30FPS))
	{
		// Check IT !
		Float DTime = (Float)(CurFrame30FPS - m_SpeedCheckTime30FPS) * 1 / 30.f;

		Float Speed = (_Pos - m_SpeedCheckPos).GetNorm() / DTime;
		Float AngleCos = _Dir*m_SpeedCheckDir;
		Float AngleSpeed = 0.f;
		if (AngleCos < -0.9999f)
			AngleSpeed = M_PI;
		else if (AngleCos < 0.9999f)
			AngleSpeed = ACos(AngleCos) / DTime;

//MESSAGE_Z("Speed %f %f",Speed,AngleSpeed);
		if ((Speed < 2.f) && (AngleSpeed < M_PI_2))
			IsSpeedOK = TRUE;
	}

	m_SpeedCheckTime30FPS = CurFrame30FPS;
	m_SpeedCheckPos = _Pos;
	m_SpeedCheckDir = _Dir;

//	if (!IsSpeedOK)
//		return;

	// Check Cone.
	if (m_IdFirstCone != m_IdLastCone)
	{
		// Same Cone ?
		if (m_TabConeView[(m_IdLastCone-1) & SR_CONE_TAB_MSK].Frame30FPS == CurFrame30FPS)
			return;
	}

	Playspace_SR_ConeView &CurCone = m_TabConeView[m_IdLastCone];

	m_IdLastCone = (m_IdLastCone+1) & SR_CONE_TAB_MSK;
	if (m_IdLastCone == m_IdFirstCone)
	{
		// Kill Oldest...
		m_IdFirstCone = (m_IdFirstCone+1) & SR_CONE_TAB_MSK;
	}

	CurCone.Pos = _Pos;
	CurCone.Dir = _Dir;
	CurCone.Angle = DegToRad(_DegDemiAngle);
	CurCone.Dist = _MaxDist;
	CurCone.Frame30FPS = CurFrame30FPS;
}

/**************************************************************************/

void	Playspace_SR_W::SetLagConeView(Float _LagTime)
{
	if (LockCone)
		return;

	Playspace_SR_Status Status = GetRefreshStatus();
	if (Status != Playspace_SR_Status::STATUS_OK)
	{
		if (Status == Playspace_SR_Status::STATUS_FREEZE)
		{
			// Flush Cone manage.
			m_SpeedCheckTime30FPS = 0;
			m_IdFirstCone = m_IdLastCone = 0;
			InvalidateConeView();
		}
		return;
	}

	U32 Lag30Fps = (U32)(_LagTime * 30.f);
	U32 WantedFrame30FPS = (U32)(GetAbsoluteTime() * 30.f) - Lag30Fps;

	InvalidateConeView();
	S32  CurId = m_IdFirstCone;
	while (CurId != m_IdLastCone)
	{
		if (m_TabConeView[CurId].Frame30FPS > WantedFrame30FPS)
			break;
		if (m_TabConeView[CurId].Frame30FPS < (WantedFrame30FPS - Lag30Fps))
		{
			// too old... kick it !
			m_IdFirstCone = (m_IdFirstCone+1) & SR_CONE_TAB_MSK;
		}
		else
		{	
			// Better one... for the moment
			m_CurCone = CurId;
		}

		// Next.
		CurId = (CurId+1) & SR_CONE_TAB_MSK;
	}

	if (m_CurCone < 0)
		return;

	// Prepare Cone;
	Playspace_SR_ConeView &CurCone = m_TabConeView[m_CurCone];
	m_FilterConePos = CurCone.Pos;
	m_FilterConeDir = CurCone.Dir;

	m_FilterConeMax = CurCone.Dist;
	m_FilterConeMax2 = m_FilterConeMax*m_FilterConeMax;
	m_FilterConeAngle = CurCone.Angle;
	Vec2f	ConeSinCos;
	SinCos(ConeSinCos, m_FilterConeAngle);

	m_FilterConeCos2 = ConeSinCos.y * ConeSinCos.y;
	m_FilterConeInvCos = 1.f / ConeSinCos.y;
	m_FilterConeTan = ConeSinCos.x / ConeSinCos.y;
}

/**************************************************************************/

void	Playspace_SR_W::SetSnapValue(Float	_Granularity)
{
	m_SnapGranularity = _Granularity;
}

/**************************************************************************/

void	Playspace_SR_W::SetValidZone(const Vec3f &_Min,const Vec3f &_Max)
{
	if (   ((_Min - m_Min).GetNorm2() < 0.01f)
		&& ((_Max - m_Max).GetNorm2() < 0.01f)
		)
		return;
	m_Min = _Min;
	m_Max = _Max;
	m_BlindMap.LockMinMax(m_Min, m_Max);
}

/**************************************************************************/

void	Playspace_SR_W::RefreshSRFromStaticDatas()
{
	InvalidateConeView();	// Invalidate to prevent re-use.

	// Update Cone View.
	SetLagConeView(0.2f);

	// Already init ?
	if (m_BlindMap.IsInit())
		return;

	// On Device ?
	if ((GetTypeRefresh() == REFRESH_SR_DEVICE))
		return;

	// No SR ?
	if ((GetTypeRefresh() == REFRESH_SR_NONE))
		return;

	// Do Refresh ONCE.
	m_LockThreadRefresh.Lock();
	Segment_Z RefView;
	Util_L::GetViewSegment(&RefView);
	Vec3f	Center = RefView.Org;
	SnapPos(Center);
	m_BlindMap.Init(Center);
	MarkBlindMapFromSR(TRUE, PSR_BlindMap_Voxel::IsBlock);
	m_LockThreadRefresh.Unlock();
}

/**************************************************************************/

void	Playspace_SR_W::RefreshSRFromDevice(Playspace_SR_DeviceSR *_pDeviceSR, Bool _UseFilter)
{
	InvalidateConeView();	// Invalidate to prevent re-use.

	if (m_LastFrameUpdate == GetGlobalFrameCount())
		return;
	if (GetRefreshStatus() != Playspace_SR_Status::STATUS_OK)	// Lock Refresh if any problem is detected.
		return;
	if (!_pDeviceSR)
		return;

	SetLagConeView(0.2f);
	if (_UseFilter && !IsValidConeView())
		return;

	// Lock before Manage.
	if (!m_LockThreadRefresh.TryLock())
	{
		InvalidateConeView();	// Invalidate to prevent use of NON REFRESHED CONE.
		return; // Can't Refresh... use in a thread.
	}

	m_LastFrameUpdate = GetGlobalFrameCount();
	Float t0 = GetAbsoluteTime();

	// Init Blind if needed.
	if (!m_BlindMap.IsInit())
	{
		Segment_Z RefView;
		Util_L::GetViewSegment(&RefView);
		Vec3f	Center = RefView.Org;
		SnapPos(Center);
		m_BlindMap.Init(Center);
	}

	// Touch all for delete.
	S32 NbBlock = m_TabBlocks.GetSize();
	for (S32 i = 0; i<NbBlock ; i++)
		m_TabBlocks[i].m_CounthForDelete++;

	S32 Stats_Total = 0;
	S32 Stats_NbNew = 0;
	S32 Stats_NbReLink = 0;
	S32 Stats_NbDelete = 0;
	S32 Stats_NbRefresh = 0;
	S32 Stats_NbKeep = 0;
//	S32 Stats_NbSeen = 0;
	
	Mat4x4 GlobalTransfo = _pDeviceSR->GlobalTransfo;
	Playspace_SR_BlockHdl	CurBlockHdl;
	for (S32 CurBlockId=0 ; CurBlockId<_pDeviceSR->NbSurfaces ; CurBlockId++)
	{
		Playspace_SR_SurfaceInfo	&CurSurfaceData = _pDeviceSR->TabSurfaces[CurBlockId];

		// Empty ?
		if (!CurSurfaceData.NbVertex)
			continue;
		if (!CurSurfaceData.NbIndex)
			continue;

		// Get Matrix.
		Mat4x4	MtxVtxToWorld = GlobalTransfo * CurSurfaceData.PosToWorldMtx;
		Vec3f	vCenter = MtxVtxToWorld.GetMatrixTrans();

		// Igonre because of dist ?
		if (vCenter.x < (m_Min.x - 2.f))
			continue;
		if (vCenter.x > (m_Max.x + 2.f))
			continue;
		if (vCenter.z < (m_Min.z - 2.f))
			continue;
		if (vCenter.z > (m_Max.z + 2.f))
			continue;

		// Get or construct bloc.
		CurBlockHdl.m_SurfaceHdl = CurSurfaceData.ID;
		Playspace_SR_Block	*pBlock = GetBlock(CurBlockHdl, FALSE);
		Bool	IsNewBloc = FALSE;
		if (!pBlock)
		{
			// Actually Bloc ID Change... BAD ! :(
			//pBlock = GetBlockByCenterPos(vCenter, 0.2f);	// 20cm mvt maxi.		// This was a temporary fix - no longer needed
			if (!pBlock)
			{
				// Create.
				IsNewBloc = TRUE;
				pBlock = GetNewBlock();
				pBlock->m_BlockHdl = CurBlockHdl;
				pBlock->m_HaveBeenSeen = FALSE;
				pBlock->m_NeedBlindRefresh = FALSE;
				pBlock->m_FrameRefresh = 0;
				//OUTPUT_Z("New bloc : %d", CurBlockHdl.m_SurfaceHdl.GetKey());
				Stats_NbNew++;
			}
			else
			{
				// Relink to new ID.
				//OUTPUT_Z("ReLink bloc : %d => %d", pBlock->m_BlockHdl.m_SurfaceHdl.GetKey(), CurBlockHdl.m_SurfaceHdl.GetKey());
				pBlock->m_BlockHdl = CurBlockHdl;
				Stats_NbReLink++;
			}
		}

		// Don't destroy Bloc.
		pBlock->m_CounthForDelete = 0;

		// Refresh it ?s
		if ((!IsNewBloc) && (pBlock->m_FrameRefresh == CurSurfaceData.FrameUpdate))
		{
			// Not Updated Block
			Stats_NbKeep++;
			continue;
		}

		// CRC Filter (because updated block are not always updated :P)
		U32 MyCRC = Name_Z::GetID((U8*)CurSurfaceData.pVertexPos, CurSurfaceData.NbVertex * sizeof(Vec3f));
		if ((!IsNewBloc) && (MyCRC == pBlock->m_CRCRefresh))
		{
			// bloc don't change...
			pBlock->m_FrameRefresh = CurSurfaceData.FrameUpdate;
			Stats_NbKeep++;
			continue;
		}

		// Compute Min-Max and Inside Cone.
		Bool	bInsideCone = TRUE;
		if (_UseFilter)
		{
			// Already exist.
			// Compute Min Max.
			const Vec3f		*pVtxPos = CurSurfaceData.pVertexPos;
			Vec3f	MinBox(pVtxPos[0]);
			Vec3f	MaxBox(MinBox);
			++pVtxPos;

			for (U32 i = 1; i < CurSurfaceData.NbVertex; i++)
			{
				Float x = pVtxPos->x;
				Float y = pVtxPos->y;
				Float z = pVtxPos->z;
				++pVtxPos; 
				if (x < MinBox.x)
					MinBox.x = x;
				else if (x > MaxBox.x)
					MaxBox.x = x;

				if (y < MinBox.y)
					MinBox.y = y;
				else if (y > MaxBox.y)
					MaxBox.y = y;

				if (z < MinBox.z)
					MinBox.z = z;
				else if (z > MaxBox.z)
					MaxBox.z = z;
			}
			// => Refresh ?
			VecFloat4	vMinPos = VecFloat4x4Transform3(MtxVtxToWorld, VecFloatLoad4(MinBox.x, MinBox.y, MinBox.z, 1.0f));
			VecFloat4	vMaxPos = VecFloat4x4Transform3(MtxVtxToWorld, VecFloatLoad4(MaxBox.x, MaxBox.y, MaxBox.z, 1.0f));

			VecFloat4	Center = VecFloatMul(VecFloatAdd(vMaxPos, vMinPos), VectorConstantsPrivate::vHalfF);
			Float R = VecFloatGetX(VecFloatLength3(VecFloatSub(vMaxPos, vMinPos))) * 0.5f;

			// Is in Cone ?
			bInsideCone = SRSphereVsCone(Center, R);
		}

		if (bInsideCone)
		{
			pBlock->m_HaveBeenSeen = TRUE;	// On refresh le bloc tant qu'il n'a pas été vu une fois.
		}
		else if (_UseFilter & pBlock->m_HaveBeenSeen)
		{
			// Don't refresh it this is an old seen block.
			Stats_NbKeep++;
			continue;
		}

		// Update Block.
		pBlock->m_CRCRefresh = MyCRC;
		pBlock->m_Center = vCenter;
		pBlock->m_FrameRefresh = CurSurfaceData.FrameUpdate;

		Stats_NbRefresh++;

		// Get Faces.
		pBlock->m_TabFaceIdx.SetSize(CurSurfaceData.NbIndex,TRUE);
		if (CurSurfaceData.IndexIsClockWise)
		{
			// Invert
			U32 *pDst = pBlock->m_TabFaceIdx.GetArrayPtr();
			U32 *pSrc = (U32*)CurSurfaceData.pIndex;
			for (U32 idx = 0;idx < CurSurfaceData.NbIndex;idx += 3)
			{
				register U32 p2 = *pSrc++;
				register U32 p1 = *pSrc++;
				register U32 p0 = *pSrc++;
				*pDst++ = p0;
				*pDst++ = p1;
				*pDst++ = p2;
			}
		}
		else
		{
			// copy.
			memcpy(pBlock->m_TabFaceIdx.GetArrayPtr(), CurSurfaceData.pIndex, CurSurfaceData.NbIndex * sizeof(U32));
		}

		// Get Vtx.
		pBlock->m_TabVtx.SetSize(CurSurfaceData.NbVertex,TRUE);

		Vec3f		*pVtxPos = CurSurfaceData.pVertexPos;
		Vec3f		*pOutVtx = pBlock->m_TabVtx.GetArrayPtr();
						
		for (U32 i = 0; i < CurSurfaceData.NbVertex; i++)
		{
			VecFloat4	vInPos = VecFloatLoad4(pVtxPos->x, pVtxPos->y, pVtxPos->z,1.0f);
			VecFloat4	vWorldPos = VecFloat4x4Transform3(MtxVtxToWorld, vInPos);
			*pOutVtx++ = vWorldPos;
			++pVtxPos;
		}

		// Add to BlindMap.
		pBlock->m_NeedBlindRefresh = TRUE;

		// PATCH CRASH MICROSOFT.	=> Supress because no more crash...
		U32 NbIdx = pBlock->m_TabFaceIdx.GetSize();
		U32 NbVtx = pBlock->m_TabVtx.GetSize();
		U32	*Ptr = pBlock->m_TabFaceIdx.GetArrayPtr();
		U32	*EndPtr = Ptr+NbIdx;
		while (Ptr < EndPtr)
		{
			if (   (Ptr[0] >= NbVtx)
				|| (Ptr[1] >= NbVtx)
				|| (Ptr[2] >= NbVtx)
				)
			{
				EXCEPTIONC_Z(FALSE,"MICROSOFT BAD VERTEX ON FACE !!!");
				// Move last face to cur.
				EndPtr-=3;
				Ptr[0] = EndPtr[0];
				Ptr[1] = EndPtr[1];
				Ptr[2] = EndPtr[2];

				// Suppress Last Face
				pBlock->m_TabFaceIdx.SetSize(NbIdx-3,TRUE);
				NbIdx-=3;

				// Re-Check cur face.
				continue;
			}
			// Next.
			Ptr+=3;
		}
	}

	// Release non used blocks !!!
	NbBlock = m_TabBlocks.GetSize();
	for (S32 i = 0; i<NbBlock ; i++)
	{
		Playspace_SR_Block	*pBlock = &m_TabBlocks[i];
		if (!pBlock->m_BlockHdl.IsValid())
			continue;
		if (pBlock->m_CounthForDelete < 80)	// Prevent destroy datas to ... fast.
		{
			Stats_Total++;
			continue;
		}

		Stats_NbDelete++;
		// Bloc is not get any more... Release it.
		ReleaseBlock(pBlock);
	}

	m_TypeRefresh = REFRESH_SR_DEVICE;

	Float t1 = GetAbsoluteTime();

	OUTPUT_Z("RefreshFromDevice %.3f", t1 - t0);
	OUTPUT_Z("SR Blocs Total : %d",Stats_Total);
	OUTPUT_Z("		 New     : %d    Del : %d    Relink : %d",Stats_NbNew,Stats_NbDelete, Stats_NbReLink);
	OUTPUT_Z("       Refresh : %d   Keep : %d",Stats_NbRefresh,Stats_NbKeep);

	// Unlock
	m_LockThreadRefresh.Unlock();
}

/**********************************************************/

Bool	Playspace_SR_W::LoadSRMeshRaw(const char *_FileName)
{
	Flush(FALSE);
	m_TypeRefresh = REFRESH_SR_FILE;
	m_LastFrameUpdate = GetGlobalFrameCount();

	FILE* pFile;
	fopen_s(&pFile, _FileName, "rb");
	if (!pFile)
		return FALSE;
	S32 v1, v2, v3;
	if (   (fread(&v1, sizeof(S32), 1, pFile) != 1)
		|| (fread(&v2, sizeof(S32), 1, pFile) != 1)
		|| (fread(&v2, sizeof(S32), 1, pFile) != 1)
		)
	{
		fclose(pFile);
		return FALSE;
	}

	S32		NbPoints, NbVert;
	S32		NumBlock = 0;
	Playspace_SR_BlockHdl	CurBlockHdl;
	while (fread(&NbVert, sizeof(S32), 1, pFile) == 1)
	{
		if (NbVert < 0)
			break;

		// Get Block.
		CurBlockHdl.m_BlocNum = NumBlock;
		Playspace_SR_Block	*pBlock = GetBlock(CurBlockHdl, TRUE);

		// Get Triangles.
		S32 NbTri = (NbVert / 3);
		pBlock->m_TabFaceIdx.SetSize(NbTri * 3, TRUE);
		pBlock->m_HaveBeenSeen = TRUE;
		pBlock->m_NeedBlindRefresh = FALSE;
		pBlock->m_CounthForDelete = 0;
		pBlock->m_Center = VEC3F_NULL;

		if (fread(pBlock->m_TabFaceIdx.GetArrayPtr(), sizeof(S32), NbVert, pFile) != NbVert)
		{
			fclose(pFile);
			return FALSE;
		}

		// Get Vertex.
		if (fread(&NbPoints, sizeof(S32), 1, pFile) != 1)
		{
			fclose(pFile);
			return FALSE;
		}
		pBlock->m_TabVtx.SetSize(NbPoints, TRUE);
		if (fread(pBlock->m_TabVtx.GetArrayPtr(), sizeof(Vec3f), NbPoints, pFile) != NbPoints)
		{
			fclose(pFile);
			return FALSE;
		}

		NumBlock++;
	}
	fclose(pFile);
	return TRUE;
}

/**********************************************************/

Bool	Playspace_SR_W::SetSRMesh(U32 *_pTabTriIdx, S32 _NbTriIdx, Vec3f *_pTabVtx, S32 _NbVtx)
{
	Flush(FALSE);
	m_TypeRefresh = REFRESH_SR_FILE;
	m_LastFrameUpdate = GetGlobalFrameCount();

	if (_NbTriIdx <= 0)
		return FALSE;
	if (_NbVtx <= 0)
		return FALSE;

	S32		NbPoints, NbVert;
	S32		NumBlock = 0;
	Playspace_SR_BlockHdl	CurBlockHdl;

	// Get Block.
	CurBlockHdl.m_BlocNum = NumBlock;
	Playspace_SR_Block	*pBlock = GetBlock(CurBlockHdl, TRUE);

	// Get Triangles.
	pBlock->m_TabFaceIdx.SetSize(_NbTriIdx * 3, TRUE);
	pBlock->m_HaveBeenSeen = TRUE;
	pBlock->m_NeedBlindRefresh = FALSE;
	pBlock->m_CounthForDelete = 0;
	pBlock->m_Center = VEC3F_NULL;

	memcpy(pBlock->m_TabFaceIdx.GetArrayPtr(), _pTabTriIdx, sizeof(U32) * _NbTriIdx * 3);

	// Get Vertex.
	pBlock->m_TabVtx.SetSize(_NbVtx, TRUE);
	memcpy(pBlock->m_TabVtx.GetArrayPtr(), _pTabVtx, sizeof(Vec3f) * _NbVtx);

	return TRUE;
}

/**********************************************************/

Bool	Playspace_SR_W::SetSRMesh(Dll_Interface::MeshData* _TabMesh, S32 _NbMesh, const Quat &_MeshToWorld)
{
	Flush(FALSE);
	m_TypeRefresh = REFRESH_SR_FILE;
	m_LastFrameUpdate = GetGlobalFrameCount();

	Mat4x4	meshToWorld;
	_MeshToWorld.GetMatrix(meshToWorld);

	S32		NumBlock = 0;
	Playspace_SR_BlockHdl	CurBlockHdl;
	for (S32 i=0 ; i<_NbMesh ; i++)
	{
		Dll_Interface::MeshData	&CurMesh = _TabMesh[i];
		if (CurMesh.vertCount <= 0)
			continue;
		if (CurMesh.indexCount <= 0)
			continue;

		// Get Block.
		CurBlockHdl.m_BlocNum = NumBlock;
		Playspace_SR_Block	*pBlock = GetBlock(CurBlockHdl, TRUE);

		// Get Triangles.
		S32 NbTri = (CurMesh.indexCount / 3);
		pBlock->m_TabFaceIdx.SetSize(NbTri * 3, TRUE);
		pBlock->m_HaveBeenSeen = TRUE;
		pBlock->m_NeedBlindRefresh = FALSE;
		pBlock->m_CounthForDelete = 0;
		pBlock->m_Center = VEC3F_NULL;

		memcpy(pBlock->m_TabFaceIdx.GetArrayPtr(), CurMesh.indices, sizeof(U32) * NbTri * 3);

		// Get Vertex.
		Mat4x4 meshTransform = Util_L::Convert_XMAT4x4_to_Mat4x4(CurMesh.transform) * meshToWorld;
		pBlock->m_TabVtx.SetSize(CurMesh.vertCount, TRUE);
		for (int v = 0; v < CurMesh.vertCount; ++v)
		{
			DirectX::XMFLOAT3& srcXMF = CurMesh.verts[v];
			Vec4f srcVec4 = Vec4f(srcXMF.x, srcXMF.y, srcXMF.z, 1.0f);
			srcVec4 = VecFloat4x4Transform3(meshTransform, srcVec4);
			pBlock->m_TabVtx[v] = srcVec4;
		}

		NumBlock++;
	}

	return TRUE;
}

/**********************************************************/

void	Playspace_SR_W::FlushBlind()
{
	m_BlindMap.Flush();
	S32 NbBlock = m_TabBlocks.GetSize();
	for (S32 i = 0; i<NbBlock; i++)
	{
		Playspace_SR_Block	&CurBlock = m_TabBlocks[i];
		CurBlock.m_HaveBeenSeen = FALSE;
		CurBlock.m_NeedBlindRefresh = TRUE;
	}
}

/**********************************************************/

void	Playspace_SR_W::RefreshZBuffer(HU8DA &_ZBuffer,const Vec3f &_Pos, const Vec3f &_Dir, Float _Angle, Float _MaxDist)
{
	m_BlindMap.ResetZBuffer(_ZBuffer);

	Float t0 = GetAbsoluteTime();

	RSTimeT1 = RSTimeT2 = 0.f;

	S32 NbBlock = m_TabBlocks.GetSize();
	for (S32 i = 0; i < NbBlock; i++)
	{
		Playspace_SR_Block	&CurBlock = m_TabBlocks[i];
		if (!CurBlock.m_BlockHdl.IsValid())
			continue;
		S32 NbVertex = CurBlock.m_TabVtx.GetSize();
		if (!NbVertex)
			continue;

		// Ici, on gagnerait un peu de temps en faisant une exclusion avec la sphère englobante.
		// => Mais sphère pas stockée... donc pas pour le moment.
		m_BlindMap.AddMeshToZBuffer(_ZBuffer,CurBlock.m_TabFaceIdx.GetArrayPtr(),CurBlock.m_TabFaceIdx.GetSize()/3,3*sizeof(U32),CurBlock.m_TabVtx.GetArrayPtr(), NbVertex,_Pos, _Dir,_Angle, _MaxDist);
	}

	Float t1 = GetAbsoluteTime();
	MESSAGE_Z("Rasterize Mesh %.3f - %.3f %.3f",t1-t0,RSTimeT1,RSTimeT2);
}

/**********************************************************/

void	Playspace_SR_W::RefreshBlindAndModeFromZBuffer(Playspace_SR_BlindMap::Mode _Mode,const Vec3f &_Pos, const Vec3f &_Dir,const Vec3f &_LateralDir, Float _HalfAngle, Float _MaxDist)
{
	if (!IsValidConeView())
		return;

	Playspace_SR_ConeView ConeBlind;
	ConeBlind.Pos = m_FilterConePos;
	ConeBlind.Dir = m_FilterConeDir;
	ConeBlind.Angle = m_FilterConeAngle;
	ConeBlind.Dist = m_FilterConeMax; // Not Used.

	Playspace_SR_ConeView ConeMode;
	ConeMode.Pos = _Pos;
	ConeMode.Dir = _Dir;
	ConeMode.Angle = _HalfAngle;
	ConeMode.Dist = _MaxDist; // Not Used.

	m_BlindMap.ProcessConeViewNew(_Mode,ConeBlind,ConeMode,_LateralDir);
}

/**********************************************************/

void	Playspace_SR_W::RefreshZBufferFromConeView()
{
	if (!IsValidConeView())
	{
		m_BlindMap.m_LastZBuffer.SetSize(0,TRUE);
		return;
	}
	RefreshZBuffer(m_BlindMap.m_LastZBuffer,m_FilterConePos, m_FilterConeDir, m_FilterConeAngle, m_FilterConeMax);
}

/**********************************************************/

void	Playspace_SR_W::RefreshBlindFromConeView()
{
	if (!IsValidConeView())
		return;
	m_BlindMap.ProcessConeView(Playspace_SR_BlindMap::Mode_Blind,m_FilterConePos, m_FilterConeDir,VEC3F_NULL, m_FilterConeAngle, m_FilterConeMax);
}

/**********************************************************/
void	Playspace_SR_W::RefreshPaint(const Vec3f &_Pos, const Vec3f &_Dir,const Vec3f &_LateralDir, Float _DegDemiAngle, Float _MaxDist)
{
	m_BlindMap.ProcessConeView(Playspace_SR_BlindMap::Mode_Paint,_Pos, _Dir,_LateralDir, DegToRad(_DegDemiAngle), _MaxDist);
}

/**********************************************************/
void	Playspace_SR_W::RefreshClear(const Vec3f &_Pos, const Vec3f &_Dir,const Vec3f &_LateralDir, Float _DegDemiAngle, Float _MaxDist)
{
	m_BlindMap.ProcessConeView(Playspace_SR_BlindMap::Mode_Clear,_Pos, _Dir,_LateralDir, DegToRad(_DegDemiAngle), _MaxDist);
}

/**********************************************************/

void	Playspace_SR_W::ThreadSafe_ExtractSRToMesh(Playspace_Mesh &_Mesh, Bool _FilterAndMarkFaces)
{
	if (!_FilterAndMarkFaces)
	{
		m_LockThreadRefresh.Lock();	// Bloc en attendant fin du refresh...

		S32 NbBlock = m_TabBlocks.GetSize();
		for (S32 i = 0; i<NbBlock; i++)
		{
			Playspace_SR_Block	&CurBlock = m_TabBlocks[i];
			if (!CurBlock.m_BlockHdl.IsValid())
				continue;
			_Mesh.AddMesh(CurBlock.m_TabFaceIdx.GetArrayPtr(), CurBlock.m_TabFaceIdx.GetSize(), CurBlock.m_TabVtx.GetArrayPtr(), CurBlock.m_TabVtx.GetSize(), FALSE);
		}

		m_LockThreadRefresh.Unlock();
		return;
	}

	if (!m_BlindMap.IsInit())
		return;

	m_LockThreadRefresh.Lock();	// Bloc en attendant fin du refresh...

	// Get with filter.
	PSR_BlindMap_VoxelHUDA	TabVoxel;
	HugeDynArray_Z<Playspace_Mesh::FlagFace, 16, FALSE, FALSE>	TabFlagFaces;

	S32 NbBlock = m_TabBlocks.GetSize();
	for (S32 i = 0; i<NbBlock; i++)
	{
		Playspace_SR_Block	&CurBlock = m_TabBlocks[i];
		if (!CurBlock.m_BlockHdl.IsValid())
			continue;

		S32 NbPoints = CurBlock.m_TabVtx.GetSize();
		S32 NbFaces = CurBlock.m_TabFaceIdx.GetSize() / 3;

		// Get points infos.
		m_BlindMap.GetPointInfos(CurBlock.m_TabVtx.GetArrayPtr(), NbPoints, TabVoxel);
		TabFlagFaces.SetSize(NbFaces, TRUE);
		TabFlagFaces.Null();

		// Get Faces infos.
		U32	*pFacesId = CurBlock.m_TabFaceIdx.GetArrayPtr();
		for (S32 i = 0; i<NbFaces; i++)
		{
			
			U32 VtxId0 = *pFacesId++;
			U32 VtxId1 = *pFacesId++;
			U32 VtxId2 = *pFacesId++;

			U8 Voxel0 = TabVoxel[VtxId0].Flags;
			U8 Voxel1 = TabVoxel[VtxId1].Flags;
			U8 Voxel2 = TabVoxel[VtxId2].Flags;

			U8 AndVoxel = Voxel0 & Voxel1 & Voxel2;
			U8 OrVoxel = Voxel0 | Voxel1 | Voxel2;

			TabFlagFaces[i].IsTri = 1;
			
			if (!(AndVoxel & PSR_BlindMap_Voxel::IsOut))
			{
				TabFlagFaces[i].IsMarked = 1;	// Keep this face.
				if (OrVoxel & PSR_BlindMap_Voxel::IsPaint)
					TabFlagFaces[i].IsPaintMode = 2;
				else if (OrVoxel & PSR_BlindMap_Voxel::IsUnPaint)
					TabFlagFaces[i].IsPaintMode = 1;

				if (OrVoxel & PSR_BlindMap_Voxel::IsSeen)
				{
					U8	CountMax = Max(Max(TabVoxel[VtxId0].SeenCount,TabVoxel[VtxId1].SeenCount),TabVoxel[VtxId2].SeenCount);

					if (!CountMax)
						TabFlagFaces[i].IsSeenQuality = 0;
					else if (CountMax < QUALITY_BLIND_1)
						TabFlagFaces[i].IsSeenQuality = 1;
					else if (CountMax < QUALITY_BLIND_2)
						TabFlagFaces[i].IsSeenQuality = 2;
					else
						TabFlagFaces[i].IsSeenQuality = 3;
				}
			}
		}

		_Mesh.AddMesh(CurBlock.m_TabFaceIdx.GetArrayPtr(), CurBlock.m_TabFaceIdx.GetSize(), CurBlock.m_TabVtx.GetArrayPtr(), CurBlock.m_TabVtx.GetSize(), FALSE, TabFlagFaces.GetArrayPtr());
	}

	m_LockThreadRefresh.Unlock();
}

/**********************************************************/

void	Playspace_SR_W::RefreshBlindMapFromDevice(Bool _ForceTotal,Bool _ClearFisrt)
{
	m_LockThreadBlind.Lock();
	if (_ClearFisrt)
	{
		// if total, clear first !
		m_BlindMap.CleatFlag(PSR_BlindMap_Voxel::IsBlock);
	}
	MarkBlindMapFromSR(_ForceTotal,PSR_BlindMap_Voxel::IsBlock);
	m_LockThreadBlind.Unlock();
}

/**********************************************************/

void	Playspace_SR_W::ThreadSafe_TotalRefreshBlindMapFromMesh(Playspace_Mesh &_Mesh)	//=> On ne l'utilise pas à cause de la récupération du paint par le Mesh.
{
	m_LockThreadRefresh.Lock();	// Bloc en attendant fin du refresh...

	// Push To BlindMap...
	m_BlindMap.AddMesh(_Mesh,PSR_BlindMap_Voxel::IsWork);
	m_BlindMap.AddMeshEnd();
	m_BlindMap.ConvertWorkFlag(PSR_BlindMap_Voxel::IsBlock);

	m_LockThreadRefresh.Unlock();
}

/**********************************************************/

void	Playspace_SR_W::ThreadSafe_TotalRefreshBlindMapFromSR()
{
	m_LockThreadRefresh.Lock();	// Bloc en attendant fin du refresh...

	MarkBlindMapFromSR(TRUE,PSR_BlindMap_Voxel::IsWork);
	m_BlindMap.ConvertWorkFlag(PSR_BlindMap_Voxel::IsBlock);

	m_LockThreadRefresh.Unlock();
}

/**********************************************************/

void	Playspace_SR_W::SuppressUnpaintBorder(Playspace_Mesh &_Mesh)
{
	S32 Size = m_BlindMap.m_BlindMap.GetSize();
	if (!Size)
		return;

	S32		NbFaces = _Mesh.m_TabQuad.GetSize();
	Vec3f	MeshMin(1e8f,1e8f,1e8f);
	Vec3f	MeshMax(-1e8f,-1e8f,-1e8f);

	for (S32 i = 0; i<NbFaces ; i++)
	{
		Playspace_Mesh::Face &CurFace = _Mesh.m_TabQuad[i];
		if (CurFace.IsPaintMode != 2)	// Keep Only paint faces.
			continue;
		Vec3f &Pos0 = _Mesh.m_TabPoints[CurFace.TabPoints[0]];
		MeshMin = Min(MeshMin, Pos0);
		MeshMax = Max(MeshMax, Pos0);
		Vec3f &Pos1 = _Mesh.m_TabPoints[CurFace.TabPoints[1]];
		MeshMin = Min(MeshMin, Pos1);
		MeshMax = Max(MeshMax, Pos1);
		Vec3f &Pos2 = _Mesh.m_TabPoints[CurFace.TabPoints[2]];
		MeshMin = Min(MeshMin, Pos2);
		MeshMax = Max(MeshMax, Pos2);
	}

	Vec3i	vMin(0,0,0);

	S32 x = (S32)((MeshMin.x - m_BlindMap.m_fOrigin.x) * SR_BLINDRAY_MAP_TO_CELL);
	S32 z = (S32)((MeshMin.z - m_BlindMap.m_fOrigin.z) * SR_BLINDRAY_MAP_TO_CELL);
	S32 y = (S32)((MeshMin.y - m_BlindMap.m_fOrigin.y) * SR_BLINDRAY_MAP_TO_CELL);

	vMin.x = Max(vMin.x,x);
	vMin.y = Max(vMin.y,y);
	vMin.z = Max(vMin.z,z);

	Vec3i	vMax(SR_BLINDRAY_MAP_SIZE,SR_BLINDRAY_MAP_SIZE,SR_BLINDRAY_MAP_SIZE);
	x = (S32)((MeshMax.x - m_BlindMap.m_fOrigin.x) * SR_BLINDRAY_MAP_TO_CELL);
	z = (S32)((MeshMax.z - m_BlindMap.m_fOrigin.z) * SR_BLINDRAY_MAP_TO_CELL);
	y = (S32)((MeshMax.y - m_BlindMap.m_fOrigin.y) * SR_BLINDRAY_MAP_TO_CELL);

	vMax.x = Min(vMax.x,x);
	vMax.y = Min(vMax.y,y);
	vMax.z = Min(vMax.z,z);

	if (vMin.x > vMax.x)
		return;
	if (vMin.y > vMax.y)
		return;
	if (vMin.z > vMax.z)
		return;

	// Clear Unpaint.
	PSR_BlindMap_Voxel *pVoxel = m_BlindMap.m_BlindMap.GetArrayPtr();
	S32 DeltaY = vMax.y-vMin.y+1;
	for (S32 z = 0; z<SR_BLINDRAY_MAP_SIZE; z++)
	{
		Bool IsOkZ = (z<vMin.z) || (z>vMax.z);
		for (S32 x = 0; x<SR_BLINDRAY_MAP_SIZE; x++)
		{
			Bool IsOkX = (x<vMin.x) || (x>vMax.x);

			if (IsOkX || IsOkZ)
			{
				for (S32 y = 0; y<SR_BLINDRAY_MAP_SIZE ; y++,pVoxel++)
				{
					if (pVoxel->Flags & (PSR_BlindMap_Voxel::IsUnPaint))
						pVoxel->Flags &= 0xFF^PSR_BlindMap_Voxel::IsUnPaint;
				}
			}
			else
			{
				for (S32 y = 0; y<vMin.y ; y++,pVoxel++)
				{
					if (pVoxel->Flags & (PSR_BlindMap_Voxel::IsUnPaint))
						pVoxel->Flags &= 0xFF^PSR_BlindMap_Voxel::IsUnPaint;
				}
				pVoxel+= DeltaY;
				for (S32 y = vMax.y+1 ; y<SR_BLINDRAY_MAP_SIZE ; y++,pVoxel++)
				{
					if (pVoxel->Flags & (PSR_BlindMap_Voxel::IsUnPaint))
						pVoxel->Flags &= 0xFF^PSR_BlindMap_Voxel::IsUnPaint;
				}
			}
		}
	}
	EXCEPTIONC_Z(pVoxel == (m_BlindMap.m_BlindMap.GetArrayPtr() + (SR_BLINDRAY_MAP_SIZE*SR_BLINDRAY_MAP_SIZE*SR_BLINDRAY_MAP_SIZE)),"BAD SuppressUnpaintBorder");
}

/**********************************************************/

void	Playspace_SR_W::ApplyQuat(const Quat &_Transfo,Bool _FullReInsertMesh)
{
	// Recompute Min Max (Y don't change).
	Vec3f Center = (m_Max + m_Min) * 0.5f;
	Float Dist = Center.x - m_Min.x;

	Center = _Transfo * Center;
	m_Min.x = Center.x - Dist;
	m_Min.z = Center.z - Dist;

	m_Max.x = Center.x + Dist;
	m_Max.z = Center.z + Dist;

	// Apply to BlindMap.
	m_BlindMap.ApplyQuatAndMinMax(_Transfo, m_Min, m_Max);

	// Apply Quat and Mark blocs...
	S32 NbBlock = m_TabBlocks.GetSize();
	//Float t0 = GetAbsoluteTime();

	Mat4x4	MatTransfo;
	_Transfo.GetMatrix(MatTransfo);

	for (S32 i = 0; i<NbBlock; i++)
	{
		Playspace_SR_Block	&CurBlock = m_TabBlocks[i];
		if (!CurBlock.m_BlockHdl.IsValid())
			continue;

		Vec4f pc(CurBlock.m_Center);
		CurBlock.m_Center = VecFloat4x4Transform3(MatTransfo,pc);

		S32 NbVertex = CurBlock.m_TabVtx.GetSize();
		for (S32 j = 0; j < NbVertex; j++)
		{
			Vec4f v(CurBlock.m_TabVtx[j]);
			CurBlock.m_TabVtx[j] = VecFloat4x4Transform3(MatTransfo,v);
		}

		// Add to BlindMap.
//		m_BlindMap.AddPointList(CurBlock.m_TabVtx.GetArrayPtr(), CurBlock.m_TabVtx.GetSize());
		CurBlock.m_NeedBlindRefresh = TRUE;
		if (_FullReInsertMesh)
		{
			m_BlindMap.AddMesh(CurBlock.m_TabFaceIdx.GetArrayPtr(),CurBlock.m_TabFaceIdx.GetSize()/3,3*sizeof(U32),CurBlock.m_TabVtx.GetArrayPtr(), NbVertex);
			CurBlock.m_NeedBlindRefresh = FALSE;
		}
	}
	m_BlindMap.AddMeshEnd();

	//Float t1 = GetAbsoluteTime();
	//MESSAGE_Z("AddMesh : %.3f", t1 - t0);
}

/**********************************************************/

void	Playspace_SR_W::MoveIfNeeded(Bool _FullReInsertMesh)
{
	Vec3f vCenter = m_BlindMap.m_fCenter;
	Float vDeltaCenter = (Float)(SR_BLINDRAY_MAP_SIZE >> 1) * SR_BLINDRAY_MAP_TO_WORLD;
	Vec3f vMin = vCenter - Vec3f(vDeltaCenter,vDeltaCenter,vDeltaCenter);
	Vec3f vMax = vCenter + Vec3f(vDeltaCenter,vDeltaCenter,vDeltaCenter);

	if (   (m_Min.x > vMin.x) && (m_Min.z > vMin.z)
		&& (m_Max.x < vMax.x) && (m_Max.z < vMax.z)
		)
		return;

	vCenter = (m_Min + m_Max) * 0.5f;
	SnapPos(vCenter);

	if (!m_BlindMap.ApplyMoveXZAndMinMax(vCenter,m_Min, m_Max))
		return;

	OUTPUT_Z("Playspace_SR_W MOVE Blindmap");
	if (_FullReInsertMesh)
		MarkBlindMapFromSR(TRUE,PSR_BlindMap_Voxel::IsBlock);
}

/**********************************************************/

S32		Playspace_SR_W::GetNextBlock(S32 _NumBlock)
{
	if (_NumBlock < 0)
		_NumBlock = -1;

	S32 NbBlock = m_TabBlocks.GetSize();
	_NumBlock++;
	while (_NumBlock < NbBlock)
	{
		Playspace_SR_Block	&CurBlock = m_TabBlocks[_NumBlock];
		if (CurBlock.m_BlockHdl.IsValid())
			return _NumBlock;
		_NumBlock++;
	}

	return -1;
}

/**********************************************************/

