// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include <pch.h>
#include <Math_Z.h>
#include <Eigen_Z.h>
#include <PlaySpace\PlaySpace_Mesh_W.h>
#include <GenericQSort_Z.h>

/**************************************************************************/
/* PointsLinks                                                                   */
/**************************************************************************/

FINLINE_Z Playspace_Mesh::PointsLinks& Playspace_Mesh::PointsLinks::operator = (const Playspace_Mesh::PointsLinks&_other)
{
	NbFaces = _other.NbFaces;
	SetSize(NbFaces);

	S32 nb = Min(NbFaces,NbStaticFaces);
	for (S32 i=0 ; i<nb ; i++)
		TabFacesS[i] = _other.TabFacesS[i];
	nb = NbFaces-NbStaticFaces;
	for (S32 i=0 ; i<nb ; i++)
	{
		TabFacesD[i] = _other.TabFacesD[i];
	}

	return *this;
}

/**************************************************************************/

FINLINE_Z void	Playspace_Mesh::PointsLinks::Add(const Playspace_Mesh::PointsLinks&_other)
{
	S32 nb = Min(_other.NbFaces,NbStaticFaces);
	for (S32 i=0 ; i<nb ; i++)
		AddFace(_other.TabFacesS[i]);
	nb = _other.NbFaces-NbStaticFaces;
	for (S32 i=0 ; i<nb ; i++)
		AddFace(_other.TabFacesD[i]);
	return;
}

/**************************************************************************/

FINLINE_Z void Playspace_Mesh::PointsLinks::SetSize(S32 _Size)
{
	NbFaces = _Size;
	if (_Size > NbStaticFaces)
	{
		_Size -= NbStaticFaces;
		if (NbReserve < _Size)
		{
			do {
				NbReserve += NbStaticFaces;
			} while (NbReserve < _Size);
			// Alloc.
			if (!TabFacesD)
				TabFacesD =	(S32 *)Alloc_Z(NbReserve*sizeof(S32));
			else
				TabFacesD = (S32 *)Realloc_Z(TabFacesD,NbReserve*sizeof(S32));
		}
	}
}

/**************************************************************************/

FINLINE_Z Bool	Playspace_Mesh::PointsLinks::RemoveFace(S32 _num)
{
	S32 nb = Min(NbStaticFaces,NbFaces);
	for (S32 i=0 ; i<nb ; i++)
	{
		if (TabFacesS[i] == _num)
		{
			for (i = i+1 ; i<nb ; i++)
				TabFacesS[i-1] = TabFacesS[i];

			if (nb < NbFaces)
			{
				TabFacesS[NbStaticFaces-1] = TabFacesD[0];
				nb = NbFaces-NbStaticFaces;
				for (i=1; i<nb ; i++)
				{
					TabFacesD[i-1] = TabFacesD[i];
				}
			} 
			NbFaces--;
			return TRUE;
		}
	}
	nb = NbFaces-NbStaticFaces;
	for (S32 i=0 ; i<nb ; i++)
	{
		if (TabFacesD[i] == _num)
		{
			for (i = i+1 ; i<nb ; i++)
			{
				TabFacesD[i - 1] = TabFacesD[i];
			}
			NbFaces--;
			return TRUE;
		}
	}
	return FALSE;	// not found
}

/**************************************************************************/

FINLINE_Z void	Playspace_Mesh::PointsLinks::AddFace(S32 _num)
{
	// First : Fast end insertion.
	if (!NbFaces || (GetFace(NbFaces - 1) < _num))
	{
		S32 id = NbFaces;
		SetSize(NbFaces+1);
		if (id < NbStaticFaces)
			TabFacesS[id] = _num;
		else
			TabFacesD[id-NbStaticFaces] = _num;
		return;	// End insert.
	}

	// Search insertion point (faster because of end pre-test).
	S32 nb = Min(NbStaticFaces,NbFaces);
	for (S32 i=0 ; i<nb ; i++)
	{
		if (TabFacesS[i] == _num)
			return; // Already exist.
		else if (TabFacesS[i] > _num)
		{
			// Insert.
			SetSize(NbFaces+1);
			nb = NbFaces-NbStaticFaces-1;
			if (nb >= 0)
			{
				while (nb > 0)
				{
					TabFacesD[nb] = TabFacesD[nb-1];
					nb--;
				}
				TabFacesD[0] = TabFacesS[NbStaticFaces-1];
				for (S32 j = NbStaticFaces-1 ; j>i ; j--)
					TabFacesS[j] = TabFacesS[j-1];
			}
			else
			{
				for (S32 j = NbFaces-1 ; j>i ; j--)
					TabFacesS[j] = TabFacesS[j-1];
			}
			TabFacesS[i] = _num;
			return;
		}
	}

	nb = NbFaces-NbStaticFaces;
	for (S32 i=0 ; i<nb ; i++)
	{
		if (TabFacesD[i] == _num)
			return; // Already exist.
		else if (TabFacesD[i] > _num)
		{
			// Insert.
			SetSize(NbFaces+1);
			for (S32 j = nb ; j>i ; j--)
			{
				TabFacesD[j] = TabFacesD[j-1];
			}
			TabFacesD[i] = _num;
			return;
		}
	}
}

/**************************************************************************/

FINLINE_Z void	Playspace_Mesh::PointsLinks::Init(S32 *_TabValue,S32 _nb)
{
	SetSize(_nb);

	S32 nb = Min(NbStaticFaces,NbFaces);
	for (S32 i=0 ; i<nb ; i++)
		TabFacesS[i] = *_TabValue++;

	nb = NbFaces-NbStaticFaces;
	for (S32 i=0 ; i<nb ; i++)
	{
		TabFacesD[i] = *_TabValue++;
	}
}

/**************************************************************************/

FINLINE_Z S32		Playspace_Mesh::PointsLinks::CopyTo(S32 *_TabValue,S32 _nbMax)
{
	_nbMax = Min(_nbMax,NbFaces);
	S32 nb = Min(NbStaticFaces,_nbMax);
	for (S32 i=0 ; i<nb ; i++)
		*_TabValue++ = TabFacesS[i];

	nb = _nbMax-NbStaticFaces;
	for (S32 i=0 ; i<nb ; i++)
	{
		*_TabValue++ = TabFacesD[i];
	}

	return _nbMax;
}
/**************************************************************************/

S32		Playspace_Mesh::PointsLinks::MergeTo(S32 *_TabValue,S32 _nb,S32 _nbMax)
{
	EXCEPTIONC_Z(_nb+NbFaces <= _nbMax,"Playspace_Mesh::PointsLinks::MergeTo SizeError");

	if (!NbFaces)
		return _nb;
	if (!_nb)
		return CopyTo(_TabValue,_nbMax);

	//	Move value => in place merge.
	S32 *pMoveSrc = _TabValue+_nb-1;
	S32 *pMoveDst = _TabValue+_nbMax-1;
	for (S32 i=0 ; i<_nb ; i++)
		*pMoveDst-- = *pMoveSrc--;

	S32 *TabWork = _TabValue+_nbMax-_nb;

	S32 f0 = 0;
	S32 f1 = 0;

	S32 Curf0 = TabWork[f0];
	S32 Curf1 = GetFace(f1);

	S32 NbResult = 0;
	for(;;)
	{
		if (Curf0 == Curf1)
		{
			_TabValue[NbResult++] = Curf0;
			f0++;f1++;
			if ((f0 >= _nb) || (f1 >= NbFaces))
				break;
			Curf0 = TabWork[f0];
			Curf1 = GetFace(f1);
		}
		else if  (Curf0 < Curf1)
		{
			_TabValue[NbResult++] = Curf0;
			f0++;
			if (f0 >= _nb) 
				break;
			Curf0 = TabWork[f0];
		}
		else
		{
			_TabValue[NbResult++] = Curf1;
			f1++;
			if (f1 >= NbFaces) 
				break;
			Curf1 = GetFace(f1);
		}
	}
	while (f0 < _nb)
		_TabValue[NbResult++] = TabWork[f0++];
	while (f1 < NbFaces)
		_TabValue[NbResult++] = GetFace(f1++);

	return NbResult;
}

/**************************************************************************/
/* MESH                                                                   */
/**************************************************************************/

#define MESH_QUADTREE_SNAP	0.005f	// 5 mm de snap...

#define MESH_QUADTREE_CELLSIZE		0.09f // Ne doit pas avoir d'alignement avec le Voxel Space !
#define MESH_QUADTREE_ICELLSIZE		(1.f/MESH_QUADTREE_CELLSIZE)
#define MESH_QUADTREE_TOTAL_SHIFT	15
#define MESH_QUADTREE_TOTAL_SIZE	(1<<MESH_QUADTREE_TOTAL_SHIFT)
#define MESH_QUADTREE_TOTAL_MASK	(MESH_QUADTREE_TOTAL_SIZE - 1)

#define	POS_MESH_QUADTREE(ix,iy,iz) ((ix + (iz << 5) + (iy << 10)) & MESH_QUADTREE_TOTAL_MASK)

Playspace_Mesh::Playspace_Mesh()
{
	m_IsInEditMode = FALSE;
	InvalidatePointsLinks();
	InvalidateFaceToolNormal();
	InvalidatePointsToolNormal();
	m_pFreeTabPoint = NULL;
}

/**************************************************************************/

Playspace_Mesh::Playspace_Mesh(const Playspace_Mesh&_other)
{
	*this = _other;
}

/**************************************************************************/

Playspace_Mesh& Playspace_Mesh::operator = (const Playspace_Mesh&_other)
{
	CopyFrom(_other);
	return *this;
}

/**************************************************************************/
void Playspace_Mesh::CopyFrom(const Playspace_Mesh&_other, Bool _keepMemory, Bool _vertsAndIndicesOnly)
{
	Flush(_keepMemory);

	m_IsInEditMode = _other.m_IsInEditMode && !_vertsAndIndicesOnly;
	m_HavePointLinks = _other.m_HavePointLinks && !_vertsAndIndicesOnly;
	m_HavePointsToolNormal = _other.m_HavePointsToolNormal && !_vertsAndIndicesOnly;
	m_HaveFaceToolNormal = _other.m_HaveFaceToolNormal && !_vertsAndIndicesOnly;

	if (m_IsInEditMode)
	{
		// Recompute QuadTree.
		m_pFreeTabPoint = NULL;
		m_QuadTreePt.SetSize(MESH_QUADTREE_TOTAL_SIZE, TRUE);
		memset(m_QuadTreePt.GetArrayPtr(), 0, MESH_QUADTREE_TOTAL_SIZE*sizeof(Playspace_Mesh::RefPoint*));

		for (S32 i = 0; i<m_TabPoints.GetSize(); i++)
			InsertQuadTreePoint(i, m_TabPoints[i]);
	}

	if (m_HavePointLinks)
	{
		m_TabPointsLinks = _other.m_TabPointsLinks;
	}
	if (m_HavePointsToolNormal)
	{
		m_TabPointsToolNormal = _other.m_TabPointsToolNormal;
	}
	if (m_HaveFaceToolNormal)
	{
		m_TabFaceToolNormal = _other.m_TabFaceToolNormal;
	}

	// Copy Mesh.
	if (!_vertsAndIndicesOnly)
	{
		m_TabPoints = _other.m_TabPoints;
		m_TabQuad = _other.m_TabQuad;
	}
	else
	{
		// Use optimized copy
		m_TabPoints.SetSize(_other.m_TabPoints.GetSize(), TRUE);
		memcpy(m_TabPoints.GetArrayPtr(), _other.m_TabPoints.GetArrayPtr(), m_TabPoints.GetSize() * m_TabPoints.GetEleSize());
		m_TabQuad.SetSize(_other.m_TabQuad.GetSize(), TRUE);
		memcpy(m_TabQuad.GetArrayPtr(), _other.m_TabQuad.GetArrayPtr(), m_TabQuad.GetSize() * m_TabQuad.GetEleSize());
	}
}

/**************************************************************************/

Float	Playspace_Mesh::GetSnapDist()
{
	return MESH_QUADTREE_SNAP;
}

/**************************************************************************/

void	Playspace_Mesh::Swap(Playspace_Mesh &_Other)
{
	::Swap(m_pFreeTabPoint, _Other.m_pFreeTabPoint);
	m_TabRefPoint.Swap(_Other.m_TabRefPoint);
	m_QuadTreePt.Swap(_Other.m_QuadTreePt);

	::Swap(m_IsInEditMode, _Other.m_IsInEditMode);
	::Swap(m_HavePointLinks, _Other.m_HavePointLinks);
	::Swap(m_HavePointsToolNormal, _Other.m_HavePointsToolNormal);
	::Swap(m_HaveFaceToolNormal, _Other.m_HaveFaceToolNormal);
	

	m_TabPoints.Swap(_Other.m_TabPoints);
	m_TabPointsLinks.Swap(_Other.m_TabPointsLinks);
	m_TabPointsToolNormal.Swap(_Other.m_TabPointsToolNormal);
	m_TabFaceToolNormal.Swap(_Other.m_TabFaceToolNormal);
	m_TabQuad.Swap(_Other.m_TabQuad);
}

/**************************************************************************/

void	Playspace_Mesh::Flush(Bool _KeepMemory)
{
	InvalidatePointsLinks();
	InvalidatePointsToolNormal();
	InvalidateFaceToolNormal();
	if (!_KeepMemory)
	{
		m_TabRefPoint.Flush();
		m_pFreeTabPoint = NULL;

		m_QuadTreePt.Flush();
		m_TabPoints.Flush();
		m_TabQuad.Flush();
		m_TabPointsLinks.Flush();
		m_TabPointsToolNormal.Flush();
		m_TabFaceToolNormal.Flush();
	}
	else
	{
		m_TabRefPoint.Empty();
		m_pFreeTabPoint = NULL;

		m_QuadTreePt.Empty();
		m_TabPoints.Empty();
		m_TabQuad.Empty();
		m_TabPointsLinks.Empty();
		m_TabPointsToolNormal.Empty();
		m_TabFaceToolNormal.Empty();
	}
}

/**************************************************************************/

void	Playspace_Mesh::SetEditMode(Bool _IsSet,Bool _KeepPointNormal,Bool _KeepFaceNormal)
{
	m_IsInEditMode = _IsSet;

	// Switch off Edit Mode.
	if (!m_IsInEditMode)
	{
		m_TabPoints.Minimize();
		m_TabQuad.Minimize();
		m_QuadTreePt.Flush();
		m_TabRefPoint.Flush();
		m_pFreeTabPoint = NULL;

		m_TabPointsLinks.Flush();
		InvalidatePointsLinks();
		if (!_KeepPointNormal)
		{
			m_TabPointsToolNormal.Flush();
			InvalidatePointsToolNormal();
		}
		else
		{
			m_TabPointsToolNormal.Minimize();
		}

		if (!_KeepFaceNormal)
		{
			m_TabFaceToolNormal.Flush();
			InvalidateFaceToolNormal();
		}
		else
		{
			m_TabFaceToolNormal.Minimize();
		}
		return;
	}

	// Switch On.
	m_TabRefPoint.Empty();
	m_pFreeTabPoint = NULL;
	m_QuadTreePt.SetSize(MESH_QUADTREE_TOTAL_SIZE,TRUE);
	memset(m_QuadTreePt.GetArrayPtr(), 0, MESH_QUADTREE_TOTAL_SIZE*sizeof(Playspace_Mesh::RefPoint*));

	for (S32 i = 0; i<m_TabPoints.GetSize(); i++)
		InsertQuadTreePoint(i, m_TabPoints[i]);
}

/**************************************************************************/

S32		Playspace_Mesh::GetQuadTreePoint(Vec3f &Point)
{
	// Là c'est très moyen... C'est lent et le snap ne se fait que sous certaines conditions.
	// => Snap plus petit mais en entier ? à voir...
	S32	iX = Point.x * MESH_QUADTREE_ICELLSIZE + 0.5f;
	S32	iY = Point.y * MESH_QUADTREE_ICELLSIZE + 0.5f;
	S32	iZ = Point.z * MESH_QUADTREE_ICELLSIZE + 0.5f;

	Playspace_Mesh::RefPoint *pCur = m_QuadTreePt[POS_MESH_QUADTREE(iX,iY,iZ)];
	S32		BestPt = -1;
	Float	BestDelta = MESH_QUADTREE_SNAP*MESH_QUADTREE_SNAP;
	while (pCur)
	{
		S32	num = pCur->NumPoint;
		Vec3f &CurPt = m_TabPoints[num];

		Float DeltaY = CurPt.y - Point.y;		// Ici faire une réjection entière... simple fiable et rapide, mais oblige à stocker le CRC entier.
		if ((DeltaY*DeltaY) < BestDelta)
		{
			Float Delta = (CurPt - Point).GetNorm2();
			if (Delta < BestDelta)
			{
				BestPt = num;
				BestDelta = Delta;
			}
		}
		pCur = pCur->pNext;
	}

	if (BestPt >= 0)
		return BestPt;
	return -1;
}

/**************************************************************************/

void	Playspace_Mesh::RemoveQuadTreePoint(S32 Num, Vec3f &Point)
{
	S32		MinX = (S32)((Point.x - MESH_QUADTREE_SNAP)* MESH_QUADTREE_ICELLSIZE + 0.5f);
	S32		MaxX = (S32)((Point.x + MESH_QUADTREE_SNAP)* MESH_QUADTREE_ICELLSIZE + 0.5f);

	S32		MinY = (S32)((Point.y - MESH_QUADTREE_SNAP)* MESH_QUADTREE_ICELLSIZE + 0.5f);
	S32		MaxY = (S32)((Point.y + MESH_QUADTREE_SNAP)* MESH_QUADTREE_ICELLSIZE + 0.5f);

	S32		MinZ = (S32)((Point.z - MESH_QUADTREE_SNAP)* MESH_QUADTREE_ICELLSIZE + 0.5f);
	S32		MaxZ = (S32)((Point.z + MESH_QUADTREE_SNAP)* MESH_QUADTREE_ICELLSIZE + 0.5f);

	for (S32 y = MinY; y <= MaxY; y++)
	{
		for (S32 z = MinZ; z <= MaxZ; z++)
		{
			for (S32 x = MinX; x <= MaxX; x++)
			{
				S32 PosQuad = POS_MESH_QUADTREE(x,y,z);

				Playspace_Mesh::RefPoint **pCur = &(m_QuadTreePt[PosQuad]);
				while (*pCur)
				{
					if ((*pCur)->NumPoint == Num)
					{
						Playspace_Mesh::RefPoint *pNext = (*pCur)->pNext;
						(*pCur)->pNext = m_pFreeTabPoint;
						m_pFreeTabPoint = (*pCur);
						(*pCur) = pNext;
						break;
					}

					pCur = &((*pCur)->pNext);
				}
			}
		}
	}
}

/**************************************************************************/

void	Playspace_Mesh::InsertQuadTreePoint(S32 Num, Vec3f &Point)
{
	S32		MinX = (S32)((Point.x - MESH_QUADTREE_SNAP)* MESH_QUADTREE_ICELLSIZE + 0.5f);
	S32		MaxX = (S32)((Point.x + MESH_QUADTREE_SNAP)* MESH_QUADTREE_ICELLSIZE + 0.5f);

	S32		MinY = (S32)((Point.y - MESH_QUADTREE_SNAP)* MESH_QUADTREE_ICELLSIZE + 0.5f);
	S32		MaxY = (S32)((Point.y + MESH_QUADTREE_SNAP)* MESH_QUADTREE_ICELLSIZE + 0.5f);

	S32		MinZ = (S32)((Point.z - MESH_QUADTREE_SNAP)* MESH_QUADTREE_ICELLSIZE + 0.5f);
	S32		MaxZ = (S32)((Point.z + MESH_QUADTREE_SNAP)* MESH_QUADTREE_ICELLSIZE + 0.5f);

	for (S32 y = MinY; y <= MaxY; y++)
	{
		for (S32 z = MinZ; z <= MaxZ; z++)
		{
			for (S32 x = MinX; x <= MaxX; x++)
			{
				S32 PosQuad = POS_MESH_QUADTREE(x,y,z);
				if (m_QuadTreePt[PosQuad] && (m_QuadTreePt[PosQuad]->NumPoint == Num))	// Se trouve forcément en premier si il vient d'être ajouté.
					continue;	// Already insert here.
				Playspace_Mesh::RefPoint *pNew = NewRefPoint();
				pNew->NumPoint = Num;
				pNew->pNext = m_QuadTreePt[PosQuad];
				m_QuadTreePt[PosQuad] = pNew;
			}
		}
	}
}

/**************************************************************************/

void	Playspace_Mesh::RenumQuadTreePoint(S32 Src, S32 Dst, Vec3f &Point)
{
	S32		MinX = (S32)((Point.x - MESH_QUADTREE_SNAP)* MESH_QUADTREE_ICELLSIZE + 0.5f);
	S32		MaxX = (S32)((Point.x + MESH_QUADTREE_SNAP)* MESH_QUADTREE_ICELLSIZE + 0.5f);

	S32		MinY = (S32)((Point.y - MESH_QUADTREE_SNAP)* MESH_QUADTREE_ICELLSIZE + 0.5f);
	S32		MaxY = (S32)((Point.y + MESH_QUADTREE_SNAP)* MESH_QUADTREE_ICELLSIZE + 0.5f);

	S32		MinZ = (S32)((Point.z - MESH_QUADTREE_SNAP)* MESH_QUADTREE_ICELLSIZE + 0.5f);
	S32		MaxZ = (S32)((Point.z + MESH_QUADTREE_SNAP)* MESH_QUADTREE_ICELLSIZE + 0.5f);

	for (S32 y = MinY; y <= MaxY; y++)
	{
		for (S32 z = MinZ; z <= MaxZ; z++)
		{
			for (S32 x = MinX; x <= MaxX; x++)
			{
				S32 PosQuad = POS_MESH_QUADTREE(x,y,z);

				Playspace_Mesh::RefPoint *pCur = m_QuadTreePt[PosQuad];
				while (pCur)
				{
					if (pCur->NumPoint == Src)
					{
						pCur->NumPoint = Dst;
						break;
					}
					pCur = pCur->pNext;
				}
			}
		}
	}
}


/**************************************************************************/

S32		Playspace_Mesh::GetPointInZoneQuadTreePoint(Vec3f &_Min,Vec3f &_Max,S32DA &_Result)
{
	S32		MinX = (S32)(_Min.x * MESH_QUADTREE_ICELLSIZE + 0.5f);
	S32		MaxX = (S32)(_Max.x * MESH_QUADTREE_ICELLSIZE + 0.5f);

	S32		MinY = (S32)(_Min.y * MESH_QUADTREE_ICELLSIZE + 0.5f);
	S32		MaxY = (S32)(_Max.y * MESH_QUADTREE_ICELLSIZE + 0.5f);

	S32		MinZ = (S32)(_Min.z * MESH_QUADTREE_ICELLSIZE + 0.5f);
	S32		MaxZ = (S32)(_Max.z * MESH_QUADTREE_ICELLSIZE + 0.5f);

	S32 NbPt = _Result.GetSize();

	for (S32 y = MinY; y <= MaxY; y++)
	{
		for (S32 z = MinZ; z <= MaxZ; z++)
		{
			for (S32 x = MinX; x <= MaxX; x++)
			{
				S32 PosQuad = POS_MESH_QUADTREE(x,y,z);

				Playspace_Mesh::RefPoint *pCur = m_QuadTreePt[PosQuad];
				while (pCur)
				{
					S32 NumPoint = pCur->NumPoint;
					Vec3f Pt = m_TabPoints[NumPoint];
					if (	(Pt.x >= _Min.x) && (Pt.x <= _Max.x)
						&&	(Pt.y >= _Min.y) && (Pt.y <= _Max.y)
						&&	(Pt.z >= _Min.z) && (Pt.z <= _Max.z)
						)
					{
						S32 CurPt = 0;
						for (;;)
						{
							if (CurPt<NbPt)
							{
								if (_Result[CurPt++] == NumPoint)
									break;
								continue;
							}
							_Result.Add(NumPoint);
							NbPt++;
							break;
						}
					}
					pCur = pCur->pNext;
				}
			}
		}
	}
	return NbPt;
}

/**************************************************************************/

void	Playspace_Mesh::AddQuad(Vec3f &_p1, Vec3f &_p2, Vec3f &_p3, Vec3f &_p4, Bool _IsVirtual)
{
	Vec3f	Tab[] = {_p1,_p2,_p3,_p4};
	AddSurface(Tab, 4, _IsVirtual);
}

/**************************************************************************/

void	Playspace_Mesh::AddSurface(Vec3f *_pPoints, S32 _NbPoints, Bool _IsVirtual,U8 _IsPaintMode,U8 _IsSeenQuality)
{
	InvalidatePointsLinks();
	InvalidateFaceToolNormal();

	EXCEPTIONC_Z(m_IsInEditMode, "Playspace_Mesh::AddSurface not in Edit Mode");
	if (_NbPoints < 3)
		return;
	EXCEPTIONC_Z(_NbPoints < 32, "too many point in Playspace_Mesh::AddSurface");

	// Add Points.
	S32	RefPoints[32];
	for (S32 i = 0; i<_NbPoints; i++)
		RefPoints[i] = AddPoint(_pPoints[i]);

	// Simplify Surface.
	while (RefPoints[0] == RefPoints[_NbPoints - 1])
	{
		_NbPoints--;
		if (_NbPoints < 3)
			return;
	}

	S32 DestPt = 0;
	S32 NumPt = 1;
	while (NumPt < _NbPoints)
	{
		if (RefPoints[DestPt] != RefPoints[NumPt])
		{
			DestPt++;
			RefPoints[DestPt] = RefPoints[NumPt];
		}
		NumPt++;
	}
	_NbPoints = DestPt + 1;
	if (_NbPoints < 3)
		return;

	// Create Quads and tris.
	S32	CurPoint = 2;
	while (CurPoint < _NbPoints)
	{
		Playspace_Mesh::Face &CurFace = m_TabQuad[m_TabQuad.Add()];
		CurFace.IsTri = FALSE;
		CurFace.IsPaintMode = _IsPaintMode;
		CurFace.IsSeenQuality = _IsSeenQuality;
		CurFace.IsVirtual = _IsVirtual;
		CurFace.IsExternal = FALSE;

		S32	NumPt = 0;
		CurFace.TabPoints[NumPt++] = RefPoints[0];
		CurPoint--;

		while (NumPt < 4)
		{
			if (CurPoint >= _NbPoints)
			{
				// Triangle => Finish and stop.
				EXCEPTIONC_Z(NumPt == 3, "Error in Playspace_Mesh::AddSurface");
				CurFace.IsTri = TRUE;
				CurFace.TabPoints[NumPt++] = RefPoints[CurPoint - 1];
				break;
			}
			CurFace.TabPoints[NumPt++] = RefPoints[CurPoint++];
		}
	}
}

/**************************************************************************/

S32		Playspace_Mesh::NbSharedPoint(S32 _f1, S32 _f2)
{
	Playspace_Mesh::Face &Face1 = m_TabQuad[_f1];
	Playspace_Mesh::Face &Face2 = m_TabQuad[_f2];

	S32 NbPt1 = Face1.IsTri ? 3 : 4;
	S32 NbPt2 = Face2.IsTri ? 3 : 4;

	S32 NbPt = 0;

	for (S32 i = 0; i<NbPt1; i++)
	{
		S32 v1 = Face1.TabPoints[i];
		for (S32 j = 0; j<NbPt2; j++)
		{
			if (Face2.TabPoints[j] == v1)
				NbPt++;
		}
	}

	return NbPt;
}

/**************************************************************************/

void	Playspace_Mesh::MovePoint(S32 _Num, Vec3f &_Dst)
{
	RemoveQuadTreePoint(_Num, m_TabPoints[_Num]);
	m_TabPoints[_Num] = _Dst;
	InsertQuadTreePoint(_Num, m_TabPoints[_Num]);
}

/**************************************************************************/

S32		Playspace_Mesh::AddPoint(Vec3f	&Point)
{
	InvalidatePointsLinks();
	InvalidatePointsToolNormal();
	InvalidateFaceToolNormal();

	EXCEPTIONC_Z(m_IsInEditMode, "Playspace_Mesh::AddSurface not in Edit Mode");

	S32 Num = GetQuadTreePoint(Point);
	if (Num >= 0)
		return Num;

	// Add To TabPoint.
	S32 result = m_TabPoints.Add();
	m_TabPoints[result] = Point;

	// Add To QuadTree.
	InsertQuadTreePoint(result, Point);

	return result;
}

/**************************************************************************/

void	Playspace_Mesh::AddMesh(HU32DA &_TabTriIdx, HugeDynArray_Z<Vec3f> &_TabVtx, Bool IsClockWise)
{
	AddMesh(_TabTriIdx.GetArrayPtr(), _TabTriIdx.GetSize(), _TabVtx.GetArrayPtr(), _TabVtx.GetSize(), IsClockWise);
}

/**************************************************************************/

void	Playspace_Mesh::AddMesh(U32 *_pTabTriIdx, S32 _NbTriIdx, Vec3f *_pTabVtx, S32 _NbVtx, Bool IsClockWise, FlagFace *pFlagsFaces)
{
	HS32DA	TabVtxId;

	InvalidatePointsLinks();
	InvalidatePointsToolNormal();
	InvalidateFaceToolNormal();

	// Init Vtx tab.
	TabVtxId.SetSize(_NbVtx);
	for (S32 i = 0; i < _NbVtx; i++)
		TabVtxId[i] = -1;

	S32 NbTri = _NbTriIdx / 3;
	U32	*pIndex = _pTabTriIdx;
	if (m_TabQuad.GetReservedSize() < (m_TabQuad.GetSize() + NbTri))
		m_TabQuad.SetReserve(m_TabQuad.GetSize() + Max(NbTri,(S32)8192));

	if (m_TabPoints.GetReservedSize() < (m_TabPoints.GetSize() + _NbVtx))
		m_TabPoints.SetReserve(m_TabPoints.GetSize() + Max(_NbVtx,(S32)8192));

	S32	CurFaces = 0;
	for (S32 i = 0; i < NbTri; i++)
	{
		U8	IsPaintMode = 2;
		U8	IsSeenQuality = 3;
		if (pFlagsFaces)
		{
			if (!pFlagsFaces[i].IsMarked)
			{
				pIndex+=3;
				continue;
			}
			IsPaintMode = pFlagsFaces[i].IsPaintMode;
			IsSeenQuality = pFlagsFaces[i].IsSeenQuality;
		}

		U32 p0 = *pIndex++;
		U32 p1 = *pIndex++;
		U32 p2 = *pIndex++;

		if (!IsClockWise)
		{
			::Swap(p0, p2);
		}

		EXCEPTIONC_Z((p0<(U32)_NbVtx), "Playspace_Mesh::AddMesh Bad vertex ");
		EXCEPTIONC_Z((p1<(U32)_NbVtx), "Playspace_Mesh::AddMesh Bad vertex ");
		EXCEPTIONC_Z((p2<(U32)_NbVtx), "Playspace_Mesh::AddMesh Bad vertex ");

		if (TabVtxId[p0] < 0)
			TabVtxId[p0] = AddPoint(_pTabVtx[p0]);

		p0 = TabVtxId[p0];

		if (TabVtxId[p1] < 0)
			TabVtxId[p1] = AddPoint(_pTabVtx[p1]);

		p1 = TabVtxId[p1];

		if (TabVtxId[p2] < 0)
			TabVtxId[p2] = AddPoint(_pTabVtx[p2]);

		p2 = TabVtxId[p2];

		if (p0 == p1)
			continue;
		if (p0 == p2)
			continue;
		if (p1 == p2)
			continue;


		Playspace_Mesh::Face &CurFace = m_TabQuad[m_TabQuad.Add()];

		CurFace.IsTri = TRUE;
		CurFace.IsVirtual = FALSE;
		CurFace.IsPaintMode = IsPaintMode;
		CurFace.IsSeenQuality = IsSeenQuality;
		CurFace.TabPoints[0] = p0;
		CurFace.TabPoints[1] = p1;
		CurFace.TabPoints[2] = p2;
		CurFace.TabPoints[3] = p2;
	}
}

/**************************************************************************/

void	Playspace_Mesh::AddMesh(Playspace_Mesh &_Mesh,Bool _OnlyMarkedFaces)
{
	HS32DA	TabVtxId;

	InvalidatePointsLinks();
	InvalidatePointsToolNormal();
	InvalidateFaceToolNormal();

	// Init Vtx tab.
	S32		NbVtx = _Mesh.m_TabPoints.GetSize();
	Vec3f	*pTabPoints = _Mesh.m_TabPoints.GetArrayPtr();
	TabVtxId.SetSize(NbVtx);
	for (S32 i = 0; i < NbVtx; i++)
		TabVtxId[i] = -1;
	
	S32 NbFaces = _Mesh.m_TabQuad.GetSize();
	Playspace_Mesh::Face *pFaces = _Mesh.m_TabQuad.GetArrayPtr();

	if (m_TabQuad.GetReserved() < NbFaces)
		m_TabQuad.SetReserve(NbFaces);

	for (S32 i = 0; i < NbFaces; i++,pFaces++)
	{
		if (_OnlyMarkedFaces && !pFaces->IsMarked)
			continue;

		S32 *PtPtr = pFaces->TabPoints;
		S32 p0,p1,p2,p3;

		p0 = *PtPtr++;
		if (TabVtxId[p0] < 0)
			TabVtxId[p0] = AddPoint(pTabPoints[p0]);
		p0 = TabVtxId[p0];

		p1 = *PtPtr++;
		if (TabVtxId[p1] < 0)
			TabVtxId[p1] = AddPoint(pTabPoints[p1]);
		p1 = TabVtxId[p1];

		p2 = *PtPtr++;
		if (TabVtxId[p2] < 0)
			TabVtxId[p2] = AddPoint(pTabPoints[p2]);
		p2 = TabVtxId[p2];

		p3 = *PtPtr++;
		if (TabVtxId[p3] < 0)
			TabVtxId[p3] = AddPoint(pTabPoints[p3]);
		p3 = TabVtxId[p3];

		Playspace_Mesh::Face &CurFace = m_TabQuad[m_TabQuad.Add()];
		CurFace.IsExternal = pFaces->IsExternal;
		CurFace.IsTri = pFaces->IsTri;
		CurFace.IsVirtual = pFaces->IsVirtual;
		CurFace.IsPaintMode = pFaces->IsPaintMode;
		CurFace.IsSeenQuality = pFaces->IsSeenQuality;
		CurFace.IsMarked = TRUE;
		CurFace.TabPoints[0] = p0;
		CurFace.TabPoints[1] = p1;
		CurFace.TabPoints[2] = p2;
		CurFace.TabPoints[3] = p3;
	}
}

/**************************************************************************/

void	Playspace_Mesh::RemoveMarkedFaces()
{
	// Now Delete Faces if tagged.
	S32 NbFaces = m_TabQuad.GetSize();
	for (S32 i = NbFaces - 1; i >= 0; i--)
	{
		if (!m_TabQuad[i].IsMarked)
			continue;
		RemoveFace(i, TRUE);
	}
}

/**************************************************************************/

S32		Playspace_Mesh::NbFacesOnEdge(S32 _p0, S32 _p1)
{
	Playspace_Mesh::PointsLinks	&InfosP0 = m_TabPointsLinks[_p0];
	Playspace_Mesh::PointsLinks	&InfosP1 = m_TabPointsLinks[_p1];
	S32 nb = 0;
	S32 f0 = 0;
	S32 f1 = 0;
	while ((f0 < InfosP0.GetNbFaces()) && (f1 < InfosP1.GetNbFaces()))
	{
		if (InfosP0.GetFace(f0) == InfosP1.GetFace(f1))
		{
			nb++;
			f0++;
			f1++;
		}
		else if (InfosP0.GetFace(f0) < InfosP1.GetFace(f1))
			f0++;
		else
			f1++;
	}
	return nb;
}
/**************************************************************************/

void	Playspace_Mesh::RemoveFace(S32 _f0, Bool _RemoveVtxIfNeeded)
{
	// Remove references to that face.
	if (m_HavePointLinks)
	{
		Playspace_Mesh::Face &CurFace = m_TabQuad[_f0];
		S32	NbFacePt = CurFace.IsTri ? 3 : 4;
		for (S32 i = 0; i<NbFacePt; i++)
		{
			Playspace_Mesh::PointsLinks &PtInfos = m_TabPointsLinks[CurFace.TabPoints[i]];
			PtInfos.RemoveFace(_f0);

			if (!PtInfos.GetNbFaces() && _RemoveVtxIfNeeded)
				RemoveVertex(CurFace.TabPoints[i]);
		}
	}

	// At the End ?
	S32 LastFace = m_TabQuad.GetSize()-1;
	if (_f0 == LastFace)
	{
		m_TabQuad.SetSize(LastFace, TRUE);
		return;
	}

	// Invert with Last Face.
	m_TabQuad[_f0] = m_TabQuad[LastFace];
	m_TabQuad.SetSize(LastFace, TRUE);

	// Change reference to that face.
	if (m_HavePointLinks)
	{
		// Refresh Infos.
		Playspace_Mesh::Face &CurFace = m_TabQuad[_f0];
		S32	NbFacePt = CurFace.IsTri ? 3 : 4;
		for (S32 i = 0; i<NbFacePt; i++)
		{
			Playspace_Mesh::PointsLinks &PtInfos = m_TabPointsLinks[CurFace.TabPoints[i]];
			if (PtInfos.RemoveFace(LastFace))
				PtInfos.AddFace(_f0);
		}
	}
}

/**************************************************************************/

Float	Playspace_Mesh::ComputePointNormal(S32 _p0, Vec3f &_Normal, Float *_LocalArea)
{
	if (m_HaveFaceToolNormal)
	{
		// Use already computed Face normal.
		Playspace_Mesh::PointsLinks &PtInfos = m_TabPointsLinks[_p0];
		Vec3f	SumNormal = VEC3F_NULL;

		// Compute Normal.
		Float	TotalArea = 0.f;
		S32 NbLinkFaces = PtInfos.GetNbFaces();

		for (S32 i = 0; i < NbLinkFaces; i++)
		{
			S32 NumFace = PtInfos.GetFace(i);
			Playspace_Mesh::ToolFaceNormal &CurFaceNormal = m_TabFaceToolNormal[NumFace];
			SumNormal += CurFaceNormal.Normal;

			if (m_TabQuad[NumFace].IsTri)
				TotalArea += CurFaceNormal.Surface * 0.333333333333333333333333f; //1/3 of Area per Vertex.
			else
				TotalArea += CurFaceNormal.Surface * 0.25f; // 1/4 of Area per Vertex.
		}

		if (_LocalArea)
			*_LocalArea = TotalArea;

		if (!SumNormal.CNormalize())
		{
			if (!SumNormal.ANormalize())
			{
				SumNormal = VEC3F_UP;
				NbLinkFaces = 0;
			}
		}
		_Normal = SumNormal;

		// Compute Error.
		Float BiggestError = 1000.f;
		for (S32 i = 0; i < NbLinkFaces; i++)
		{
			Playspace_Mesh::ToolFaceNormal &CurFaceNormal = m_TabFaceToolNormal[PtInfos.GetFace(i)];
			Float dot = SumNormal * CurFaceNormal.Normal;
			if (dot < BiggestError)
				BiggestError = dot;
		}
		if (!NbLinkFaces)
			BiggestError = -1.f;
		return BiggestError;
	}
	else
	{
		// Recompute All.
		SafeArray_Z<Vec3f,128,FALSE,FALSE> TabNormal;
		Playspace_Mesh::PointsLinks &PtInfos = m_TabPointsLinks[_p0];
		Vec3f	SumNormal = VEC3F_NULL;
		Vec3f	p1, p2, p3, p4;
	
		// Compute Normal.
		Float	TotalArea = 0.f;
		S32 NbLinkFaces = PtInfos.GetNbFaces();
		for (S32 i = 0; i < NbLinkFaces; i++)
		{
			Playspace_Mesh::Face &CurFace = m_TabQuad[PtInfos.GetFace(i)];

			p1 = m_TabPoints[CurFace.TabPoints[0]];
			p2 = m_TabPoints[CurFace.TabPoints[1]];
			p3 = m_TabPoints[CurFace.TabPoints[3]];
			p4 = m_TabPoints[CurFace.TabPoints[2]];

			Vec3f LocalNormal = (p4 - p1) ^ (p2 - p3);		// Si Quad ou Tri => Aire est correcte
			SumNormal += LocalNormal;		// += Normal * Area * 2.f

			Float Ratio = 1.f/6.f;	// 1/2 * 1/3 of Area per Vertex.
			if (!CurFace.IsTri)
				Ratio = 1.f / 8.f;	// 1/2 * 1/4 of Area per Vertex.

			Float LocalArea = LocalNormal.GetNorm();
			if (LocalArea > 1e-10)
			{
				LocalNormal *= 1.f / LocalArea;
				TotalArea += LocalArea * Ratio;
			}

			TabNormal[i] = LocalNormal;
		}

		if (_LocalArea)
			*_LocalArea = TotalArea;

		if (!SumNormal.CNormalize())
		{
			if (!SumNormal.ANormalize())
			{
				SumNormal = VEC3F_UP;
				NbLinkFaces = 0;
			}
		}
		_Normal = SumNormal;

		// Compute Error.
		Float BiggestError = 1000.f;
		for (S32 i = 0; i < NbLinkFaces; i++)
		{
			Float dot = SumNormal * TabNormal[i];
			if (dot < BiggestError)
				BiggestError = dot;
		}
		if (!NbLinkFaces)
			BiggestError = -1.f;
		return BiggestError;
	}
}

/**************************************************************************/

void	Playspace_Mesh::RemoveVertex(S32 _p0)
{
	EXCEPTIONC_Z(m_HavePointLinks, "Pas de RemoveVertex sans FaceInfos");

	// Invert with Last Ppint.
	RemoveQuadTreePoint(_p0, m_TabPoints[_p0]);

	S32 LastPoint = m_TabPoints.GetSize() - 1;
	if (_p0 == LastPoint)
	{
		m_TabPoints.SetSize(LastPoint, TRUE);
		m_TabPointsLinks.SetSize(LastPoint, TRUE);
		return;
	}

	RenumQuadTreePoint(LastPoint, _p0, m_TabPoints[LastPoint]);
	m_TabPoints[_p0] = m_TabPoints[LastPoint];
	m_TabPointsLinks[_p0] = m_TabPointsLinks[LastPoint];
	m_TabPoints.SetSize(LastPoint, TRUE);
	m_TabPointsLinks.SetSize(LastPoint, TRUE);

	Playspace_Mesh::PointsLinks	&InfosPt = m_TabPointsLinks[_p0];
	S32 NbLinkFaces = InfosPt.GetNbFaces();
	for (S32 i = 0; i<NbLinkFaces; i++)
	{
		Playspace_Mesh::Face &CurFace = m_TabQuad[InfosPt.GetFace(i)];
		for (S32 j = 0; j<4; j++)
		{
			if (CurFace.TabPoints[j] == LastPoint)
				CurFace.TabPoints[j] = _p0;
		}
	}
}

/**************************************************************************/

void		Playspace_Mesh::ColapseEdge(S32 _p0, S32 _p1, bool atMiddle)
{
	EXCEPTIONC_Z(_p0 != _p1, "Can't colapse one vertex");
	EXCEPTIONC_Z(m_HavePointLinks, "Pas de Colapse sans FaceInfos");

	if (atMiddle)
	{
		if (_p1 < _p0)
			::Swap(_p0, _p1);

		// Compute new pos.
		Vec3f p0 = m_TabPoints[_p0];
		Vec3f p1 = m_TabPoints[_p1];

		Vec3f pm = (p0 + p1) * 0.5f;

		MovePoint(_p0, pm);
	}

	// Merge Faces...
	S32 NbResultFaces = 0;
	SafeArray_Z<S32, 128, FALSE, FALSE> ResultFaces;
	SafeArray_Z<S32, 128, FALSE, FALSE> RemovedFaces;
	S32 NbRemovedFaces = 0;

	// Compute to be merge and to be suppressed.
	Playspace_Mesh::PointsLinks	&InfosP0 = m_TabPointsLinks[_p0];
	Playspace_Mesh::PointsLinks	&InfosP1 = m_TabPointsLinks[_p1];
	S32 nb = 0;
	S32 f0 = 0;
	S32 f1 = 0;
	S32 NbLinkFacesP0 = InfosP0.GetNbFaces();
	S32 NbLinkFacesP1 = InfosP1.GetNbFaces();
	while ((f0 < NbLinkFacesP0) && (f1 < NbLinkFacesP1))
	{
		if (InfosP0.GetFace(f0) == InfosP1.GetFace(f1))
		{
			Playspace_Mesh::Face &CurFace = m_TabQuad[InfosP0.GetFace(f0)];
			Bool HaveToDelete = FALSE;
			if (CurFace.IsTri)
				HaveToDelete = TRUE;
			else
			{
				// Cross-Edge.
				for (S32 i=0 ; i<4 ; i++)
				{
					S32 CurPt = CurFace.TabPoints[i];
					if ((CurPt == _p0) || (CurPt == _p1))
					{
						S32 _p3 = CurFace.TabPoints[(i + 2) & 0x3];
						if ((_p3 == _p0) || (_p3 == _p1))
						{
							// Crossed !
							HaveToDelete = TRUE;
							break;
						}
					}
				}
			}

			if (HaveToDelete)
			{
				// Delete this face!
				RemovedFaces[NbRemovedFaces++] = InfosP0.GetFace(f0);
				f0++;
				f1++;
			}
			else
			{
				// keep it because Quad !
				ResultFaces[NbResultFaces++] = InfosP0.GetFace(f0);
				f0++;
				f1++;
			}
		}
		else if (InfosP0.GetFace(f0) < InfosP1.GetFace(f1))
			ResultFaces[NbResultFaces++] = InfosP0.GetFace(f0++);
		else
			ResultFaces[NbResultFaces++] = InfosP1.GetFace(f1++);
	}
	while (f0 < NbLinkFacesP0)
		ResultFaces[NbResultFaces++] = InfosP0.GetFace(f0++);
	while (f1 < NbLinkFacesP1)
		ResultFaces[NbResultFaces++] = InfosP1.GetFace(f1++);

	// result merged.
	InfosP0.Init(ResultFaces.GetArrayPtr(),NbResultFaces);
	f0 = 0;
	while (f0 < NbResultFaces)
	{
		Playspace_Mesh::Face &CurFace = m_TabQuad[ResultFaces[f0]];
		for (S32 j = 0; j<4 ; j++)
		{
			if (CurFace.TabPoints[j] == _p1)
				CurFace.TabPoints[j] = _p0;
		}

		if (!CurFace.IsTri)
		{
			if (CurFace.TabPoints[0] == CurFace.TabPoints[3])
			{
				CurFace.IsTri = TRUE;
				CurFace.TabPoints[3] = CurFace.TabPoints[2];
			}
			else if (CurFace.TabPoints[0] == CurFace.TabPoints[1])
			{
				CurFace.IsTri = TRUE;
				CurFace.TabPoints[1] = CurFace.TabPoints[2];
				CurFace.TabPoints[2] = CurFace.TabPoints[3];
			}
			else if (CurFace.TabPoints[1] == CurFace.TabPoints[2])
			{
				CurFace.IsTri = TRUE;
				CurFace.TabPoints[2] = CurFace.TabPoints[3];
			}		
			else if (CurFace.TabPoints[2] == CurFace.TabPoints[3])
			{
				CurFace.IsTri = TRUE;
			}
		}

		f0++;
	}

	// suppress faces.
	NbRemovedFaces--;
	while (NbRemovedFaces >= 0)
	{
		RemoveFace(RemovedFaces[NbRemovedFaces],FALSE);
		NbRemovedFaces--;
	}

	// suppress Vertex.
	RemoveVertex(_p1);
}

/**************************************************************************/

void	Playspace_Mesh::ComputePointsLinks(Bool _Force)
{
	if (m_HavePointLinks && !_Force)
		return;

	S32 NbPt = m_TabPoints.GetSize();
	m_TabPointsLinks.SetSize(NbPt, TRUE);

	// Init.
	for (S32 i = 0; i < NbPt; i++)
		m_TabPointsLinks[i].SetSize(0);

	// Compute Links.
	S32 NbFaces = m_TabQuad.GetSize();
	for (S32 i = 0; i < NbFaces; i++)
	{
		Playspace_Mesh::Face &CurFace = m_TabQuad[i];
		m_TabPointsLinks[CurFace.TabPoints[0]].AddFace(i);
		m_TabPointsLinks[CurFace.TabPoints[1]].AddFace(i);
		m_TabPointsLinks[CurFace.TabPoints[2]].AddFace(i);
		if (!CurFace.IsTri)
			m_TabPointsLinks[CurFace.TabPoints[3]].AddFace(i);
	}

	m_HavePointLinks = TRUE;
}

/**************************************************************************/

const static Float fExternalGroundTolerance = 0.08f;
const static Float fExternalCeillingTolerance = 0.08f;
const static Float fExternalBackTolerance = -0.16f;
const static Float fExternalSideTolerance = 1.2f;

void	Playspace_Mesh::OldComputeExternalFaces(Float _yGround, Float _yCeilling) // Security => Only for verify ComputeExternalFaces (x 10)
{
	// compute normals
	ComputeFacesToolNormal();

	// get all floor faces
	HS32DA floorIndices; 
	HS32DA wallIndices; 
	S32 NbFaces = m_TabQuad.GetSize();
	floorIndices.SetReserve(NbFaces);
	wallIndices.SetReserve(NbFaces);
	for (S32 f = 0; f < NbFaces; f++)
	{
		const ToolFaceNormal& faceNormal = m_TabFaceToolNormal[f];
		Playspace_Mesh::Face& quad = m_TabQuad[f];
		quad.IsExternal = FALSE;
		if( Abs(faceNormal.Normal.y) < 0.866f )
		{
			wallIndices.Add(f);
		}
		else if( faceNormal.Normal.y > 0.866f )
		{
			if( !quad.IsVirtual )
				floorIndices.Add(f);

			// external floor
			if( faceNormal.Center.y < _yGround + fExternalGroundTolerance)
				quad.IsExternal = TRUE;
		}
		else
		{
			// external ceilling
			if( faceNormal.Center.y > _yCeilling - fExternalGroundTolerance )
				quad.IsExternal = TRUE;
		}
	}

	// check all wall faces
	S32 nbWall = wallIndices.GetSize();
	S32 nbFloor = floorIndices.GetSize();
	for(S32 w = 0; w < nbWall; w++ )
	{
		S32 wallIdx = wallIndices[w];
		const ToolFaceNormal& wallNormal = m_TabFaceToolNormal[wallIdx];
		Vec3f normal = wallNormal.Normal;
		normal.CHNormalize();
		Bool isExternal = TRUE;
		Vec3f tangent(-normal.z,0.f,normal.x); //VEC3F_UP ^ normal;
		for(S32 f = 0; f < nbFloor; f++ )
		{
			S32 floorIdx = floorIndices[f];
			const ToolFaceNormal& floorNormal = m_TabFaceToolNormal[floorIdx];
			if (floorNormal.Center.y < wallNormal.Center.y)
			{
				Vec3f dir(floorNormal.Center.x - wallNormal.Center.x, 0.f, floorNormal.Center.z - wallNormal.Center.z);
				Float dot = (normal.x * dir.x + normal.z * dir.z);
				if (dot < fExternalBackTolerance)
				{
					Float dot2 = (tangent.x*dir.x + tangent.z*dir.z);
					if (Abs(dot2) < Abs(dot) * fExternalSideTolerance)
					{
						isExternal = FALSE;
						break;
					}
				}
			}
		}

		if( isExternal )
			m_TabQuad[wallIdx].IsExternal = TRUE;
	}

	// normal useless
	InvalidateFaceToolNormal(TRUE);
}

/**************************************************************************/

class SortedExternalFacePtr
{
public:
	Playspace_Mesh::Face			*pFace;
	Playspace_Mesh::ToolFaceNormal	*pFaceNormal;
	SortedExternalFacePtr			*pNext;

	FINLINE_Z Bool	operator<(SortedExternalFacePtr &b) {return pFaceNormal->Center.y < b.pFaceNormal->Center.y;}
	FINLINE_Z Bool	operator>(SortedExternalFacePtr &b) {return pFaceNormal->Center.y > b.pFaceNormal->Center.y;}
};

void	Playspace_Mesh::ComputeExternalFaces(Float _yGround, Float _yCeilling)
{
	S32 NbFaces = m_TabQuad.GetSize();
	if (!NbFaces)
		return;

	// compute normals
	ComputeFacesToolNormal();

	// get all floor faces
	HugeDynArray_Z<SortedExternalFacePtr,128,FALSE,FALSE>	TabFloor;
	HugeDynArray_Z<SortedExternalFacePtr,128,FALSE,FALSE>	TabWall;

	TabFloor.SetReserve(NbFaces);
	TabWall.SetReserve(NbFaces);

	Vec3f	vMin = m_TabFaceToolNormal[0].Center;
	Vec3f	vMax = vMin;

	for (S32 f = 0; f < NbFaces; f++)
	{
		Playspace_Mesh::ToolFaceNormal *pFaceNormal = &m_TabFaceToolNormal[f];
		Playspace_Mesh::Face *pFace = &m_TabQuad[f];
		pFace->IsExternal = FALSE;

		vMin = Min(pFaceNormal->Center,vMin);
		vMax = Max(pFaceNormal->Center,vMax);

		if( Abs(pFaceNormal->Normal.y) < 0.866f )
		{
			S32 num = TabWall.Add();
			TabWall[num].pFace = pFace;
			TabWall[num].pFaceNormal = pFaceNormal;
		}
		else if( pFaceNormal->Normal.y > 0.866f )
		{
			if( !pFace->IsVirtual )
			{
				S32 num = TabFloor.Add();
				TabFloor[num].pFace = pFace;
				TabFloor[num].pFaceNormal = pFaceNormal;
			}

			// external floor
			if( pFaceNormal->Center.y < _yGround + fExternalGroundTolerance)
				pFace->IsExternal = TRUE;
		}
		else
		{
			// external ceilling
			if( pFaceNormal->Center.y > _yCeilling - fExternalGroundTolerance )
				pFace->IsExternal = TRUE;
		}
	}

	// Construct QuadTree.

	GenericQSortValDesc(TabFloor.GetSize(),TabFloor.GetArrayPtr());

	SafeArray_Z<SortedExternalFacePtr*,256,FALSE,FALSE>	QuadFloor;
	QuadFloor.Null();
	S32 nbFloor = TabFloor.GetSize();

	Vec3f UnScale = (vMax - vMin) / 16.f;
	Vec3f Scale(1.f/UnScale.x,0.f,1.f/UnScale.z);

	for(S32 f = 0; f < nbFloor; f++ )
	{
		SortedExternalFacePtr *pFloor = &TabFloor[f];
		Vec3f Pos = (pFloor->pFaceNormal->Center - vMin) & Scale;
		SortedExternalFacePtr **pCase = &QuadFloor[(S32)Pos.x + (((S32)Pos.z)<<4)];
		pFloor->pNext = *pCase;
		*pCase = pFloor;
	}

	// check all wall faces
	S32 nbWall = TabWall.GetSize();
	for(S32 w = 0; w < nbWall; w++ )
	{
		const Playspace_Mesh::ToolFaceNormal *pWallNormal = TabWall[w].pFaceNormal;
		Vec3f normal = pWallNormal->Normal;
		Vec3f center = pWallNormal->Center;
		normal.CHNormalize();
		Bool isExternal = TRUE;
		Vec3f tangent(-normal.z,0.f,normal.x); //VEC3F_UP ^ normal;

		Vec3f DeltaPt = vMin;
		if (normal.x < 0.f)
			DeltaPt.x += UnScale.x;
		if (normal.z < 0.f)
			DeltaPt.z += UnScale.z;

		S32		num = 0;
		for (S32 z=0 ; z<16 ; z++)
		{
			Float fX = DeltaPt.x;
			Float fY = (z * UnScale.z) + DeltaPt.z;

			for (S32 x=0 ; x<16 ; x++,num++,fX += UnScale.x)
			{
				SortedExternalFacePtr *pFloor = QuadFloor[num];
				if (!pFloor)
					continue;

				Vec2f MainDir(fX - center.x, fY - center.z);
				Float dot = (normal.x * MainDir.x + normal.z * MainDir.y);
				if (dot > 0.f)
					continue;

				while (pFloor)
				{
					const Playspace_Mesh::ToolFaceNormal *pFloorNormal = pFloor->pFaceNormal;
					if ((pFloorNormal->Center.y - vMin.y) >= 2.00f) // Take only care of horizontal surface under 2m
						break;

					Vec3f dir(pFloorNormal->Center.x - center.x, 0.f, pFloorNormal->Center.z - center.z);
					Float dot = (normal.x * dir.x + normal.z * dir.z);
					if (dot < fExternalBackTolerance)
					{
						dot *= fExternalSideTolerance;
						dot *= dot;
						Float dot2 = (tangent.x*dir.x + tangent.z*dir.z);
						dot2 *= dot2;
						if (dot2 < dot)
						{
							isExternal = FALSE;
							x = z = 16;
							break;
						}
					}
					pFloor = pFloor->pNext;
				}
			}
		}

		if( isExternal )
			TabWall[w].pFace->IsExternal = TRUE;
	}
}

/**************************************************************************/

void	Playspace_Mesh::ComputePointsToolNormal(Bool _Force)
{
	if (m_HavePointsToolNormal && !_Force)
		return;
	EXCEPTIONC_Z(m_HaveFaceToolNormal, "ComputePointsToolNormal Need FaceToolNormal");

	S32 NbPt = m_TabPoints.GetSize();
	m_TabPointsToolNormal.SetSize(NbPt,TRUE);
	for (S32 i = 0; i<NbPt; i++)
	{
		// Compute normale.
		Vec3f vNormal = VEC3F_NULL;
		Float Surface = 0.f;

		Playspace_Mesh::PointsLinks &PtLinks = m_TabPointsLinks[i];
		S32 NbLinkFaces = PtLinks.GetNbFaces();

		for (S32 j = 0; j < NbLinkFaces; j++)
		{
			S32 NumFace = PtLinks.GetFace(j);
			Playspace_Mesh::ToolFaceNormal &CurFaceNormal = m_TabFaceToolNormal[NumFace];
			vNormal += CurFaceNormal.Normal;

			if (m_TabQuad[NumFace].IsTri)
				Surface += CurFaceNormal.Surface * 0.333333333333333333333333f; //1/3 of Area per Vertex.
			else
				Surface += CurFaceNormal.Surface * 0.25f; // 1/4 of Area per Vertex.
		}

		if (!vNormal.CNormalize())
		{
			if (!vNormal.ANormalize())
			{
				vNormal = VEC3F_UP;
				NbLinkFaces = 0;
			}
		}

		// Compute Error.
		Float BiggestError = 1000.f;
		for (S32 j = 0; j < NbLinkFaces; j++)
		{
			Playspace_Mesh::ToolFaceNormal &CurFaceNormal = m_TabFaceToolNormal[PtLinks.GetFace(j)];
			Float dot = vNormal * CurFaceNormal.Normal;
			if (dot < BiggestError)
				BiggestError = dot;
		}
		if (!NbLinkFaces)
			BiggestError = -1.f;

		// Reporte val.
		Playspace_Mesh::ToolPointNormal &PInfos = m_TabPointsToolNormal[i];
		PInfos.Normal = vNormal;
		PInfos.Surface = Surface;
		PInfos.Error = BiggestError;
		
	}

	m_HavePointsToolNormal = TRUE;
}


/**************************************************************************/

void	Playspace_Mesh::ComputeFacesAndPointToolNormalForMarkedFaces()
{
	S32 NbFace = m_TabQuad.GetSize();
	m_TabFaceToolNormal.SetSize(NbFace, TRUE);

	S32 NbPt = m_TabPoints.GetSize();
	m_TabPointsToolNormal.SetSize(NbPt,TRUE);

	// Invalidate normales.
	Playspace_Mesh::ToolPointNormal *pPInfos = m_TabPointsToolNormal.GetArrayPtr();
	for (S32 i = 0; i<NbPt; i++,pPInfos++)
	{
		// Reporte val.
		pPInfos->Surface = -1.f;
	}

	// Mark to be compute normales.
	Playspace_Mesh::ToolFaceNormal *pFInfos = m_TabFaceToolNormal.GetArrayPtr();
	Playspace_Mesh::Face *pCurFace = m_TabQuad.GetArrayPtr();
	for (S32 i = 0 ; i<NbFace ; i++,pFInfos++,pCurFace++)
	{
		pFInfos->Surface = -1.f;
		if (!pCurFace->IsMarked)
			continue;
		m_TabPointsToolNormal[pCurFace->TabPoints[0]].Surface = 0.f;
		m_TabPointsToolNormal[pCurFace->TabPoints[1]].Surface = 0.f;
		m_TabPointsToolNormal[pCurFace->TabPoints[2]].Surface = 0.f;
		m_TabPointsToolNormal[pCurFace->TabPoints[3]].Surface = 0.f;
	}
	pPInfos = m_TabPointsToolNormal.GetArrayPtr();
	Playspace_Mesh::PointsLinks *pPtLinks = m_TabPointsLinks.GetArrayPtr();
	for (S32 i = 0; i<NbPt; i++,pPtLinks++,pPInfos++)
	{
		// Reporte val.
		if (pPInfos->Surface < 0.f)
			continue;
		S32 NbLinkFaces = pPtLinks->GetNbFaces();
		for (S32 j = 0; j < NbLinkFaces; j++)
			m_TabFaceToolNormal[pPtLinks->GetFace(j)].Surface = 0.f;;
	}

	// Compute Marked Face Normales.
	Vec3f	p1,p2,p3,p4;
	pFInfos = m_TabFaceToolNormal.GetArrayPtr();
	pCurFace = m_TabQuad.GetArrayPtr();
	for (S32 i = 0 ; i<NbFace ; i++,pFInfos++,pCurFace++)
	{
		if (!pCurFace->IsMarked && (pFInfos->Surface < 0.f))
			continue;

		p1 = m_TabPoints[pCurFace->TabPoints[0]];
		p2 = m_TabPoints[pCurFace->TabPoints[1]];
		p3 = m_TabPoints[pCurFace->TabPoints[3]];
		p4 = m_TabPoints[pCurFace->TabPoints[2]];

		Vec3f LocalNormal = (p4 - p1) ^ (p2 - p3);		// Si Quad ou Tri => Aire est correcte
		Vec3f Center;
		if (pCurFace->IsTri)
			Center = (p1 + p2 + p4) * 0.333333333333333333333333f;
		else
			Center = (p1 + p2 + p3 + p4) * 0.25f;

		Float LocalArea = LocalNormal.GetNorm();
		if (LocalArea > 1e-20f)
			LocalNormal *= 1.f / LocalArea;

		LocalArea *= 0.5f;

		pFInfos->Normal = LocalNormal;
		pFInfos->Center = Center;
		pFInfos->Surface = LocalArea;
	}	

	// Compute Marked Point Normales.
	pPInfos = m_TabPointsToolNormal.GetArrayPtr();
	pPtLinks = m_TabPointsLinks.GetArrayPtr();
	for (S32 i = 0; i<NbPt; i++,pPInfos++,pPtLinks++)
	{
		if (pPInfos->Surface < 0.f)
			continue;

		// Compute normale.
		Vec3f vNormal = VEC3F_NULL;
		Float Surface = 0.f;

		S32 NbLinkFaces = pPtLinks->GetNbFaces();

		for (S32 j = 0; j < NbLinkFaces; j++)
		{
			S32 NumFace = pPtLinks->GetFace(j);
			Playspace_Mesh::ToolFaceNormal &CurFaceNormal = m_TabFaceToolNormal[NumFace];
			vNormal += CurFaceNormal.Normal;

			if (m_TabQuad[NumFace].IsTri)
				Surface += CurFaceNormal.Surface * 0.333333333333333333333333f; //1/3 of Area per Vertex.
			else
				Surface += CurFaceNormal.Surface * 0.25f; // 1/4 of Area per Vertex.
		}

		if (!vNormal.CNormalize())
		{
			if (!vNormal.ANormalize())
			{
				vNormal = VEC3F_UP;
				NbLinkFaces = 0;
			}
		}

		// Compute Error.
		Float BiggestError = 1000.f;
		for (S32 j = 0; j < NbLinkFaces; j++)
		{
			Playspace_Mesh::ToolFaceNormal &CurFaceNormal = m_TabFaceToolNormal[pPtLinks->GetFace(j)];
			Float dot = vNormal * CurFaceNormal.Normal;
			if (dot < BiggestError)
				BiggestError = dot;
		}
		if (!NbLinkFaces)
			BiggestError = -1.f;

		// Reporte val.
		pPInfos->Normal = vNormal;
		pPInfos->Surface = Surface;
		pPInfos->Error = BiggestError;	
	}
}

/**************************************************************************/

void	Playspace_Mesh::ComputeFacesToolNormal(Bool _Force)
{
	if (m_HaveFaceToolNormal && !_Force)
		return;

	S32 NbFace = m_TabQuad.GetSize();
	m_TabFaceToolNormal.SetSize(NbFace, TRUE);

	Vec3f	p1,p2,p3,p4;

	for (S32 i = 0 ; i<NbFace ; i++)
	{
		Playspace_Mesh::Face &CurFace = m_TabQuad[i];

		p1 = m_TabPoints[CurFace.TabPoints[0]];
		p2 = m_TabPoints[CurFace.TabPoints[1]];
		p3 = m_TabPoints[CurFace.TabPoints[3]];
		p4 = m_TabPoints[CurFace.TabPoints[2]];

		Vec3f LocalNormal = (p4 - p1) ^ (p2 - p3);		// Si Quad ou Tri => Aire est correcte
		Vec3f Center;
		if (CurFace.IsTri)
			Center = (p1 + p2 + p4) * 0.333333333333333333333333f;
		else
			Center = (p1 + p2 + p3 + p4) * 0.25f;

		Float LocalArea = LocalNormal.GetNorm();
		if (LocalArea > 1e-20f)
			LocalNormal *= 1.f / LocalArea;

		LocalArea *= 0.5f;

		Playspace_Mesh::ToolFaceNormal &FInfos = m_TabFaceToolNormal[i];
		FInfos.Normal = LocalNormal;
		FInfos.Center = Center;
		FInfos.Surface = LocalArea;
	}

	m_HaveFaceToolNormal = TRUE;
}

/**************************************************************************/

// find the 2 faces with p0 and p1, and, in each case, the other point in that face
FINLINE_Z S32	Playspace_Mesh::GetCommonFacesOnPoints(S32 _p0, S32 _p1, S32 *_TabResult, S32 _NbMax, S32 _IgnoreFaceId)
{
	S32 nbFaces = 0;

	S32 f0 = 0;
	S32 f1 = 0;

	Playspace_Mesh::PointsLinks	&InfosP0 = m_TabPointsLinks[_p0];
	Playspace_Mesh::PointsLinks	&InfosP1 = m_TabPointsLinks[_p1];

	S32 NbLinkFacesP0 = InfosP0.GetNbFaces();
	S32 NbLinkFacesP1 = InfosP1.GetNbFaces();
	while ((f0 < NbLinkFacesP0) && (f1 < NbLinkFacesP1))
	{
		S32 NumF0 = InfosP0.GetFace(f0);
		S32 NumF1 = InfosP1.GetFace(f1);
		if (NumF0 == NumF1)
		{
			if (NumF0 != _IgnoreFaceId)
			{
				if (nbFaces == _NbMax)
					return nbFaces;
				_TabResult[nbFaces++] = NumF0;
			}

			f0++;
			f1++;
		}
		else if (NumF0 < NumF1)
			f0++;
		else
			f1++;
	}

	return nbFaces;
}

FINLINE_Z Bool	Playspace_Mesh::IsPointMovable(S32 p, Vec3f& newPos)
{
	S32 idx1, idx2, idx3;
	S32 NbLinkFaces = m_TabPointsLinks[p].GetNbFaces();

	for (S32 i = 0; i < NbLinkFaces; i++)
	{
		S32 numF = m_TabPointsLinks[p].GetFace(i);
		Playspace_Mesh::Face &CurFace = m_TabQuad[numF];
		if (CurFace.IsTri)
		{
			if (CurFace.TabPoints[0] == p)
			{
				idx1 = 1;
				idx2 = 2;
			}
			else if (CurFace.TabPoints[1] == p)
			{
				idx1 = 2;
				idx2 = 0;
			}
			else
			{
				idx1 = 0;
				idx2 = 1;
			}
			Vec3f v1 = m_TabPoints[CurFace.TabPoints[idx2]] - m_TabPoints[CurFace.TabPoints[idx1]];
			Vec3f v2 = m_TabPoints[p] - m_TabPoints[CurFace.TabPoints[idx1]];
			Vec3f v3 = newPos - m_TabPoints[CurFace.TabPoints[idx1]];

			if (((v1 ^ v2) * (v1 ^ v3)) < 0.0f)
				return FALSE;
		}
		else
		{
			if (CurFace.TabPoints[0] == p)
			{
				idx1 = 1;
				idx2 = 2;
				idx3 = 3;
			}
			else if (CurFace.TabPoints[1] == p)
			{
				idx1 = 2;
				idx2 = 3;
				idx3 = 0;
			}
			else if (CurFace.TabPoints[2] == p)
			{
				idx1 = 3;
				idx2 = 0;
				idx3 = 1;
			}
			else
			{
				idx1 = 0;
				idx2 = 1;
				idx3 = 2;
			}

			Vec3f v0 = m_TabPoints[CurFace.TabPoints[idx1]] - newPos;
			Vec3f v1 = m_TabPoints[CurFace.TabPoints[idx2]] - m_TabPoints[CurFace.TabPoints[idx1]];
			Vec3f v2 = m_TabPoints[CurFace.TabPoints[idx3]] - m_TabPoints[CurFace.TabPoints[idx2]];
			Vec3f v3 = newPos - m_TabPoints[CurFace.TabPoints[idx3]];

			Vec3f n0 = v3 ^ v0;
			Vec3f n1 = v0 ^ v1;
			Vec3f n2 = v1 ^ v2;
			Vec3f n3 = v2 ^ v3;

			S32 nb = 0;

			if ((n0 * n1) < 0.0f)
				nb++;

			if ((n0 * n2) < 0.0f)
				nb++;

			if ((n0 * n3) < 0.0f)
				nb++;

			if (nb == 2)
				return FALSE;
		}
	}
	return TRUE;
}


FINLINE_Z Bool	Playspace_Mesh::IsCollapsableEdgeOnTri(S32 p0, S32 p1, float cosAngleMin2, Bool checkExternal /*= FALSE*/)
{
	S32 idx1, idx2;
	S32 pt[2];
	S32 TabFaces[3];

	S32 NbFaces = GetCommonFacesOnPoints(p0, p1, TabFaces, 3);
	if (NbFaces != 2)
		return FALSE;

	S32 CommonFace2 = TabFaces[0];
	S32 CommonFace3 = TabFaces[1];

	Bool isExternalF2 = m_TabQuad[CommonFace2].IsExternal;
	Bool isExternalF3 = m_TabQuad[CommonFace3].IsExternal;

	if( checkExternal && (isExternalF2 != isExternalF3) )
		return FALSE;	

	Vec3f middle = (m_TabPoints[p0] + m_TabPoints[p1]) * 0.5f;

	pt[0] = p0;
	pt[1] = p1;

	for (S32 p = 0; p < 2; p++)
	{
		S32 NbLinkFacesP = m_TabPointsLinks[pt[p]].GetNbFaces();
		for (S32 i = 0; i < NbLinkFacesP ; i++)
		{
			S32 numF = m_TabPointsLinks[pt[p]].GetFace(i);
			if ((numF != CommonFace2) && (numF != CommonFace3))
			{
				Playspace_Mesh::Face &CurFace = m_TabQuad[numF];
				if( checkExternal )
				{
					Bool isExternal = CurFace.IsExternal;
					if( isExternal != isExternalF2 )
						return FALSE;
				}

				if (CurFace.TabPoints[0] == pt[p])
				{
					idx1 = 1;
					idx2 = 2;
				}
				else if (CurFace.TabPoints[1] == pt[p])
				{
					idx1 = 0;
					idx2 = 2;
				}
				else
				{
					idx1 = 0;
					idx2 = 1;
				}
				Vec3f v1 = m_TabPoints[CurFace.TabPoints[idx2]] - m_TabPoints[CurFace.TabPoints[idx1]];
				Vec3f v2 = m_TabPoints[pt[p]] - m_TabPoints[CurFace.TabPoints[idx1]];
				Vec3f v3 = middle - m_TabPoints[CurFace.TabPoints[idx1]];

				if (((v1 ^ v2) * (v1 ^ v3)) < 0.0f)
					return FALSE;

				Vec3f v4 = middle - m_TabPoints[CurFace.TabPoints[idx2]];
				float norm2 = v3.GetNorm2() * v4.GetNorm2();
				if (norm2 < 1e-3f * 1e-3f)
					return FALSE;

				float CosAngle = v3 * v4;

				if ((CosAngle * CosAngle) > (cosAngleMin2 * norm2))
					return FALSE;
			}
		}
	}
	return TRUE;
}

/**************************************************************************/

void	Playspace_Mesh::RemoveDeadFaces(Float _DistMax)
{
	EXCEPTIONC_Z(m_IsInEditMode, "Playspace_Mesh::AddSurface not in Edit Mode");

	InvalidateFaceToolNormal();
	InvalidatePointsToolNormal();
	ComputePointsLinks(FALSE);

	// ReSnap points.
	for (S32 i = 0; i<m_TabPoints.GetSize(); i++)
	{
		S32 Num = GetQuadTreePoint(m_TabPoints[i]);
		if (Num == i)
			continue;
		EXCEPTIONC_Z(Num >= 0,"RemoveDeadFaces : Bad point");
		// Another same vertex => merge.
		ColapseEdge(i,Num,FALSE);

		// Redo it...
		i--;
	}

	// Remove dead faces.
	Float DistMax2 = _DistMax * _DistMax;
	S32 iCurFace = m_TabQuad.GetSize() -1;

	while (iCurFace >= 0)
	{
		Bool	RedoFace = FALSE;
		Playspace_Mesh::Face &CurFace = m_TabQuad[iCurFace];

		if (CurFace.IsTri)
		{
			// Triangle.
			S32 p0 = CurFace.TabPoints[0];
			S32 p1 = CurFace.TabPoints[1];
			S32 p2 = CurFace.TabPoints[2];

			Bool DeleteIt = FALSE;

			// Real Dead.
			if ((p0 == p1) || (p1 == p2) || (p2 == p0))
				RemoveFace(iCurFace, TRUE);
			else
			{
				// Tiny edge.
				Vec3f &v0 = m_TabPoints[p0];
				Vec3f &v1 = m_TabPoints[p1];
				Vec3f &v2 = m_TabPoints[p2];

				Float d0 = (v0 - v1).GetNorm2();
				Float d1 = (v1 - v2).GetNorm2();
				Float d2 = (v2 - v0).GetNorm2();

				if (d0 < DistMax2)
					ColapseEdge(p0, p1);
				else if (d1 < DistMax2)
					ColapseEdge(p1, p2);
				else if (d2 < DistMax2)
					ColapseEdge(p2, p0);
			}
		}
		else
		{
			// Quad.
			S32 p0 = CurFace.TabPoints[0];
			S32 p1 = CurFace.TabPoints[1];
			S32 p2 = CurFace.TabPoints[2];
			S32 p3 = CurFace.TabPoints[3];

			// Crossed faces.
			if ((p0 == p2) || (p1 == p3))
				RemoveFace(iCurFace, TRUE);
			else if ((m_TabPoints[p0] - m_TabPoints[p2]).GetNorm2() < DistMax2)
				ColapseEdge(p0, p2);
			else if ((m_TabPoints[p1] - m_TabPoints[p3]).GetNorm2() < DistMax2)
				ColapseEdge(p1, p3);
			else
			{
				// Try To Triangumlarize...
				if (p0 == p1)
				{
					RedoFace = TRUE;
					CurFace.IsTri = TRUE;
					CurFace.TabPoints[1] = CurFace.TabPoints[2];
					CurFace.TabPoints[2] = CurFace.TabPoints[3];
				}
				else if (p1 == p2)
				{
					RedoFace = TRUE;
					CurFace.IsTri = TRUE;
					CurFace.TabPoints[2] = CurFace.TabPoints[3];
				}
				else if (p2 == p3)
				{
					RedoFace = TRUE;
					CurFace.IsTri = TRUE;
				}
				else if (p0 == p3)
				{
					RedoFace = TRUE;
					CurFace.TabPoints[3] = CurFace.TabPoints[2];
					CurFace.IsTri = TRUE;
				}
				else if ((m_TabPoints[p0] - m_TabPoints[p1]).GetNorm2() < DistMax2)
				{
					RedoFace = TRUE;
					ColapseEdge(p0, p1);
				}
				else if ((m_TabPoints[p1] - m_TabPoints[p2]).GetNorm2() < DistMax2)
				{
					RedoFace = TRUE;
					ColapseEdge(p1, p2);
				}
				else if ((m_TabPoints[p2] - m_TabPoints[p3]).GetNorm2() < DistMax2)
				{
					RedoFace = TRUE;
					ColapseEdge(p2, p3);
				}
				else if ((m_TabPoints[p3] - m_TabPoints[p0]).GetNorm2() < DistMax2)
				{
					RedoFace = TRUE;
					ColapseEdge(p3, p0);
				}
			}
		}

		// Next.
		if (!RedoFace)
			iCurFace--;
		if (iCurFace >= m_TabQuad.GetSize())
			iCurFace = m_TabQuad.GetSize() - 1;
	}
}

/**************************************************************************/

Bool Playspace_Mesh::DecomposePolyedra(vItem* points, S32 nbPts, Vec3f& normal, S32* output, S32& nbGenTri)
{
	vItem *head, *lstPtr, *lstPrv, *lstNxt, *minPtr, *maxPtr;
	S32 currNbPoints, nb = 0;
	Float minCross, maxCross;

	if (nbPts < 3)
		return FALSE;

	if (nbPts == 3)
	{
		nbGenTri = 1;
		lstPtr = points;
		for (S32 i = 0; i < 3; i++)
		{
			output[i] = lstPtr->point;
			lstPtr = lstPtr->next;
		}
		return TRUE;
	}

	nbGenTri = 0;
	head = points;
	currNbPoints = nbPts;
	while (currNbPoints > 4)
	{
		// select a candidate

		minCross = 1e8f;
		maxCross = 0.0f;
		minPtr = NULL;
		maxPtr = NULL;

		lstPtr = head;
		for (S32 i = 0; i < currNbPoints; i++)
		{
			if ((lstPtr->crossEdge > (1.0e-3f * 1.0e-3f)) && (lstPtr->crossCenter > (1.0e-3f * 1.0e-3f)))
			{
				if (lstPtr->dot <= 0.0f)
				{
					if (lstPtr->crossEdge < minCross)
					{
						minCross = lstPtr->crossEdge;
						minPtr = lstPtr;
					}
				}
				else
				{
					if (lstPtr->crossEdge > maxCross)
					{
						maxCross = lstPtr->crossEdge;
						maxPtr = lstPtr;
					}
				}
			}
			lstPtr = lstPtr->next;
		}

		if ((!minPtr) && (!maxPtr))		// no candidate ???  in theory not possible...
			return FALSE;

		if (minPtr)
			lstPtr = minPtr;		// take the negative dot in priority
		else
			lstPtr = maxPtr;

		// emit a triangle

		lstPrv = lstPtr->prev;
		lstNxt = lstPtr->next;

		output[nb++] = lstPrv->point;
		output[nb++] = lstPtr->point;
		output[nb++] = lstNxt->point;
		nbGenTri++;

		// remove the point in the chain

		lstPrv->next = lstPtr->next;
		lstNxt->prev = lstPtr->prev;

		if (head == lstPtr)
			head = lstPtr->next;

		// recompute next and prev attributes (crossEdge, dot and crossCenter)

		vItem* lstPrvPrv = lstPrv->prev;
		vItem* lstNxtNxt = lstNxt->next;

		lstPrv->edge = lstNxt->radius - lstPrv->radius;

		Vec3f v1 = (lstPrvPrv->edge ^ lstPrv->edge);
		v1.ANormalize();
		lstPrv->crossEdge = v1 * normal;
		lstPrv->dot = lstPrvPrv->edge * lstPrv->edge;

		v1 = (lstPrvPrv->radius ^ lstNxt->radius);
		v1.ANormalize();
		lstPrv->crossCenter = v1 * normal;

		v1 = (lstPrv->edge ^ lstNxt->edge);
		v1.ANormalize();
		lstNxt->crossEdge = v1 * normal;
		lstNxt->dot = lstPrv->edge * lstNxt->edge;

		v1 = (lstPrv->radius ^ lstNxtNxt->radius);
		v1.ANormalize();
		lstNxt->crossCenter = v1 * normal;

		// don't forget to update the current number of point ;)

		currNbPoints--;
	}

	// here currNbPoints == 4

	minCross = 1e8f;
	maxCross = 0.0f;
	minPtr = NULL;
	maxPtr = NULL;

	lstPtr = head;
	for (S32 i = 0; i < currNbPoints; i++)
	{
		if (lstPtr->crossEdge >(1.0e-3f * 1.0e-3f))
		{
			if (lstPtr->dot <= 0.0f)
			{
				if (lstPtr->crossEdge < minCross)
				{
					minCross = lstPtr->crossEdge;
					minPtr = lstPtr;
				}
			}
			else
			{
				if (lstPtr->crossEdge > maxCross)
				{
					maxCross = lstPtr->crossEdge;
					maxPtr = lstPtr;
				}
			}
		}
		lstPtr = lstPtr->next;
	}

	if ((!minPtr) && (!maxPtr))		// no candidate ???  in theory not possible...
		return FALSE;

	if (minPtr)
		lstPtr = minPtr;		// take the negative dot in priority
	else
		lstPtr = maxPtr;

	// emit two triangles

	lstPrv = lstPtr->prev;
	lstNxt = lstPtr->next;

	output[nb++] = lstPrv->point;
	output[nb++] = lstPtr->point;
	output[nb++] = lstNxt->point;
	nbGenTri++;

	vItem* lstNxtNxt = lstNxt->next;		// is the same as would be lstPrvPrv

	output[nb++] = lstNxt->point;
	output[nb++] = lstNxtNxt->point;
	output[nb++] = lstPrv->point;
	nbGenTri++;

	return TRUE;
}

/**************************************************************************/

void	Playspace_Mesh::SimplifyTri2(Float cosMaxErr, Bool _ignoreVirtual /*= TRUE*/, Bool _ignoreExternal /*= TRUE*/)
{
	if (IsEmpty())
		return;

	ComputePointsLinks();
	InvalidateFaceToolNormal();
	InvalidatePointsToolNormal();

	HS8DA ptsTab;
	HS8DA faces;

	HugeDynArray_Z<Vec3f, 4096, FALSE, FALSE> faceNormals;

	S32 nbPass = 1;							//	JY: peut-etre les passer en params ?
	S32 maxNeighbours = 8;					//

	S32 triangles[96];
	S32 nbTri0, nbTri1, nbTriTotal;

	vItem list[32];											// la taille doit IMPERATIVEMENT etre supérieure à (maxNeighbours + 2)
	vItem *lstPtr, *lstNxt, *lstPrv, *head[2], *tail[2];

	S32 nb[2], currPoint, lastPoint, currState, currFace;
	Bool virt[2], ext[2];									// are the clusters virtual or external ?
	Bool toggled = FALSE, normOk[2];
	Vec3f center, v1, v2, normal[2];
	S32* ptr, *ptr2;

	S32 nbFaces = m_TabQuad.GetSize();
	Playspace_Mesh::Face* pFaceIdx = m_TabQuad.GetArrayPtr();

	S32 nbPoints = m_TabPoints.GetSize();
	Vec3f* TabPoints = m_TabPoints.GetArrayPtr();

	ptsTab.SetSize(nbPoints, TRUE);
	S8* pPtsTab = ptsTab.GetArrayPtr();
	memset(pPtsTab, 0, nbPoints * sizeof(S8));

	faces.SetSize(nbFaces, TRUE);
	S8* pFaces = faces.GetArrayPtr();
	memset(pFaces, 0, nbFaces * sizeof(S8));

	faceNormals.SetSize(nbFaces, TRUE);
	Vec3f* pFaceNormals = faceNormals.GetArrayPtr();

	for (S32 i = 0; i < nbFaces; i++)
	{
		ptr = pFaceIdx[i].TabPoints;
		v1 = (TabPoints[ptr[1]] - TabPoints[ptr[0]]) ^ (TabPoints[ptr[2]] - TabPoints[ptr[0]]);
		Float len2 = v1.GetNorm2();
		if (len2 < (1.0e-6f * 1.0e-6f))
			pFaces[i] = 126;
		else
			pFaceNormals[i] = v1 * InvSqrt(1.0f, len2);		// INVSQRT
	}

	for (S32 pass = 0; pass < nbPass; pass++)
	{
		for (S32 pt = 0; pt < nbPoints; pt++)
		{
			if (pPtsTab[pt])
				goto Next_point_label;	// point already removed

			center = TabPoints[pt];
			PointsLinks& ptLinks = m_TabPointsLinks[pt];
			S32 nbNeighbours = ptLinks.GetNbFaces(); 

			if ((nbNeighbours > 2) && (nbNeighbours <= maxNeighbours))
			{
				S32 numFace = ptLinks.GetFace(0);
				ptr = pFaceIdx[numFace].TabPoints;

				head[0] = list;
				tail[0] = list + 1;

				if (pt == ptr[0])
				{
					list->point = ptr[1];
					tail[0]->point = ptr[2];
				}
				else if (pt == ptr[1])
				{
					list->point = ptr[2];
					tail[0]->point = ptr[0];
				}
				else
				{
					list->point = ptr[0];
					tail[0]->point = ptr[1];
				}

				list->next = tail[0];
				tail[0]->prev = list;
				list->radius = TabPoints[list->point] - center;
				tail[0]->radius = TabPoints[tail[0]->point] - center;
				nb[0] = 2;
				nb[1] = 0;
				lastPoint = tail[0]->point;
				lstNxt = tail[0] + 1;

				ext[0] = pFaceIdx[numFace].IsExternal;
				virt[0] = pFaceIdx[numFace].IsVirtual;

				toggled = FALSE;
				if (pFaces[numFace] != 126)
				{
					normal[0] = pFaceNormals[numFace];
					normOk[0] = TRUE;
				}
				else
					normOk[0] = FALSE;

				normOk[1] = FALSE;
				currState = 0;
				for (S32 i = 1; i < nbNeighbours; i++)
				{
					currPoint = -1;
					for (S32 j = 0; j < nbNeighbours; j++)
					{
						currFace = ptLinks.GetFace(j);
						ptr = pFaceIdx[currFace].TabPoints;
						if ((ptr[0] == pt) && (ptr[1] == lastPoint))
						{
							currPoint = ptr[2];
							break;
						}
						else if ((ptr[1] == pt) && (ptr[2] == lastPoint))
						{
							currPoint = ptr[0];
							break;
						}
						else if ((ptr[2] == pt) && (ptr[0] == lastPoint))
						{
							currPoint = ptr[1];
							break;
						}
					}

					//if (pFaces[currFace] == 126)
					//	goto Next_point_label;

					if (currPoint == -1)
						goto Next_point_label;		// Rejection => point on border

					if (((pFaces[currFace] != 126) && normOk[currState] && ((normal[currState] * pFaceNormals[currFace]) < cosMaxErr)) || (!_ignoreVirtual && (virt[currState] != pFaceIdx[currFace].IsVirtual)) || (!_ignoreExternal && (ext[currState] != pFaceIdx[currFace].IsExternal)))
					{
						if (toggled)
							goto Next_point_label;		// Rejection => more than two clusters

						if (currState == 0)
						{
							currState++;
							lstNxt->point = lastPoint;
							lstNxt->radius = TabPoints[lastPoint] - center;
							head[1] = tail[1] = lstNxt++;
							nb[1]++;
							virt[1] = pFaceIdx[currFace].IsVirtual;
							ext[1] = pFaceIdx[currFace].IsExternal;
						}
						else
						{
							if (((pFaces[currFace] != 126) && normOk[0] && ((normal[0] * pFaceNormals[currFace]) < cosMaxErr)) || (!_ignoreVirtual && (virt[0] != pFaceIdx[currFace].IsVirtual)) || (!_ignoreExternal && (ext[0] != pFaceIdx[currFace].IsExternal)))
								goto Next_point_label;			// Rejection => more than two clusters
							else
							{
								currState--;
								lstNxt->point = lastPoint;
								lstNxt->radius = TabPoints[lastPoint] - center;
								tail[0]->next = lstNxt;
								lstNxt->prev = tail[0];
								tail[0] = lstNxt++;
								nb[0]++;
								toggled = TRUE;
							}
						}
					}
					if ((pFaces[currFace] != 126) && !normOk[currState])
					{
						normal[currState] = pFaceNormals[currFace];
						normOk[currState] = TRUE;
					}
					lstNxt->point = lastPoint = currPoint;
					lstNxt->radius = TabPoints[lastPoint] - center;
					tail[currState]->next = lstNxt;
					lstNxt->prev = tail[currState];
					tail[currState] = lstNxt++;
					nb[currState]++;
				}

				if (currState == 0)
				{
					tail[0] = tail[0]->prev;		// because we have already put the first point
					nb[0]--;
				}

				tail[0]->next = list;
				list->prev = tail[0];

				if (nb[1])
				{
					tail[1]->next = head[1];
					head[1]->prev = tail[1];
				}

				if ((currPoint != list->point) || !normOk[0] || ((nb[1] != 0) && !normOk[1]))			// Rejection => pathological cases (not 2-manifold or only degenerate triangles)
					goto Next_point_label;

				if ((nb[0] == 2) && (nb[1] == nbNeighbours))
				{
					head[0] = head[1];
					nb[0] = nb[1];
					nb[1] = 0;
					normal[0] = normal[1];
					ext[0] = ext[1];
					virt[0] = virt[1];
				}

				if ((nb[1] == 2) && (nb[0] == nbNeighbours))
					nb[1] = 0;

				if ((nb[0] == 2) || (nb[1] == 2))			// Rejection => pathological cases (like branching point on Riemann surface)
					goto Next_point_label;

				lstPtr = head[0];
				lstNxt = NULL;
				while (lstNxt != head[0])
				{
					lstNxt = lstPtr->next;
					lstPtr->edge = lstNxt->radius - lstPtr->radius;
					lstPtr = lstNxt;
				}

				lstPtr = head[0];
				lstNxt = NULL;
				while (lstNxt != head[0])
				{
					lstPrv = lstPtr->prev;
					lstNxt = lstPtr->next;
					Vec3f v1 = (lstPrv->edge ^ lstPtr->edge);
					v1.ANormalize();
					lstPtr->crossEdge = v1 * normal[0];
					lstPtr->dot = lstPrv->edge * lstPtr->edge;
					v1 = (lstPrv->radius ^ lstNxt->radius);
					v1.ANormalize();
					lstPtr->crossCenter = v1 * normal[0];
					lstPtr = lstNxt;
				}
				nbTri1 = 0;
				if (DecomposePolyedra(head[0], nb[0], normal[0], triangles, nbTri0))
				{
					if (nb[1])
					{
						lstPtr = head[1];
						lstNxt = NULL;
						while (lstNxt != head[1])
						{
							lstNxt = lstPtr->next;
							lstPtr->edge = lstNxt->radius - lstPtr->radius;
							lstPtr = lstNxt;
						}

						lstPtr = head[1];
						lstNxt = NULL;
						while (lstNxt != head[1])
						{
							lstPrv = lstPtr->prev;
							lstNxt = lstPtr->next;
							Vec3f v2 = (lstPrv->edge ^ lstPtr->edge);
							v2.ANormalize();
							lstPtr->crossEdge = v2 * normal[1];
							lstPtr->dot = lstPrv->edge * lstPtr->edge;
							v2 = (lstPrv->radius ^ lstNxt->radius);
							v2.ANormalize();
							lstPtr->crossCenter = v2 * normal[1];
							lstPtr = lstNxt;
						}

						if (!DecomposePolyedra(head[1], nb[1], normal[1], triangles + (3 * nbTri0), nbTri1))		// Rejection => decompose polyedra returned FALSE
							goto Next_point_label;
					}
					nbTriTotal = nbTri0 + nbTri1;
					EXCEPTIONC_Z((nbTriTotal == (nbNeighbours - 2)), "Le mesh est dans un espace non euclidien ou bien il y a un gros bug dans DecomposePolyedra...");

					if (nbTriTotal != (nbNeighbours - 2))		// on est dans la 4e dimension...
						goto Next_point_label;

					ptr2 = triangles;
					for (S32 j = 0; j < nbTriTotal; j++)
					{
						S32 numFace = ptLinks.GetFace(j);
						ptr = pFaceIdx[numFace].TabPoints;

						if (pt != ptr[0])
							m_TabPointsLinks[ptr[0]].RemoveFace(numFace);

						if (pt != ptr[1])
							m_TabPointsLinks[ptr[1]].RemoveFace(numFace);

						if (pt != ptr[2])
							m_TabPointsLinks[ptr[2]].RemoveFace(numFace);

						ptr[0] = ptr2[0];
						ptr[1] = ptr2[1];
						ptr[2] = ptr2[2];
						ptr[3] = ptr2[2];

						m_TabPointsLinks[ptr2[0]].AddFace(numFace);
						m_TabPointsLinks[ptr2[1]].AddFace(numFace);
						m_TabPointsLinks[ptr2[2]].AddFace(numFace);

						if (_ignoreVirtual)
							pFaceIdx[numFace].IsVirtual = FALSE;
						else
							pFaceIdx[numFace].IsVirtual = (j < nbTri0 ? virt[0] : virt[1]);

						if (_ignoreExternal)
							pFaceIdx[numFace].IsExternal = FALSE;
						else
							pFaceIdx[numFace].IsExternal = (j < nbTri0 ? ext[0] : ext[1]);

						v1 = (TabPoints[ptr[1]] - TabPoints[ptr[0]]) ^ (TabPoints[ptr[2]] - TabPoints[ptr[0]]);
						Float len2 = v1.GetNorm2();
						if (len2 < (1.0e-6f * 1.0e-6f))
							pFaces[numFace] = 126;
						else
							pFaceNormals[numFace] = v1 * InvSqrt(1.0f, len2);		// INVSQRT

						ptr2 += 3;
					}

					numFace = ptLinks.GetFace(nbTriTotal);
					ptr = pFaceIdx[numFace].TabPoints;

					if (pt != ptr[0])
						m_TabPointsLinks[ptr[0]].RemoveFace(numFace);

					if (pt != ptr[1])
						m_TabPointsLinks[ptr[1]].RemoveFace(numFace);

					if (pt != ptr[2])
						m_TabPointsLinks[ptr[2]].RemoveFace(numFace);

					pFaces[numFace] = 127;
					numFace = ptLinks.GetFace(nbTriTotal + 1);
					ptr = pFaceIdx[numFace].TabPoints;

					if (pt != ptr[0])
						m_TabPointsLinks[ptr[0]].RemoveFace(numFace);

					if (pt != ptr[1])
						m_TabPointsLinks[ptr[1]].RemoveFace(numFace);

					if (pt != ptr[2])
						m_TabPointsLinks[ptr[2]].RemoveFace(numFace);

					pFaces[numFace] = 127;
					pPtsTab[pt] = 1;
				}
			}
			else
			{
				if (nbNeighbours == 2)
				{
					S32 p0, p1, p2, p3, p4;
					S32 numFace0 = ptLinks.GetFace(0);
					S32 numFace1 = ptLinks.GetFace(1);

					if (!_ignoreVirtual && (pFaceIdx[numFace0].IsVirtual != pFaceIdx[numFace1].IsVirtual))
						goto Next_point_label;

					if (!_ignoreExternal && (pFaceIdx[numFace0].IsExternal != pFaceIdx[numFace1].IsExternal))
						goto Next_point_label;

					ptr = pFaceIdx[numFace0].TabPoints;
					if (pt == ptr[0])
					{
						p0 = ptr[1];
						p1 = ptr[2];
					}
					else if (pt == ptr[1])
					{
						p0 = ptr[2];
						p1 = ptr[0];
					}
					else
					{
						p0 = ptr[0];
						p1 = ptr[1];
					}

					ptr2 = pFaceIdx[numFace1].TabPoints;
					if (pt == ptr2[0])
					{
						p2 = ptr2[1];
						p3 = ptr2[2];
					}
					else if (pt == ptr2[1])
					{
						p2 = ptr2[2];
						p3 = ptr2[0];
					}
					else
					{
						p2 = ptr2[0];
						p3 = ptr2[1];
					}

					if (p1 == p2)
					{
						p2 = p3;
					}
					else if (p0 == p3)
					{
						p4 = p1;
						p1 = p0;
						p0 = p2;
						p2 = p4;
					}
					else
						goto Next_point_label;

					if ((p0 == p1) || (p0 == p2) || (p1 == p2))
						goto Next_point_label;

					v1 = TabPoints[p0] - center;
					v1.Normalize();
					v2 = TabPoints[p2] - center;
					Vec3f delta = v2 - v1 * (v1 * v2);

					if (delta.GetNorm2() < (2.0e-2f * 2.0e-2f))
					{
						if (pt != ptr[0])
							m_TabPointsLinks[ptr[0]].RemoveFace(numFace0);

						if (pt != ptr[1])
							m_TabPointsLinks[ptr[1]].RemoveFace(numFace0);

						if (pt != ptr[2])
							m_TabPointsLinks[ptr[2]].RemoveFace(numFace0);

						ptr[0] = p0;
						ptr[1] = p1;
						ptr[2] = p2;
						ptr[3] = p2;

						m_TabPointsLinks[p0].AddFace(numFace0);
						m_TabPointsLinks[p1].AddFace(numFace0);
						m_TabPointsLinks[p2].AddFace(numFace0);

						if (_ignoreVirtual)
							pFaceIdx[numFace0].IsVirtual = FALSE;

						if (_ignoreExternal)
							pFaceIdx[numFace0].IsExternal = FALSE;

						v1 = (TabPoints[p1] - TabPoints[p0]) ^ (TabPoints[p2] - TabPoints[p0]);
						Float len2 = v1.GetNorm2();
						if (len2 < (1.0e-6f * 1.0e-6f))
							pFaces[numFace0] = 126;
						else
							pFaceNormals[numFace0] = v1 * InvSqrt(1.0f, len2);		// INVSQRT

						if (pt != ptr2[0])
							m_TabPointsLinks[ptr2[0]].RemoveFace(numFace1);

						if (pt != ptr2[1])
							m_TabPointsLinks[ptr2[1]].RemoveFace(numFace1);

						if (pt != ptr2[2])
							m_TabPointsLinks[ptr2[2]].RemoveFace(numFace1);

						pFaces[numFace1] = 127;
						pPtsTab[pt] = 1;
					}
				}
			}
		Next_point_label:
			;
		}
	}

	HS32DA redir;
	redir.SetSize(nbPoints, TRUE);
	S32* pRedir = redir.GetArrayPtr();

	S32 idx1 = 0;
	S32 idx2 = nbPoints - 1;
	while (idx1 < idx2)
	{
		if (pPtsTab[idx1])			// if point 'idx1' is removed
		{
			while ((idx1 < idx2) && pPtsTab[idx2])		// get the last non removed point
				idx2--;

			if (idx1 == idx2)
				break;

			TabPoints[idx1] = TabPoints[idx2];					// and fills the hole
			pRedir[idx2] = idx1;
			idx2--;
		}
		else
			pRedir[idx1] = idx1;

		idx1++;
	}

	if (!pPtsTab[idx1])
	{
		pRedir[idx1] = idx1;
		idx1++;
	}

	S32 nbPoints2 = idx1;
	idx1 = 0;
	idx2 = nbFaces - 1;
	while (idx1 < idx2)
	{
		ptr = pFaceIdx[idx1].TabPoints;
		EXCEPTIONC_Z((ptr[0] < nbPoints) && (ptr[1] < nbPoints) && (ptr[2] < nbPoints), "BIM !!!!!");

		if (pFaces[idx1] == 127)						// if face 'idx1' is removed
		{
			while ((idx1 < idx2) && (pFaces[idx2] == 127))		// get the last non removed face
				idx2--;

			if (idx1 == idx2)
				break;

			ptr2 = pFaceIdx[idx2].TabPoints;
			EXCEPTIONC_Z((ptr2[0] < nbPoints) && (ptr2[1] < nbPoints) && (ptr2[2] < nbPoints), "BIM !!!!!");
			EXCEPTIONC_Z((pRedir[ptr2[0]] >= 0) && (pRedir[ptr2[0]] < nbPoints2) && (pRedir[ptr2[1]] >= 0) && (pRedir[ptr2[1]] < nbPoints2) && (pRedir[ptr2[2]] >= 0) && (pRedir[ptr2[2]] < nbPoints2), "BIM dans les points !!!!");

			ptr[0] = pRedir[ptr2[0]];
			ptr[1] = pRedir[ptr2[1]];
			ptr[2] = pRedir[ptr2[2]];
			ptr[3] = ptr[2];

			pFaceIdx[idx1].IsVirtual = pFaceIdx[idx2].IsVirtual;
			pFaceIdx[idx1].IsExternal = pFaceIdx[idx2].IsExternal;
			idx2--;
		}
		else
		{
			EXCEPTIONC_Z((pRedir[ptr[0]] >= 0) && (pRedir[ptr[0]] < nbPoints2) && (pRedir[ptr[1]] >= 0) && (pRedir[ptr[1]] < nbPoints2) && (pRedir[ptr[2]] >= 0) && (pRedir[ptr[2]] < nbPoints2), "BIM dans les points !!!!");

			ptr[0] = pRedir[ptr[0]];
			ptr[1] = pRedir[ptr[1]];
			ptr[2] = pRedir[ptr[2]];
			ptr[3] = ptr[2];
		}

		idx1++;
	}

	if (pFaces[idx1] != 127)
	{
		ptr = pFaceIdx[idx1].TabPoints;
		EXCEPTIONC_Z((ptr[0] < nbPoints) && (ptr[1] < nbPoints) && (ptr[2] < nbPoints), "BIM !!!!!");
		EXCEPTIONC_Z((pRedir[ptr[0]] >= 0) && (pRedir[ptr[0]] < nbPoints2) && (pRedir[ptr[1]] >= 0) && (pRedir[ptr[1]] < nbPoints2) && (pRedir[ptr[2]] >= 0) && (pRedir[ptr[2]] < nbPoints2), "BIM dans les points !!!!");

		ptr[0] = pRedir[ptr[0]];
		ptr[1] = pRedir[ptr[1]];
		ptr[2] = pRedir[ptr[2]];
		ptr[3] = ptr[2];

		idx1++;
	}

	nbFaces = idx1;

	m_TabPoints.SetSize(nbPoints2);
	m_TabQuad.SetSize(nbFaces);

	m_QuadTreePt.SetSize(MESH_QUADTREE_TOTAL_SIZE);
	memset(m_QuadTreePt.GetArrayPtr(), 0, MESH_QUADTREE_TOTAL_SIZE*sizeof(Playspace_Mesh::RefPoint*));

	for (S32 i = 0; i<m_TabPoints.GetSize(); i++)
		InsertQuadTreePoint(i, m_TabPoints[i]);

	InvalidatePointsLinks();
	return;
}

/**************************************************************************/

void	Playspace_Mesh::SimplifyTri(Bool _simplifyExternal /*= TRUE*/)
{
	InvalidateFaceToolNormal();
	InvalidatePointsToolNormal();
	S32 NbPass = 0;

	// Compute Face infos.
	ComputePointsLinks();
	Float ErrorDot = Cos(DegToRad(8.f));
	Float cosAngleMin2 = Cos(DegToRad(8.f)) * Cos(DegToRad(8.f));	// if the collapse would generate a triangle with an angle too small (cos too big) THEN do not collapse

	Bool SomethingToDo = TRUE;
	while (SomethingToDo)
	{
		SomethingToDo = FALSE;
		NbPass++;

		for (S32 p0 = 0; p0 < m_TabPoints.GetSize(); p0++)
		{
			S32 NbLinkFacesP0 = m_TabPointsLinks[p0].GetNbFaces();
			if (!NbLinkFacesP0)
				continue;
			if (NbLinkFacesP0 > 10)
				continue;

			Vec3f	NormalP0;
			Float	ErrorP0 = ComputePointNormal(p0, NormalP0);
			if (ErrorP0 > ErrorDot)
			{
				for (S32 j = 0; j < NbLinkFacesP0; j++)
				{
					S32 numF = m_TabPointsLinks[p0].GetFace(j);

					Playspace_Mesh::Face &CurFace = m_TabQuad[numF];
					for (S32 k = 0; k < 3; k++)
					{
						if (CurFace.TabPoints[k] <= p0)
							continue;
						S32 p1 = CurFace.TabPoints[k];
						S32 NbLinkFacesP1 = m_TabPointsLinks[p1].GetNbFaces();
						if ((NbLinkFacesP1 + NbLinkFacesP0) > 16)
							continue;

						Vec3f	NormalP1;
						Float	ErrorP1 = ComputePointNormal(p1, NormalP1);

						if ((ErrorP1 > ErrorDot) && ((NormalP0*NormalP1) > ErrorDot))
						{
							if (IsCollapsableEdgeOnTri(p0, p1, cosAngleMin2,!_simplifyExternal))		// CF. comments on cosAngleMin2
							{
								ColapseEdge(p0, p1);
								SomethingToDo = TRUE;
								p0--;
								goto NEXT_POINT;
							}
						}
					}
				}
			}
		NEXT_POINT:
			;
		}
	}
	
	CheckValidity();
}

/**************************************************************************/

S32 Playspace_Mesh::GetNextPoint(S32 center, S32 point)
{
	Playspace_Mesh::PointsLinks &PtLinks = m_TabPointsLinks[center];
	S32 NbLinkFaces = PtLinks.GetNbFaces();

	for (S32 j = 0; j < NbLinkFaces; j++)
	{
		S32 NumFace = PtLinks.GetFace(j);
		Playspace_Mesh::Face& face = m_TabQuad[NumFace];

		if ((face.TabPoints[0] == center) && (face.TabPoints[1] == point))
			return face.TabPoints[2];
		else if ((face.TabPoints[1] == center) && (face.TabPoints[2] == point))
			return face.TabPoints[0];
		else if ((face.TabPoints[2] == center) && (face.TabPoints[0] == point))
			return face.TabPoints[1];
	}

	return -1;
}

S32 Playspace_Mesh::GetPointNeighbours(S32 pointIdx, S32* neighbours, S32 maxNeighbours, Bool* pIsOnBorder)
{
	Playspace_Mesh::PointsLinks &PtLinks = m_TabPointsLinks[pointIdx];
	S32 pt, lastPoint, nb = 0;

	S32 NumFace = PtLinks.GetFace(0);								// take the first face
	if (pointIdx == m_TabQuad[NumFace].TabPoints[0])
	{
		lastPoint = m_TabQuad[NumFace].TabPoints[1];
		pt = m_TabQuad[NumFace].TabPoints[2];
	}
	else if (pointIdx == m_TabQuad[NumFace].TabPoints[1])
	{
		lastPoint = m_TabQuad[NumFace].TabPoints[2];
		pt = m_TabQuad[NumFace].TabPoints[0];
	}
	else
	{
		lastPoint = m_TabQuad[NumFace].TabPoints[0];
		pt = m_TabQuad[NumFace].TabPoints[1];
	}

	neighbours[nb++] = lastPoint;
	while ((pt != -1) && (pt != lastPoint))
	{
		if (nb >= maxNeighbours)		// EXCEPTIONC_Z(nb < maxNeighbours, "Too many neighbours !!!!");
			return -1;
		
		neighbours[nb++] = pt;
		pt = GetNextPoint(pointIdx, pt);
	}

	if (pIsOnBorder)
	{
		if (pt == -1)
			*pIsOnBorder = TRUE;
		else
			*pIsOnBorder = FALSE;
	}

	return nb;
}

void Playspace_Mesh::DecomposePolyedraToTriangles(S32* points, S32 nbPts, S32* output, S32& nbGenTri)
{
	if (nbPts < 3)
		return;

	if (nbPts == 3)
	{
		nbGenTri = 1;
		for (S32 i = 0; i < 3; i++)
			output[i] = points[i];

		return;
	}

	S32 nbDiags, nbTri, nbTri2;
	S32 pts[32];
	S32 dec = 0, opposite = nbPts >> 1;
	Float dmin = 1e8f;

	for (S32 i = 0; i < nbPts; i++)
		pts[nbPts + i] = pts[i] = points[i];

	if (nbPts & 1)
		nbDiags = nbPts;
	else
		nbDiags = opposite;

	for (S32 i = 0; i < nbDiags; i++)
	{
		Float d = (m_TabPoints[pts[i]] - m_TabPoints[pts[i + opposite]]).GetNorm2();
		if (d < dmin)
		{
			dec = i;
			dmin = d;
		}
	}

	// here, we will split the polyedra in two parts:
	// pts[dec]..pts[dec+opposite]    which has opposite+1 vertices
	//
	// pts[dec+opposite]..pts[dec+nbVec]    which has nbVec-opposite+1 vertices

	DecomposePolyedraToTriangles(pts + dec, opposite + 1, output, nbTri);
	DecomposePolyedraToTriangles(pts + dec + opposite, 1 + nbPts - opposite, output + (nbTri * 3), nbTri2);
	nbGenTri = nbTri + nbTri2;
	return;
}

bool	Playspace_Mesh::IsPolyConvex(S32 nbPoints, S32* points)
{
	if (nbPoints <= 3)
		return TRUE;

	Vec3f v0 = m_TabPoints[points[0]] - m_TabPoints[points[nbPoints-1]];
	Vec3f v1 = m_TabPoints[points[1]] - m_TabPoints[points[0]];
	Vec3f normal = v0 ^ v1;
	Vec3f normal2;
	for (S32 p = 2; p < nbPoints; p++)
	{
		v0 = v1;
		v1 = m_TabPoints[points[p]] - m_TabPoints[points[p - 1]];
		normal2 = v0 ^ v1;
		if ((normal2 * normal) < 0.0f)
			return FALSE;
	}
	v0 = v1;
	v1 = m_TabPoints[points[0]] - m_TabPoints[points[nbPoints - 1]];
	normal2 = v0 ^ v1;
	if ((normal2 * normal) < 0.0f)
		return FALSE;

	return TRUE;
}

/**************************************************************************/

S32 Playspace_Mesh::GetNeighbourFaces(S32 face, S32* neighbours, Float* dotNormals, Float* areas, S32 _NbMax)
{
	static Bool	ExceptionLocal = TRUE;

	ComputePointsLinks();
	Playspace_Mesh::PointsLinks *TabPointsLinks = m_TabPointsLinks.GetArrayPtr();
	Playspace_Mesh::Face &CurFace = m_TabQuad[face];
	S32 nbFaces = (CurFace.IsTri ? 3 : 4);

	Vec3f fNormal = (m_TabPoints[CurFace.TabPoints[2]] - m_TabPoints[CurFace.TabPoints[0]]) ^ (m_TabPoints[CurFace.TabPoints[1]] - m_TabPoints[CurFace.TabPoints[3]]);
	fNormal.Normalize();
	S32 nb = 0;
	for (S32 i = 0; i < nbFaces; i++)
	{
		S32 i2 = (i < (nbFaces - 1)) ? i + 1 : 0;
		Playspace_Mesh::PointsLinks	&InfosP0 = TabPointsLinks[CurFace.TabPoints[i]];
		Playspace_Mesh::PointsLinks	&InfosP1 = TabPointsLinks[CurFace.TabPoints[i2]];
		S32 f0 = 0;
		S32 f1 = 0;
		while ((f0 < InfosP0.GetNbFaces()) && (f1 < InfosP1.GetNbFaces()))
		{
			S32 CurFace = InfosP0.GetFace(f0);
			S32 OtherFace = InfosP1.GetFace(f1);
			if (CurFace == OtherFace)
			{
				if (CurFace != face)
				{
					Bool alreadyPresent = FALSE;
					for (S32 j = 0; j < nb; j++)
					{
						if (neighbours[j] == CurFace)
						{
							alreadyPresent = TRUE;
							break;
						}
					}
					if (!alreadyPresent)
					{
#if !defined(_WINSTORE) && !defined(_USE_ONECORE)
						if (ExceptionLocal && (nb >= _NbMax))
						{
							ExceptionLocal = FALSE;
							CANCEL_EXCEPTIONC_Z(FALSE, "GetNeighbourFaces: Too many Neighbours");
						}
#endif
						if (nb >= _NbMax)
							return nb;

						neighbours[nb] = CurFace;
						Playspace_Mesh::Face &OtherFace = m_TabQuad[CurFace];
						Vec3f fNormal2 = (m_TabPoints[OtherFace.TabPoints[2]] - m_TabPoints[OtherFace.TabPoints[0]]) ^ (m_TabPoints[OtherFace.TabPoints[1]] - m_TabPoints[OtherFace.TabPoints[3]]);
						Float fArea2 = fNormal2.GetNorm();
						if (fArea2 > 1e-6f)
							fNormal2 *= 1.f / fArea2;

						fArea2 *= 0.5f;
						dotNormals[nb] = fNormal * fNormal2;
						areas[nb] = fArea2;
						nb++;
					}
				}
				f0++;
				f1++;
			}
			else if (CurFace < OtherFace)
				f0++;
			else
				f1++;
		}
	}
	return nb;
}

void Playspace_Mesh::CheckBadWinding()
{
	const int MaxNeighbourFaces = 12;
	Vec3f v[4], v2[4];
	Vec3f fNormal, fCenter, fNormal2;
	S32 nbBad, bad[MaxNeighbourFaces];
	S32 nbFaces = m_TabQuad.GetSize();

	ComputePointsLinks();
	Playspace_Mesh::PointsLinks *TabPointsLinks = m_TabPointsLinks.GetArrayPtr();
	S32 F[MaxNeighbourFaces], F2[MaxNeighbourFaces], F3[MaxNeighbourFaces];
	Float N[MaxNeighbourFaces], N2[MaxNeighbourFaces], N3[MaxNeighbourFaces], A[MaxNeighbourFaces], A2[MaxNeighbourFaces], A3[MaxNeighbourFaces];
	S32 nbNeighbours, nbNeighbours2, nbNeighbours3;
	Float fArea;
	for (S32 i = 0; i < nbFaces; i++)
	{
		Playspace_Mesh::Face &CurFace = m_TabQuad[i];
		v[0] = m_TabPoints[CurFace.TabPoints[0]];
		v[1] = m_TabPoints[CurFace.TabPoints[1]];
		v[2] = m_TabPoints[CurFace.TabPoints[2]];
		v[3] = m_TabPoints[CurFace.TabPoints[3]];

		fNormal = (v[2] - v[0]) ^ (v[1] - v[3]);		// Si Quad ou Tri => Aire est correcte
		fArea = fNormal.GetNorm();
		if (fArea > 1e-8f)
			fNormal *= 1.f / fArea;

		fArea *= 0.5f;
		if (CurFace.IsTri)
			fCenter = (v[0] + v[1] + v[2]) * 0.333333333f;
		else
			fCenter = (v[0] + v[1] + v[2] + v[3]) * 0.25f;

		if (!CurFace.IsTri)							// check des quads problématiques à trianguler (par exemple: les papillons planaires)
		{
			Vec3f Diag02 = v[2] - v[0];
			Vec3f Diag13 = v[3] - v[1];

			Float dot02 = (Diag02 ^ (v[1] - v[0])) * (Diag02 ^ (v[3] - v[0]));
			Float dot13 = (Diag13 ^ (v[0] - v[1])) * (Diag13 ^ (v[2] - v[1]));

			if ((dot02 >= 0.0f) && (dot13 >= 0.0f))			// là on a un problème...
			{
				Float col = g_StaticRandom.RandFromU32(i);
				col = fmodf(col, 1.0f);
				Color col1;
				col1.r = col * 6.0f;
				col1.g = 1.0f;
				col1.b = 1.0f;
				col1 = col1.FromHSVToRGB();
				col1.a = 1.0f;

				DRAW_DEBUG_FACE(v[0], v[1], v[2], col1, .displayDuration(3000.f));
				DRAW_DEBUG_FACE(v[0], v[2], v[3], col1, .displayDuration(3000.f));

				Vec3f center = fCenter + Vec3f(0.0f, 0.01f, 0.0f);

				DRAW_DEBUG_STRING3D_DURATION(center, 3000.0f, "Face %d (aire: %f) est un papillon", i, fArea);
			}
		}

		nbBad = 0;
		nbNeighbours = GetNeighbourFaces(i, F, N, A, MaxNeighbourFaces);
		for (S32 j = 0; j < nbNeighbours; j++)
		{
			Playspace_Mesh::Face &face = m_TabQuad[F[j]];
			if (N[j] < -0.98f)
			{
				Bool found;
				nbNeighbours2 = GetNeighbourFaces(F[j], F2, N2, A2, MaxNeighbourFaces);
				for (S32 k = 0; k < nbNeighbours2; k++)
				{
					if (N2[k] >= -0.98f)
					{
						nbNeighbours3 = GetNeighbourFaces(F2[k], F3, N3, A3, MaxNeighbourFaces);
						found = FALSE;
						for (S32 n = 0; n < nbNeighbours3; n++)
						{
							if (N3[n] < -0.98f)
							{
								found = TRUE;
								break;
							}
						}
						if (!found)
							break;
					}
				}
				if (!found)
				{
					bad[nbBad++] = F[j];
				}
			}
		}
		if (nbBad)
		{
			Float col = g_StaticRandom.RandFromU32(i);
			col = fmodf(col, 1.0f);
			Color col1;
			col1.r = col * 6.0f;
			col1.g = 1.0f;
			col1.b = 1.0f;
			col1 = col1.FromHSVToRGB();
			col1.a = 1.0f;
			Color col2 = Color(1.0f, 0.0f, 0.0f, 0.5f);
			if (nbBad == nbNeighbours)
			{
				//DRAW_DEBUG_SPHERE3D(TabFaceNormal[i].Center, COLOR_BLUE * 0.9f, 0.02f, .displayDuration(1000.f));
				DRAW_DEBUG_FACE(v[0], v[1], v[2], col2, .displayDuration(3000.f));
				if (!CurFace.IsTri)
				{
					DRAW_DEBUG_FACE(v[0], v[2], v[3], col2, .displayDuration(3000.f));
				}
			}
			else
			{
				//DRAW_DEBUG_SPHERE3D(TabFaceNormal[i].Center, COLOR_RED * 0.9f, 0.02f, .displayDuration(1000.f));
				DRAW_DEBUG_FACE(v[0], v[1], v[2], col1, .displayDuration(3000.f));
				if (!CurFace.IsTri)
				{
					DRAW_DEBUG_FACE(v[0], v[2], v[3], col1, .displayDuration(3000.f));
				}
			}
			Vec3f center = fCenter + Vec3f(0.0f, 0.01f, 0.0f);
			if (nbNeighbours == 4)
			{
				if (nbBad == 4)
				{
					DRAW_DEBUG_STRING3D_DURATION(center, 3000.0f, "Face %d (aire=%f vois: %d, %d, %d, %d bad: %d, %d, %d, %d)", i, fArea, F[0], F[1], F[2], F[3], bad[0], bad[1], bad[2], bad[3]);
				}
				else if (nbBad == 3)
				{
					DRAW_DEBUG_STRING3D_DURATION(center, 3000.0f, "Face %d (aire=%f vois: %d, %d, %d, %d bad: %d, %d, %d)", i, fArea, F[0], F[1], F[2], F[3], bad[0], bad[1], bad[2]);
				}
				else if (nbBad == 2)
				{
					DRAW_DEBUG_STRING3D_DURATION(center, 3000.0f, "Face %d (aire=%f vois: %d, %d, %d, %d bad: %d, %d)", i, fArea, F[0], F[1], F[2], F[3], bad[0], bad[1]);
				}
				else
				{
					DRAW_DEBUG_STRING3D_DURATION(center, 3000.0f, "Face %d (aire=%f vois: %d, %d, %d, %d bad: %d)", i, fArea, F[0], F[1], F[2], F[3], bad[0]);
				}
			}
			else
			{
				if (nbBad == 4)
				{
					DRAW_DEBUG_STRING3D_DURATION(center, 3000.0f, "Face %d (aire=%f vois: %d, %d, %d bad: %d, %d, %d, %d)", i, fArea, F[0], F[1], F[2], bad[0], bad[1], bad[2], bad[3]);
				}
				else if (nbBad == 3)
				{
					DRAW_DEBUG_STRING3D_DURATION(center, 3000.0f, "Face %d (aire=%f vois: %d, %d, %d bad: %d, %d, %d)", i, fArea, F[0], F[1], F[2], bad[0], bad[1], bad[2]);
				}
				else if (nbBad == 2)
				{
					DRAW_DEBUG_STRING3D_DURATION(center, 3000.0f, "Face %d (aire=%f vois: %d, %d, %d bad: %d, %d)", i, fArea, F[0], F[1], F[2], bad[0], bad[1]);
				}
				else
				{
					DRAW_DEBUG_STRING3D_DURATION(center, 3000.0f, "Face %d (aire=%f vois: %d, %d, %d bad: %d)", i, fArea, F[0], F[1], F[2], bad[0]);
				}
			}
		}
	}
}

void Playspace_Mesh::RemoveBadFaces()
{
	const int MaxNeighbourFaces = 32;

	Float cosErr2 = Cos(DegToRad(2.0f));
	cosErr2 = cosErr2 * cosErr2;

	Vec3f v[4], v2[4];
	Vec3f fNormal, fCenter, fNormal2;
	S32 nbFaces = m_TabQuad.GetSize();

	ComputePointsLinks();
	Playspace_Mesh::PointsLinks *TabPointsLinks = m_TabPointsLinks.GetArrayPtr();
	S32 F[MaxNeighbourFaces], F2[MaxNeighbourFaces], F3[MaxNeighbourFaces];
	Float N[MaxNeighbourFaces], N2[MaxNeighbourFaces], N3[MaxNeighbourFaces], A[MaxNeighbourFaces], A2[MaxNeighbourFaces], A3[MaxNeighbourFaces];
	S32 nbNeighbours, nbNeighbours2, nbNeighbours3, nb, nb2;
	Float fArea;
	for (S32 pass = 0; pass < 2; pass++)
	{
		for (S32 i = m_TabQuad.GetSize() - 1; i >= 0; i--)
		{
			Playspace_Mesh::Face &CurFace = m_TabQuad[i];
			v[0] = m_TabPoints[CurFace.TabPoints[0]];
			v[1] = m_TabPoints[CurFace.TabPoints[1]];
			v[2] = m_TabPoints[CurFace.TabPoints[2]];
			v[3] = m_TabPoints[CurFace.TabPoints[3]];
			fNormal = (v[2] - v[0]) ^ (v[1] - v[3]);		// Si Quad ou Tri => Aire est correcte
			fArea = fNormal.GetNorm();
			if (fArea > 1e-8f)
				fNormal *= 1.f / fArea;

			fArea *= 0.5f;
			if (CurFace.IsTri)
				fCenter = (v[0] + v[1] + v[2]) * 0.333333333f;
			else
				fCenter = (v[0] + v[1] + v[2] + v[3]) * 0.25f;

			nb = (CurFace.IsTri ? 3 : 4);
			nbNeighbours = GetNeighbourFaces(i, F, N, A, MaxNeighbourFaces);
			for (S32 j = 0; j < nbNeighbours; j++)
			{
				Playspace_Mesh::Face &face = m_TabQuad[F[j]];
				if (N[j] < -0.98f)
				{
					Bool found;
					nbNeighbours2 = GetNeighbourFaces(F[j], F2, N2, A2, MaxNeighbourFaces);
					for (S32 k = 0; k < nbNeighbours2; k++)
					{
						if (N2[k] >= -0.98f)
						{
							nbNeighbours3 = GetNeighbourFaces(F2[k], F3, N3, A3, MaxNeighbourFaces);
							found = FALSE;
							for (S32 n = 0; n < nbNeighbours3; n++)
							{
								if (N3[n] < -0.98f)
								{
									found = TRUE;
									break;
								}
							}
							if (!found)
								break;
						}
					}
					if (!found /*&& (fArea < A[j])*/)
					{
						S32 p1, p2, p3, p4, q1, q2, q3;
						Bool found = FALSE;
						nb2 = (face.IsTri ? 3 : 4);
						for (S32 p = 0; p < nb; p++)
						{
							for (S32 q = 0; q < nb2; q++)
							{
								if (!found && (CurFace.TabPoints[p] == face.TabPoints[q]))
								{
									found = TRUE;
									p1 = p;
									q1 = q;
								}
							}
						}
						p2 = ((p1 == nb - 1) ? 0 : p1 + 1);
						p3 = ((p1 == 0) ? nb - 1 : p1 - 1);
						p4 = ((p2 == nb - 1) ? 0 : p2 + 1);
						q2 = ((q1 == 0) ? nb2 - 1 : q1 - 1);
						q3 = ((q1 == nb2 - 1) ? 0 : q1 + 1);
						if ((CurFace.TabPoints[p2] == face.TabPoints[q2]) || (CurFace.TabPoints[p2] == face.TabPoints[q3]))
						{
							Float d1 = (v[p1] - v[p3]).GetNorm2();
							Float d2 = (v[p2] - v[p4]).GetNorm2();

							Bool ok = FALSE;
							Bool c13 = IsCollapsableEdgeOnTri(CurFace.TabPoints[p1], CurFace.TabPoints[p3], cosErr2);
							Bool c24 = IsCollapsableEdgeOnTri(CurFace.TabPoints[p2], CurFace.TabPoints[p4], cosErr2);

							if (c13 && c24)
							{
								ok = TRUE;
								if (d1 < d2)
									ColapseEdge(CurFace.TabPoints[p1], CurFace.TabPoints[p3]);
								else
									ColapseEdge(CurFace.TabPoints[p2], CurFace.TabPoints[p4]);
							}
							else if (c13)
							{
								ok = TRUE;
								ColapseEdge(CurFace.TabPoints[p1], CurFace.TabPoints[p3]);
							}
							else if (c24)
							{
								ok = TRUE;
								ColapseEdge(CurFace.TabPoints[p2], CurFace.TabPoints[p4]);
							}

							if (ok)
							{
								i++;
								if (i > m_TabQuad.GetSize())
									i = m_TabQuad.GetSize();

								break;
							}
						}
					}
				}
			}
		}
	}
}

void Playspace_Mesh::Triangularize(bool flipWindingOrder)
{
	S32 nbFaces = m_TabQuad.GetSize();

	S32 idx0, idx1, idx2, idx3;
	Vec3f p0, p1, p2, p3;
	Vec3f Diag02, Diag13;

	for (S32 faceIdx = 0; faceIdx < nbFaces; faceIdx++)
	{
		Playspace_Mesh::Face *pCurFace = &m_TabQuad[faceIdx];

		if (!pCurFace->IsTri)
		{
			idx0 = pCurFace->TabPoints[0];
			idx1 = pCurFace->TabPoints[1];
			idx2 = pCurFace->TabPoints[2];
			idx3 = pCurFace->TabPoints[3];

			p0 = m_TabPoints[idx0];
			p1 = m_TabPoints[idx1];
			p2 = m_TabPoints[idx2];
			p3 = m_TabPoints[idx3];

			Diag02 = p2 - p0;
			Diag13 = p3 - p1;

			Float dot02 = (Diag02 ^ (p1 - p0)) * (Diag02 ^ (p3 - p0));
			Float dot13 = (Diag13 ^ (p0 - p1)) * (Diag13 ^ (p2 - p1));

			if ((dot02 >= 0.0f) && (dot13 >= 0.0f))
			{
				idx0++;
				if (idx0)
					idx0--;
			}

			if (dot02 < 0.0f)	// (Diag02.GetNorm2() < Diag13.GetNorm2())
			{
				pCurFace->TabPoints[3] = idx2;		// CurFace : (p0 p1 p2)
				pCurFace->IsTri = TRUE;

				// Add Face
				S32 idxNewFace = m_TabQuad.Add();
				Playspace_Mesh::Face &newFace = m_TabQuad[idxNewFace];
				newFace.IsTri = TRUE;
				pCurFace = &m_TabQuad[faceIdx];	// Reset pointer to cur face to prevent Dynarray Grow.
				newFace.IsVirtual = pCurFace->IsVirtual;
				newFace.IsExternal = pCurFace->IsExternal;
				newFace.IsPaintMode = pCurFace->IsPaintMode;
				newFace.IsSeenQuality = pCurFace->IsSeenQuality;
				newFace.TabPoints[0] = idx0;		// newFace : (p0 p2 p3)
				newFace.TabPoints[1] = idx2;
				newFace.TabPoints[2] = idx3;
				newFace.TabPoints[3] = idx3;
			}
			else
			{
				pCurFace->TabPoints[2] = idx3;		// CurFace : (p0 p1 p3)
				pCurFace->IsTri = TRUE;

				// Add Face
				S32 idxNewFace = m_TabQuad.Add();
				Playspace_Mesh::Face &newFace = m_TabQuad[idxNewFace];
				newFace.IsTri = TRUE;
				pCurFace = &m_TabQuad[faceIdx];	// Reset pointer to cur face to prevent Dynarray Grow.
				newFace.IsVirtual = pCurFace->IsVirtual;
				newFace.IsExternal = pCurFace->IsExternal;
				newFace.IsPaintMode = pCurFace->IsPaintMode;
				newFace.IsSeenQuality = pCurFace->IsSeenQuality;
				newFace.TabPoints[0] = idx1;		// newFace : (p1 p2 p3)
				newFace.TabPoints[1] = idx2;
				newFace.TabPoints[2] = idx3;
				newFace.TabPoints[3] = idx3;
			}
		}
	}

	// Optionally flip winding order...
	if (flipWindingOrder)
	{
		nbFaces = m_TabQuad.GetSize();

		for (S32 faceIdx = 0; faceIdx < nbFaces; faceIdx++)
		{
			Playspace_Mesh::Face *pCurFace = &m_TabQuad[faceIdx];

			idx1 = pCurFace->TabPoints[1];
			idx2 = pCurFace->TabPoints[2];

			pCurFace->TabPoints[1] = idx2;
			pCurFace->TabPoints[2] = idx1;
		}
	}

	InvalidateFaceToolNormal();
	InvalidatePointsToolNormal();
	InvalidatePointsLinks();
	//CheckValidity();
}


void Playspace_Mesh::DetectFacesOnBoundaries()
{
	m_TabEdgyQuad.Flush();

	ComputePointsLinks();

	S32 NbPt = m_TabPoints.GetSize();

	for (S32 i = 0; i < NbPt; i++)
	{
		S32 NbLinkFaces = m_TabPointsLinks[i].GetNbFaces();
		for (S32 j = 0; j < NbLinkFaces; j++)
		{
			S32 numF = m_TabPointsLinks[i].GetFace(j);

			Playspace_Mesh::Face &CurFace = m_TabQuad[numF];
			S32	NbFacePt = 4;
			if (CurFace.IsTri)
				NbFacePt = 3;

			for (S32 k = 0; k<NbFacePt; k++)
			{
				if (CurFace.TabPoints[k] <= i)
					continue;
				if (NbFacesOnEdge(i, CurFace.TabPoints[k]) != 1)
					continue;

				//If we get here, then CurFace is a Face on a boundary edge
				// add it to m_TabEdgyQuad
				m_TabEdgyQuad.Add(CurFace);
			}
		}
	}
	
	MESSAGE_Z("--Found %d faces on boudary edges",m_TabEdgyQuad.GetSize());

}




void Playspace_Mesh::DetectFacesOnEdgeClusters()
{

	//create a local copy of m_TabEdgyQuad
	HugeDynArray_Z<Playspace_Mesh::Face, 1024, FALSE, FALSE>	l_TabEdgyQuad = m_TabEdgyQuad;

#define MAX_HOLE 1000
	m_TabHolesBoundaries.SetSize(MAX_HOLE); // for now, only MAX_HOLE holes are seeked
	S32 NbHolesFound = 0;
	S32 NbEdgyQuad = l_TabEdgyQuad.GetSize(); 
	HugeDynArray_Z<S32, 1024, FALSE, FALSE> tempArray;
	for(S32 i = 0 ; i < NbEdgyQuad ; i++)
	{
		if((i==NbEdgyQuad/10)||(i==2*NbEdgyQuad/10)||(i==3*NbEdgyQuad/10)||(i==4*NbEdgyQuad/10)
		||(i==5*NbEdgyQuad/10)||(i==6*NbEdgyQuad/10)||(i==7*NbEdgyQuad/10)||(i==8*NbEdgyQuad/10)
		||(i==9*NbEdgyQuad/10))
		{
			MESSAGE_Z("Done... %.3f",(float)i/(float)NbEdgyQuad);
		}

		tempArray.Flush();
		//add the face index in tempArray
		tempArray.Add(i);
		//find neighboor faces and add their 
		for(S32 j = 0 ; j < NbEdgyQuad ; j++)
		{
			for(S32 k = 0 ; k < tempArray.GetSize() ; k++)
			{
				Bool bfound=FALSE;
				if(AreFacesConnected(l_TabEdgyQuad[tempArray[k]],l_TabEdgyQuad[j]))
				{
					tempArray.Add(j);
					bfound=TRUE; // no need to test all faces in tempArray
				}
				if(bfound)
					break;
			}
		}
		// tempArray now contains the index of all faces in a graph containing l_TabEdgyQuad[i]
		if(tempArray.GetSize() >= 20)
		{
			// copy it in m_TabHolesBoundaries and remove the corresponding faces from l_TabEdgyQuad
			for (S32 m = 0;m<tempArray.GetSize() ;m++)
			{
				m_TabHolesBoundaries[NbHolesFound].Add(l_TabEdgyQuad[tempArray[m]]);
			}
			for (S32 m = tempArray.GetSize()-1;m<=0 ;m--)
			{
				l_TabEdgyQuad.Remove(tempArray[m]);
				NbEdgyQuad--;
			}
			NbHolesFound++;

		}
		if(NbHolesFound>=MAX_HOLE)
			break;
	}

	m_TabHolesBoundaries.SetSize(NbHolesFound);

}




void Playspace_Mesh::DetectHolesBoundaries()
{

	//create a local copy of m_TabEdgyQuad
	HugeDynArray_Z<Playspace_Mesh::Face, 1024, FALSE, FALSE>	l_TabEdgyQuad = m_TabEdgyQuad;

#define MAX_HOLE 1000
	m_TabHolesBoundaries.SetSize(MAX_HOLE); // for now, only MAX_HOLE holes are seeked
	S32 NbHolesFound = 0;
	S32 NbEdgyQuad = l_TabEdgyQuad.GetSize(); 
	HugeDynArray_Z<S32, 1024, FALSE, FALSE> tempArray;
	for(S32 i = 0 ; i < NbEdgyQuad ; i++)
	{
		if((i==NbEdgyQuad/10)||(i==2*NbEdgyQuad/10)||(i==3*NbEdgyQuad/10)||(i==4*NbEdgyQuad/10)
		||(i==5*NbEdgyQuad/10)||(i==6*NbEdgyQuad/10)||(i==7*NbEdgyQuad/10)||(i==8*NbEdgyQuad/10)
		||(i==9*NbEdgyQuad/10))
		{
			MESSAGE_Z("Done... %.3f",(float)i/(float)NbEdgyQuad);
		}

		tempArray.Flush();
		//add the face index in tempArray
		tempArray.Add(i);
		//for the other edges, if connected to one face in the array, add in the corresponding index in the array
		for(S32 j = 0 ; j < NbEdgyQuad ; j++)
		{
			for(S32 k = 0 ; k < tempArray.GetSize() ; k++)
			{
				Bool bfound=FALSE;
				if(AreFacesConnected(l_TabEdgyQuad[tempArray[k]],l_TabEdgyQuad[j]))
				{
					tempArray.Add(j);
					bfound=TRUE; // no need to test all faces in tempArray
				}
				if(bfound)
					break;
			}
		}
		// tempArray now contains the index of all faces in a graph containing l_TabEdgyQuad[i]
		if(tempArray.GetSize() >= 20)
		{
			// copy it in m_TabHolesBoundaries and remove the corresponding faces from l_TabEdgyQuad
			for (S32 m = 0;m<tempArray.GetSize() ;m++)
			{
				m_TabHolesBoundaries[NbHolesFound].Add(l_TabEdgyQuad[tempArray[m]]);
			}
			for (S32 m = tempArray.GetSize()-1;m<=0 ;m--)
			{
				l_TabEdgyQuad.Remove(tempArray[m]);
				NbEdgyQuad--;
			}
			NbHolesFound++;

		}
		if(NbHolesFound>=MAX_HOLE)
			break;
	}

	m_TabHolesBoundaries.SetSize(NbHolesFound);

}

Bool Playspace_Mesh::AreFacesConnected(Playspace_Mesh::Face F1,Playspace_Mesh::Face F2)
{
	// Faces are connected if exactly two points are in common 
	//(if more than three points, faces are identical)
	S32 NpPtsF1 = (F1.IsTri) ? 3 : 4;
	S32 NpPtsF2 = (F2.IsTri) ? 3 : 4;
	S32 NpPtsShared = 0;

	for(S32 i=0;i<NpPtsF1;i++)
	{
		for(S32 j=0;j<NpPtsF2;j++)
		{
			if(F1.TabPoints[i] == F2.TabPoints[j])
				NpPtsShared++;
		}
	}
	return ((NpPtsShared == 2)||(NpPtsShared == 1));
}

/**************************************************************************/
static S32 NbDuplicateFace = 0;
void	Playspace_Mesh::CheckDuplicateFaces()
{
	S32 nbFace = m_TabQuad.GetSize();
	for(S32 i=0; i<nbFace; i++)
	{
		Playspace_Mesh::Face &CurFace = m_TabQuad[i];
		for(S32 j=i+1; j<nbFace; j++)
		{
			Playspace_Mesh::Face &CurFace2 = m_TabQuad[j];
			if(((CurFace.TabPoints[0]==CurFace2.TabPoints[0])||(CurFace.TabPoints[0]==CurFace2.TabPoints[2])||(CurFace.TabPoints[0]==CurFace2.TabPoints[1]))
				&& ((CurFace.TabPoints[1]==CurFace2.TabPoints[0])||(CurFace.TabPoints[1]==CurFace2.TabPoints[2])||(CurFace.TabPoints[1]==CurFace2.TabPoints[1]))
				&& ((CurFace.TabPoints[2]==CurFace2.TabPoints[0])||(CurFace.TabPoints[2]==CurFace2.TabPoints[2])||(CurFace.TabPoints[2]==CurFace2.TabPoints[1])))
			{
				OUTPUT_Z("Duplicate faces %d %d : %d %d %d",i,j,CurFace.TabPoints[0],CurFace.TabPoints[1] ,CurFace.TabPoints[2] ); 
				NbDuplicateFace++;
			}
		}
	}
	OUTPUT_Z("Nb duplicate face: %d",NbDuplicateFace);
}


void Playspace_Mesh::CheckNonManifoldFaces()
{

	S32 nbFace = m_TabQuad.GetSize();
	S32 nbNon2ManifoldEdges =0;
	for(S32 i=0; i<nbFace; i++)
	{
		Playspace_Mesh::Face &CurFace = m_TabQuad[i];
		S32 NbFaceAdjOnEdge1 = NbFacesOnEdge(CurFace.TabPoints[0],CurFace.TabPoints[1]);
		S32 NbFaceAdjOnEdge2 = NbFacesOnEdge(CurFace.TabPoints[1],CurFace.TabPoints[2]);
		S32 NbFaceAdjOnEdge3 = NbFacesOnEdge(CurFace.TabPoints[0],CurFace.TabPoints[2]);
		if((NbFaceAdjOnEdge1 > 2) || (NbFaceAdjOnEdge2 > 2) ||  ( NbFaceAdjOnEdge3 > 2)	)
		{
			MESSAGE_Z("Non 2-manifold face found %d %d %d",NbFaceAdjOnEdge1,NbFaceAdjOnEdge2,NbFaceAdjOnEdge3);
			nbNon2ManifoldEdges ++;
		}
	}
	MESSAGE_Z("Total number of non 2-manifold faces found: %d",nbNon2ManifoldEdges);
}
/**************************************************************************/

void	Playspace_Mesh::DetectHole()
{

	InvalidateFaceToolNormal();
	InvalidatePointsToolNormal();
	// Compute Face infos.
	ComputePointsLinks();


	// Search a Hole...
	S32 NbPt = m_TabPoints.GetSize();
	S32 MaxSize = -1;
	S32 NbHoleFilled = 0;
	S32 imem = 0;
	HugeDynArray_Z<S32, 1024, FALSE, FALSE> ForbiddenPoints;
	ForbiddenPoints.Empty();
U32DA TabHole;

Bool IsForced = FALSE;


	for(;;)
	{
		TabHole.Empty();

		for (S32 i = imem; i < NbPt; i++)
		{
			S32 NbForbidden = ForbiddenPoints.GetSize();
			Bool IsForbid = FALSE;
			for(S32 s=0;s<NbForbidden;s++)
			{
				if(i == ForbiddenPoints[s])
				{
					IsForbid = TRUE;
					break;
				}
			}
			if(IsForbid)
				continue;

			S32 NbLinkFaces = m_TabPointsLinks[i].GetNbFaces();
			for (S32 j = 0; j < NbLinkFaces; j++)
			{
				S32 numF = m_TabPointsLinks[i].GetFace(j);

				Playspace_Mesh::Face &CurFace = m_TabQuad[numF];
				S32	NbFacePt = 4;
				if (CurFace.IsTri)
					NbFacePt = 3;
				for (S32 k = 0; k<NbFacePt; k++)
				{
					if (CurFace.TabPoints[k] <= i)
						continue;
					if (NbFacesOnEdge(i, CurFace.TabPoints[k]) != 1)
						continue;

					TabHole.Add(i);
					TabHole.Add(CurFace.TabPoints[k]);
					break;
				}

				if (TabHole.GetSize())
					break;
			}
			if (TabHole.GetSize())
			{
				imem = i;
				imem++;
				break;
			}
		}

		// No Hole ?
		if (!TabHole.GetSize())
		{
			MESSAGE_Z("Nb Hole filled %d",NbHoleFilled);
			MESSAGE_Z("Max size of hole filled %d",MaxSize );
			if(!CheckValidity())
				MESSAGE_Z("Invalid Mesh !!!");
			return;
		}
		OUTPUT_Z("-------------------------------------------------");


		// Complete Hole list.
		for(;;)
		{
			S32 NbHPt = TabHole.GetSize();
			S32 p0 = TabHole[NbHPt - 2];
			S32 p1 = TabHole[NbHPt - 1];
			if(p1 == p0)
			{
				MESSAGE_Z("Canceled hole boundary detection with size %d (p1==p0) %d",TabHole.GetSize(),p1);
				goto CANCEL_THIS_HOLE_SEARCH;
			}
			S32 p2 = -1;
			//OUTPUT_Z("--");
			//OUTPUT_Z("P0: %d  P1=%d",p0,p1);
			S32 NbFaces =  m_TabPointsLinks[p1].GetNbFaces();
			("Nb of faces linked to %d: %d",p1,NbFaces);
			for (S32 j = 0; j < NbFaces; j++)
			{
					
				S32 numF = m_TabPointsLinks[p1].GetFace(j);
				//OUTPUT_Z("Face  %d/%d, ref %d",j+1,NbFaces,numF);
				Playspace_Mesh::Face &CurFace = m_TabQuad[numF];
				S32	NbFacePt = 4;
				if (CurFace.IsTri)
					NbFacePt = 3;
				for (S32 k = 0; k<NbFacePt; k++)
				{
					//OUTPUT_Z("Nb face on edge %d<->%d: %d",p1,CurFace.TabPoints[k],NbFacesOnEdge(p1, CurFace.TabPoints[k]));
					if (CurFace.TabPoints[k] == p0)
						continue;
					if (NbFacesOnEdge(p1, CurFace.TabPoints[k]) != 1)
						continue;
					if (CurFace.TabPoints[k] == p1)
						continue;
					//OUTPUT_Z("Adding p2=%d",CurFace.TabPoints[k]);
					p2 = CurFace.TabPoints[k];
					break;
				}
				if (p2 >= 0)
					break;
			}

			//EXCEPTIONC_Z(p2 >= 0,"FillHole => impossible case");
			if(p2<0)
			{
				MESSAGE_Z("Canceled size %d [0]=%d",TabHole.GetSize(),TabHole[0]);
				//ForbiddenPoints.Add(TabHole[0]);
				//goto CANCEL_THIS_HOLE_SEARCH;
				IsForced = TRUE;
				goto TEST_FILL_ANYWAY;
			}
			if(TabHole.GetSize() >10000)
				goto CANCEL_THIS_HOLE_SEARCH;

			//original test -> stop when back at the beginning
			if (p2 == TabHole[0]) 
				break;

			//new test -> stop AND CANCEL if back at a previously encountered point
			//[ABCDEFGCBA] will be treated as [CDEFGC] during C's turn as TabHole[0]
			for(S32 ii=1;ii<TabHole.GetSize();ii++)
			{
				if(p2 == TabHole[ii])
				{
					//goto CANCEL_THIS_HOLE_SEARCH; 
					U32DA TabHoleTemp;
					for(S32 z=0;z<TabHole.GetSize()-ii;z++)
					{
						TabHole[z]=TabHole[ii+z];
					}
					TabHole.SetSize(TabHole.GetSize()-ii);
					goto TEST_FILL_ANYWAY;
				}
			}

			TabHole.Add(p2);
		}

		// Fill this Hole.

TEST_FILL_ANYWAY:
		;

		if(TabHole.GetSize()>MaxSize)
			MaxSize = TabHole.GetSize();
		NbHoleFilled++;
		while (TabHole.GetSize() > 2)
		{
			// Search closest dist.
			S32 NbHPt = TabHole.GetSize();
			Float	BestDist = 1e10f;
			S32		BestPoint = -1;

			for (S32 i = 0; i < NbHPt; i++)
			{
				S32		iNext = i + 2;
				if (iNext >= NbHPt)
					iNext -= NbHPt;

				Float	Dist = (m_TabPoints[TabHole[i]] - m_TabPoints[TabHole[iNext]]).GetNorm2();
				if (Dist < BestDist)
				{
					BestDist = Dist;
					BestPoint = i;
				}
			}

			// Do not create gigantic face when the current hole has a non-closed boundary
			if(IsForced && (BestDist > 1))
				break;

			// Add Face !
			S32 NumFace = m_TabQuad.Add();
			Playspace_Mesh::Face &CurFace = m_TabQuad[NumFace];


			CurFace.IsTri = TRUE;
			CurFace.IsVirtual = TRUE;
			CurFace.TabPoints[0] = TabHole[BestPoint];
			BestPoint++;
			if (BestPoint >= NbHPt)	BestPoint -= NbHPt;
			S32 iSuppressPoint = BestPoint;
			CurFace.TabPoints[1] = TabHole[BestPoint];
			BestPoint++;
			if (BestPoint >= NbHPt)	BestPoint -= NbHPt;
			CurFace.TabPoints[2] = TabHole[BestPoint];
			CurFace.TabPoints[3] = TabHole[BestPoint];

			//Add Links.
			m_TabPointsLinks[CurFace.TabPoints[0]].AddFace(NumFace);
			m_TabPointsLinks[CurFace.TabPoints[1]].AddFace(NumFace);
			m_TabPointsLinks[CurFace.TabPoints[2]].AddFace(NumFace);

			// Suppress point.
			TabHole.Remove(iSuppressPoint);
		}
		IsForced = FALSE;
CANCEL_THIS_HOLE_SEARCH:
		;

	}

}


/**************************************************************************/
void	Playspace_Mesh::DetectHole2()
{

	InvalidateFaceToolNormal();
	InvalidatePointsToolNormal();
	// Compute Face infos.
	ComputePointsLinks();

	// Search a Hole...
	S32 NbPt = m_TabPoints.GetSize();

	S32 MaxSize = -1;
	 S32 NbHoleFilled = 0;
	S32 imem = 0;

	U32DA TabHole;

	Bool IsForced = FALSE;


	for(;;)
	{
		TabHole.Empty();

		for (S32 i = imem; i < NbPt; i++)
		{
			S32 NbLinkFaces = m_TabPointsLinks[i].GetNbFaces();
			for (S32 j = 0; j <NbLinkFaces; j++)
			{
				S32 numF = m_TabPointsLinks[i].GetFace(j);

				Playspace_Mesh::Face &CurFace = m_TabQuad[numF];
				S32	NbFacePt = 4;
				if (CurFace.IsTri)
					NbFacePt = 3;
				for (S32 k = 0; k<NbFacePt; k++)
				{
					if (CurFace.TabPoints[k] <= i)
						continue;
					if (NbFacesOnEdge(i, CurFace.TabPoints[k]) != 1)
						continue;

					TabHole.Add(i);
					TabHole.Add(CurFace.TabPoints[k]);
					break;
				}

				if (TabHole.GetSize())
					break;
			}
			if (TabHole.GetSize())
			{
				imem = i;
				imem++;
				break;
			}
		}

		// No Hole ?
		if (!TabHole.GetSize())
		{
			MESSAGE_Z("Nb Hole filled %d",NbHoleFilled);
			MESSAGE_Z("Max size of hole filled %d",MaxSize );
			return;
		}
		//OUTPUT_Z("-------------------------------------------------");


		// Complete Hole list.
		for(;;)
		{
			S32 NbHPt = TabHole.GetSize();
			S32 p0 = TabHole[NbHPt - 2];
			S32 p1 = TabHole[NbHPt - 1];
			if(p1 == p0)
			{
				MESSAGE_Z("Canceled hole boundary detection with size %d (p1==p0) %d",TabHole.GetSize(),p1);
				goto CANCEL_THIS_HOLE_SEARCH;
			}
			S32 p2 = -1;
			//OUTPUT_Z("--");
			//OUTPUT_Z("P0: %d  P1=%d",p0,p1);
			S32 NbFaces =  m_TabPointsLinks[p1].GetNbFaces();
			//("Nb of faces linked to %d: %d",p1,NbFaces);
			for (S32 j = 0; j < NbFaces; j++)
			{
					
				S32 numF = m_TabPointsLinks[p1].GetFace(j);
				//OUTPUT_Z("Face  %d/%d, ref %d",j+1,NbFaces,numF);
				Playspace_Mesh::Face &CurFace = m_TabQuad[numF];
				S32	NbFacePt = 4;
				if (CurFace.IsTri)
					NbFacePt = 3;
				for (S32 k = 0; k<NbFacePt; k++)
				{
					//OUTPUT_Z("Nb face on edge %d<->%d: %d",p1,CurFace.TabPoints[k],NbFacesOnEdge(p1, CurFace.TabPoints[k]));
					if (CurFace.TabPoints[k] == p0)
						continue;
					if (NbFacesOnEdge(p1, CurFace.TabPoints[k]) != 1)
						continue;
					if (CurFace.TabPoints[k] == p1)
						continue;
					//OUTPUT_Z("Adding p2=%d",CurFace.TabPoints[k]);
					p2 = CurFace.TabPoints[k];

					break;
				}
				if (p2 >= 0)
					break;
			}

			//EXCEPTIONC_Z(p2 >= 0,"FillHole => impossible case");
			if(p2<0)
			{
				MESSAGE_Z("Canceled size %d [0]=%d",TabHole.GetSize(),TabHole[0]);
				IsForced = TRUE;
				goto TEST_FILL_ANYWAY;
			}
			if(TabHole.GetSize() >10000)
				goto CANCEL_THIS_HOLE_SEARCH;

			//original test -> stop when back at the beginning
			if (p2 == TabHole[0]) 
				break;

			//new test -> stop AND CANCEL if back at a previously encountered point
			//[ABCDEFGCBA] will be treated as [CDEFGC] during C's turn as TabHole[0]
			for(S32 ii=1;ii<TabHole.GetSize();ii++)
			{
				if(p2 == TabHole[ii])
				{
					//goto CANCEL_THIS_HOLE_SEARCH; 
					U32DA TabHoleTemp;
					for(S32 z=0;z<TabHole.GetSize()-ii;z++)
					{
						TabHole[z]=TabHole[ii+z];
					}
					TabHole.SetSize(TabHole.GetSize()-ii);
					goto TEST_FILL_ANYWAY;
				}
			}

			TabHole.Add(p2);
		}

		// Fill this Hole.

TEST_FILL_ANYWAY:
		;
		if(IsForced)
		{
			S32 NbHPt = TabHole.GetSize();
			for (S32 i = 0; i < NbHPt; i++)
			{
				S32 NbFaces =  m_TabPointsLinks[TabHole[i]].GetNbFaces();
				for (S32 j = 0; j < NbFaces; j++)
				{
					S32 numF = m_TabPointsLinks[TabHole[i]].GetFace(j);
					Playspace_Mesh::Face &CurFace = m_TabQuad[numF];
				}
			}
		}
		else
		{
			S32 NbHPt = TabHole.GetSize();
			for (S32 i = 0; i < NbHPt; i++)
			{
				S32 NbFaces =  m_TabPointsLinks[TabHole[i]].GetNbFaces();
				for (S32 j = 0; j < NbFaces; j++)
				{
					S32 numF = m_TabPointsLinks[TabHole[i]].GetFace(j);
					Playspace_Mesh::Face &CurFace = m_TabQuad[numF];
				}
			}
		}
		IsForced = FALSE;
CANCEL_THIS_HOLE_SEARCH:
		;

	}

}

/**************************************************************************/

#define EXPORTMESH_OBJ_LINE_LEN 128
void Playspace_Mesh::ExportMesh_OBJ(char* filename, Bool _ExportVirtual)
{
	MESSAGE_Z("Exporting mesh to OBJ file: %s", filename);
	FILE* f;
	fopen_s(&f, filename, "wb+");
	String_Z <EXPORTMESH_OBJ_LINE_LEN> line;
	//writing vertices
	S32 NbPoints = m_TabPoints.GetSize();
	for (S32 i = 0; i<NbPoints; i++)
	{

		line.Sprintf("v %f %f %f\n", m_TabPoints[i].x, m_TabPoints[i].y, m_TabPoints[i].z);
		File_Write_String<EXPORTMESH_OBJ_LINE_LEN>(f, line);
	}

	line.Sprintf("# %d vertices\n", NbPoints);
	File_Write_String<EXPORTMESH_OBJ_LINE_LEN>(f, line);

	MESSAGE_Z("Nb vertices written %d", NbPoints);

	//writing faces (adding +1 to the ref in TabPoints because indexes in obj file are in the range [1...n]
	S32 NbFaces = m_TabQuad.GetSize();
	S32 NbTri = 0;
	S32 NbQuad = 0;
	for (S32 i = 0; i<NbFaces; i++)
	{
		if (_ExportVirtual || !m_TabQuad[i].IsVirtual)
		{
			if (m_TabQuad[i].IsTri)
			{
				line.Sprintf("f %d %d %d\n", m_TabQuad[i].TabPoints[2] + 1, m_TabQuad[i].TabPoints[1] + 1, m_TabQuad[i].TabPoints[0] + 1);
				NbTri++;
			}
			else
			{
				line.Sprintf("f %d %d %d %d\n", m_TabQuad[i].TabPoints[3] + 1, m_TabQuad[i].TabPoints[2] + 1, m_TabQuad[i].TabPoints[1] + 1, m_TabQuad[i].TabPoints[0] + 1);
				NbQuad++;
			}
		}
		File_Write_String<EXPORTMESH_OBJ_LINE_LEN>(f, line);
	}

	line.Sprintf("# %d quadrangles - %d triangles\n", NbQuad, NbTri);
	File_Write_String<EXPORTMESH_OBJ_LINE_LEN>(f, line);

	MESSAGE_Z("Nb faces written %d", NbFaces);
	fclose(f);
	MESSAGE_Z("Exporting mesh .... done");
}



/**************************************************************************/
void	Playspace_Mesh::FillHole()
{
	InvalidateFaceToolNormal();
	InvalidatePointsToolNormal();

	// Compute Face infos.
	ComputePointsLinks();

	// Search a Hole...
	S32 NbPt = m_TabPoints.GetSize();
	for (S32 num = 0; num < 10; num++)
	{
	for (S32 i = 0; i < NbPt; i++)
	{
		S32 NbLinkFacesP0 = m_TabPointsLinks[i].GetNbFaces();
		for (S32 j = 0; j < NbLinkFacesP0; j++)
		{
			S32 numF = m_TabPointsLinks[i].GetFace(j);

			Playspace_Mesh::Face &CurFace = m_TabQuad[numF];
			S32	NbFacePt = 4;
			if (CurFace.IsTri)
				NbFacePt = 3;
			for (S32 k = 0; k<NbFacePt; k++)
			{
				if (CurFace.TabPoints[k] <= i)
					continue;
				if (NbFacesOnEdge(i, CurFace.TabPoints[k]) != 1)
					continue;

				S32 p2 = CurFace.TabPoints[k];
				S32 NbLinkFacesP2 = m_TabPointsLinks[p2].GetNbFaces();
				for (S32 l = 0; l < NbLinkFacesP2; l++)
				{
					S32 numF2 = m_TabPointsLinks[p2].GetFace(l);

					Playspace_Mesh::Face &CurFace = m_TabQuad[numF2];
					S32	NbFacePt = 4;
					if (CurFace.IsTri)
						NbFacePt = 3;
					for (S32 m = 0; m<NbFacePt; m++)
					{
						if (CurFace.TabPoints[m] <= i)
							continue;
						if (NbFacesOnEdge(p2, CurFace.TabPoints[m]) != 1)
							continue;

						// Add Face !
						S32 p3 = CurFace.TabPoints[m];
						S32 NumFace = m_TabQuad.Add();
						Playspace_Mesh::Face &CurFace = m_TabQuad[NumFace];

						CurFace.IsTri = TRUE;
						CurFace.IsVirtual = TRUE;

						CurFace.TabPoints[0] = i;
						CurFace.TabPoints[1] = p2;
						CurFace.TabPoints[2] = p3;
						CurFace.TabPoints[3] = p3;

						//Add Links.
						m_TabPointsLinks[CurFace.TabPoints[0]].AddFace(NumFace);
						m_TabPointsLinks[CurFace.TabPoints[1]].AddFace(NumFace);
						m_TabPointsLinks[CurFace.TabPoints[2]].AddFace(NumFace);
						goto NEXT_TEST;
					}
				}
			}
		}
NEXT_TEST:
		;
	}
	}

	return;

	U32DA TabHole;
	for(;;)
	{
		TabHole.Empty();

		for (S32 i = 0; i < NbPt; i++)
		{
			S32 NbLinkFaces = m_TabPointsLinks[i].GetNbFaces();
			for (S32 j = 0; j < NbLinkFaces; j++)
			{
				S32 numF = m_TabPointsLinks[i].GetFace(j);

				Playspace_Mesh::Face &CurFace = m_TabQuad[numF];
				S32	NbFacePt = 4;
				if (CurFace.IsTri)
					NbFacePt = 3;
				for (S32 k = 0; k<NbFacePt; k++)
				{
					if (CurFace.TabPoints[k] <= i)
						continue;
					if (NbFacesOnEdge(i, CurFace.TabPoints[k]) != 1)
						continue;

					TabHole.Add(i);
					TabHole.Add(CurFace.TabPoints[k]);
					break;
				}

				if (TabHole.GetSize())
					break;
			}

			if (TabHole.GetSize())
				break;
		}

		// No Hole ?
		if (!TabHole.GetSize())
			return;

		// Complete Hole list.
		for(;;)
		{
			S32 NbHPt = TabHole.GetSize();
			S32 p0 = TabHole[NbHPt - 2];
			S32 p1 = TabHole[NbHPt - 1];
			S32 p2 = -1;
			S32 NbLinkFacesP1 = m_TabPointsLinks[p1].GetNbFaces();
			for (S32 j = 0; j < NbLinkFacesP1; j++)
			{
				S32 numF = m_TabPointsLinks[p1].GetFace(j);
				Playspace_Mesh::Face &CurFace = m_TabQuad[numF];
				S32	NbFacePt = 4;
				if (CurFace.IsTri)
					NbFacePt = 3;
				for (S32 k = 0; k<NbFacePt; k++)
				{
					if (CurFace.TabPoints[k] == p0)
						continue;
					if (NbFacesOnEdge(p1, CurFace.TabPoints[k]) != 1)
						continue;
					p2 = CurFace.TabPoints[k];
					break;
				}
				if (p2 >= 0)
					break;
			}

			EXCEPTIONC_Z(p2 >= 0,"FillHole => impossible case");
			if (p2 == TabHole[0])
				break;
			TabHole.Add(p2);
		}

		// Close this Hole.
		while (TabHole.GetSize() > 2)
		{
			// Search closest dist.
			S32 NbHPt = TabHole.GetSize();
			Float	BestDist = 1e10f;
			S32		BestPoint = -1;

			for (S32 i = 0; i < NbHPt; i++)
			{
				S32		iNext = i + 2;
				if (iNext >= NbHPt)
					iNext -= NbHPt;

				Float	Dist = (m_TabPoints[TabHole[i]] - m_TabPoints[TabHole[iNext]]).GetNorm2();
				if (Dist < BestDist)
				{
					BestDist = Dist;
					BestPoint = i;
				}
			}

			// Add Face !
			S32 NumFace = m_TabQuad.Add();
			Playspace_Mesh::Face &CurFace = m_TabQuad[NumFace];

			CurFace.IsTri = TRUE;
			CurFace.IsVirtual = TRUE;

			CurFace.TabPoints[0] = TabHole[BestPoint];
			BestPoint++;
			if (BestPoint >= NbHPt)	BestPoint -= NbHPt;
			S32 iSuppressPoint = BestPoint;
			CurFace.TabPoints[1] = TabHole[BestPoint];
			BestPoint++;
			if (BestPoint >= NbHPt)	BestPoint -= NbHPt;
			CurFace.TabPoints[2] = TabHole[BestPoint];
			CurFace.TabPoints[3] = TabHole[BestPoint];

			//Add Links.
			m_TabPointsLinks[CurFace.TabPoints[0]].AddFace(NumFace);
			m_TabPointsLinks[CurFace.TabPoints[1]].AddFace(NumFace);
			m_TabPointsLinks[CurFace.TabPoints[2]].AddFace(NumFace);

			// Suppress point.
			TabHole.Remove(iSuppressPoint);
		}
	}

}

/**************************************************************************/

void	Playspace_Mesh::Inflate(Float _Dist)
{
	// Prepare.
	ComputePointsLinks();
	ComputeFacesToolNormal();
	ComputePointsToolNormal();

	// Do Inflate.
	S32 NbPt = m_TabPoints.GetSize();

	for (S32 i = 0; i < NbPt; i++)
	{
		Vec3f	NewPos = m_TabPoints[i] + _Dist * m_TabPointsToolNormal[i].Normal;
		MovePoint(i, NewPos);
	}

	// Clear Bad Normal infos.
	m_TabPointsToolNormal.SetSize(0, TRUE);
	InvalidateFaceToolNormal();
	InvalidatePointsToolNormal();
}

/**************************************************************************/

void	Playspace_Mesh::AddNoise(Float _Dist)
{
	// Do Inflate.
	S32 NbPt = m_TabPoints.GetSize();

	for (S32 i = 0; i < NbPt; i++)
	{
		Vec3f	NewPos = m_TabPoints[i] + Vec3f(AleaF(-_Dist,_Dist),AleaF(-_Dist,_Dist),AleaF(-_Dist,_Dist));
		MovePoint(i, NewPos);
	}
	InvalidateFaceToolNormal();
	InvalidatePointsToolNormal();
}

/**************************************************************************/

void	Playspace_Mesh::RemoveIsolatedFaces(S32 _Size)
{
	// Prepare.
	ComputePointsLinks();
	InvalidateFaceToolNormal();
	InvalidatePointsToolNormal();

	HugeDynArray_Z<U8>	TabPointMark;
	TabPointMark.SetSize(m_TabPoints.GetSize());
	TabPointMark.Null();

	HugeDynArray_Z<U8>	TabFaceMark;
	TabFaceMark.SetSize(m_TabQuad.GetSize());
	TabFaceMark.Null();

	S32		StartStack,EndStack;
	HugeDynArray_Z<S32>	StackFace;
	StackFace.SetSize(m_TabQuad.GetSize());

	S32 NbFaces = TabFaceMark.GetSize();
	for (S32 i=0; i<NbFaces ; i++)
	{
		// Already Managed.
		if (TabFaceMark[i])
			continue;

		// Expand this group !
		StartStack = 0;
		EndStack = 0;

		TabFaceMark[i] = 1;
		StackFace[EndStack++] = i;

		do {
			Playspace_Mesh::Face	&CurFace = m_TabQuad[StackFace[StartStack++]];
			S32 nbPt = CurFace.IsTri ? 3 : 4;
			for (S32 j=0 ; j<nbPt ; j++)
			{
				S32 NumPoint = CurFace.TabPoints[j];
				U8	*pMarkPt = &TabPointMark[NumPoint];
				if (*pMarkPt)
					continue;
				*pMarkPt = 1;

				PointsLinks	&CurLinks = m_TabPointsLinks[NumPoint];
				S32 NbLinkFaces = CurLinks.GetNbFaces();
				for (S32 k = 0; k<NbLinkFaces; k++)
				{
					S32 NewFace = CurLinks.GetFace(k);
					if (TabFaceMark[NewFace])
						continue;
					TabFaceMark[NewFace] = 1;
					StackFace[EndStack++] = NewFace;
				}
			}
		} while (StartStack != EndStack);

		// Tag Faces.. if want to delete.
		if (EndStack <= _Size)
		{
			for (S32 j = 0; j<EndStack; j++)
				TabFaceMark[StackFace[j]] = 2;
		}
	}

	// Now Delete Faces if tagged.
	for (S32 i = NbFaces-1; i>=0 ; i--)
	{
		if (TabFaceMark[i] != 2)
			continue;
		RemoveFace(i,TRUE);
	}
	
	// Verify Mesh.
	CheckValidity();
}

/**************************************************************************/

class PlanarInfos
{
public:
	S32		IdNext;
	S16		GroupId;
	U8		IsDone;		// 0: Nothing 1: CurrentList 2: Finalized
};

class GroupInfos
{
public:
	Vec3f	Normal;
	Vec3f	Center;
	S32		IdFirst;
	S32		IdLast;
	S32		Nb;
};

class ReprojectInfos
{
public:
	Float	Dist;
	S32		Nb;

	void	ComputeDistFromOtthers(S32 _Delta)
	{
		Float	SumDist = 0.f;
		Float	Weight = 0.f;
		// Up
		ReprojectInfos	*pOther = this-_Delta-1;
		if (pOther->Nb)
		{
			SumDist += pOther->Dist * 0.5f;
			Weight +=  0.5f;
		}
		pOther++;
		if (pOther->Nb)
		{
			SumDist += pOther->Dist;
			Weight +=  1.f;
		}
		pOther++;
		if (pOther->Nb)
		{
			SumDist += pOther->Dist * 0.5f;
			Weight +=  0.5f;
		}
		// Middle
		pOther = this-1;
		if (pOther->Nb)
		{
			SumDist += pOther->Dist * 0.5f;
			Weight +=  0.5f;
		}
		pOther+=2;
		if (pOther->Nb)
		{
			SumDist += pOther->Dist * 0.5f;
			Weight +=  0.5f;
		}
		// Down
		pOther = this+_Delta-1;
		if (pOther->Nb)
		{
			SumDist += pOther->Dist * 0.5f;
			Weight +=  0.5f;
		}
		pOther++;
		if (pOther->Nb)
		{
			SumDist += pOther->Dist;
			Weight +=  1.f;
		}
		pOther++;
		if (pOther->Nb)
		{
			SumDist += pOther->Dist * 0.5f;
			Weight +=  0.5f;
		}

		Dist = SumDist / Weight;
		Nb = 1;
	}
};

void	Playspace_Mesh::PlanarFilter_AccurateNew()
{
	ComputePointsLinks();
	ComputeFacesToolNormal();
	ComputePointsToolNormal();	// Compute Face too.
	InvalidatePointsToolNormal();
	InvalidateFaceToolNormal();

	S32 NbFaces = m_TabQuad.GetSize();
	S32 NbPoints = m_TabPoints.GetSize();

	HugeDynArray_Z<Vec3f, 8, FALSE, FALSE>			TabLocalPoints;
	HugeDynArray_Z<PlanarInfos, 8, FALSE, FALSE>	TabInfosPlanar;
	DynArray_Z<ReprojectInfos,8,FALSE,FALSE>		TabRepro;
	DynArray_Z<GroupInfos,8,FALSE,FALSE>			TabGroups;

	TabInfosPlanar.SetSize(NbPoints);
		
	// Init Planar Datas.
	PlanarInfos *pInfos = TabInfosPlanar.GetArrayPtr();
	for (S32 i = 0; i<NbPoints; i++)
	{
		pInfos->IdNext = -1;
		pInfos->IsDone = 0;
		pInfos++;
	}

	// First : Create Groups

	Float MinError = Cos(DegToRad(10.f));

	Float ForceUpError = Cos(DegToRad(15.f));
	Float ForceVertError = Sin(DegToRad(15.f));

	Float MergeDist = 0.15f;
	Float MergeAngle = Cos(DegToRad(25.f));

	for (S32 i = 0; i<NbPoints; i++)
	{
		PlanarInfos	*pCurInfos = &TabInfosPlanar[i];
		if (pCurInfos->IsDone)
			continue;
		if (m_TabPointsToolNormal[i].Error < MinError)
			continue;

		S32	CurPoint = i;
		S32	FirstPoint = i;
		S32	LastPoint = i;
		S32 NbPointsOnZone = 1;

		S32 CurGroupId = TabGroups.GetSize();

		TabInfosPlanar[CurPoint].IsDone = 1;
		TabInfosPlanar[CurPoint].GroupId = CurGroupId;
		TabInfosPlanar[CurPoint].IdNext = -1;
		Vec3f	SumNormal = m_TabPointsToolNormal[i].Normal;
		Vec3f	SumNormalN = SumNormal;
		Vec3f	Center = m_TabPoints[i];
		Vec3f	CenterN = Center;
		Float	CenterWeight = 1.f;

		while (CurPoint >= 0)
		{
			// Check Voisins.
			Playspace_Mesh::PointsLinks &PointLinks = m_TabPointsLinks[CurPoint];
			Playspace_Mesh::ToolPointNormal &PointNormalInfos = m_TabPointsToolNormal[CurPoint];

			Vec3f MyPos = m_TabPoints[CurPoint];
			Vec3f MyNormal = PointNormalInfos.Normal;

			S32 NbFaces = PointLinks.GetNbFaces();
			for (S32 j = 0; j<NbFaces; j++)
			{
				// Same surface ?
				S32 LinkedFace = PointLinks.GetFace(j);

				Playspace_Mesh::Face &CurFace = m_TabQuad[LinkedFace];
				S32 NbTriPoints = CurFace.IsTri ? 3 : 4;
				for (S32 f = 0; f<NbTriPoints; f++)
				{
					// AddPoint ?
					S32 OtherPoint = CurFace.TabPoints[f];
					if (TabInfosPlanar[OtherPoint].IsDone)
						continue;

					// CheckContinuity.
					Playspace_Mesh::ToolPointNormal &OtherNormalInfos = m_TabPointsToolNormal[OtherPoint];
					Vec3f &OtherNormal = OtherNormalInfos.Normal;
					if ((SumNormalN * OtherNormal) < MergeAngle)
						continue;

					// Check Plane Dist.
					Vec3f	OtherPos = m_TabPoints[OtherPoint];
					Vec3f	vDelta = OtherPos - CenterN;

					Float	AbsProj = Abs(SumNormalN * vDelta);

					if (AbsProj >= MergeDist)
						continue;

					// Add Point.
					Float Surface = OtherNormalInfos.Surface * (1.f / (0.04f * 0.04f));
					SumNormal += OtherNormal * Surface;
					SumNormalN = SumNormal;
					SumNormalN.CNormalize();

					Center += OtherPos * Surface;
					CenterWeight += Surface;
					CenterN = Center * (1.f / CenterWeight);

					NbPointsOnZone++;
					TabInfosPlanar[OtherPoint].IsDone = 1;
					TabInfosPlanar[OtherPoint].GroupId = CurGroupId;
					TabInfosPlanar[OtherPoint].IdNext = -1;
					TabInfosPlanar[LastPoint].IdNext = OtherPoint;
					LastPoint = OtherPoint;
				}
			}

			// Next Point !
			CurPoint = TabInfosPlanar[CurPoint].IdNext;
		}

		// Manage it...
		if (NbPointsOnZone < 30)
		{
			// Clear All.
			CurPoint = FirstPoint;
			while (CurPoint >= 0)
			{
				TabInfosPlanar[CurPoint].IsDone = 0;
				TabInfosPlanar[CurPoint].GroupId = -1;
				CurPoint = TabInfosPlanar[CurPoint].IdNext;
			}
			continue;
		}

		// Compute Plane.
		TabLocalPoints.SetSize(NbPointsOnZone, TRUE);
		CurPoint = FirstPoint;
		S32 CurPos = 0;
		while (CurPoint >= 0)
		{
			TabLocalPoints[CurPos++] = m_TabPoints[CurPoint];
			CurPoint = TabInfosPlanar[CurPoint].IdNext;
		}

		Vec3f eigenvectors[3];
		Float eigenvalues[3];
		GenericEigen3D(TabLocalPoints.GetArrayPtr(), CurPos, eigenvectors, eigenvalues);

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
			SumNormalN = eigenvectors[0] ^ eigenvectors[1];
			SumNormalN.ANormalize();
		}
		if (SumNormalN.y > ForceUpError)
			SumNormalN = VEC3F_UP;
		else if (SumNormalN.y < -ForceUpError)
			SumNormalN = VEC3F_DOWN;
		if (Abs(SumNormalN.y) < ForceVertError)
		{
			SumNormalN.y = 0.f;
			SumNormalN.ANormalize();
		}
				
		// Create Group.
		S32 NewGroup = TabGroups.Add();
		EXCEPTIONC_Z(CurGroupId == NewGroup,"LAAAAAAAAAAAAAA");
		TabGroups[NewGroup].Normal = SumNormalN;
		TabGroups[NewGroup].Center = CenterN;
		TabGroups[NewGroup].IdFirst = FirstPoint;
		TabGroups[NewGroup].IdLast = LastPoint;
		TabGroups[NewGroup].Nb = NbPointsOnZone;
	}

	// Merge little noise.
	S32DA	TabConnectGroup;

	for (S32 i = 0; i<NbPoints; i++)
	{
		PlanarInfos	*pCurInfos = &TabInfosPlanar[i];
		if (pCurInfos->IsDone)
			continue;

		S32	CurPoint = i;
		S32	FirstPoint = i;
		S32	LastPoint = i;
		S32 NbPointsOnZone = 1;

		TabInfosPlanar[CurPoint].IsDone = 1;
		TabInfosPlanar[CurPoint].GroupId = -1;
		TabInfosPlanar[CurPoint].IdNext = -1;
		Vec3f	vMin = m_TabPoints[i];
		Vec3f	vMax = vMin;	

		TabConnectGroup.SetSize(0,TRUE);

		while (CurPoint >= 0)
		{
			// Check Voisins.
			Playspace_Mesh::PointsLinks &PointLinks = m_TabPointsLinks[CurPoint];
			S32 NbFaces = PointLinks.GetNbFaces();
			for (S32 j = 0; j<NbFaces; j++)
			{
				// Same surface ?
				S32 LinkedFace = PointLinks.GetFace(j);

				Playspace_Mesh::Face &CurFace = m_TabQuad[LinkedFace];
				S32 NbTriPoints = CurFace.IsTri ? 3 : 4;
				for (S32 f = 0; f<NbTriPoints; f++)
				{
					// AddPoint ?
					S32 OtherPoint = CurFace.TabPoints[f];
					if (TabInfosPlanar[OtherPoint].IsDone)
					{
						S32 OtherGroupId = TabInfosPlanar[OtherPoint].GroupId;
						if (OtherGroupId >= 0)
						{
							if (TabConnectGroup.Contains(OtherGroupId) < 0)
								TabConnectGroup.Add(OtherGroupId);
						}
						continue;
					}
					Vec3f pos = m_TabPoints[OtherPoint];
					vMin = Min(pos,vMin);
					vMax = Max(pos,vMax);

					NbPointsOnZone++;
					TabInfosPlanar[OtherPoint].IsDone = 1;
					TabInfosPlanar[OtherPoint].GroupId = -1;
					TabInfosPlanar[OtherPoint].IdNext = -1;
					TabInfosPlanar[LastPoint].IdNext = OtherPoint;
					LastPoint = OtherPoint;
				}
			}

			// Next Point !
			CurPoint = TabInfosPlanar[CurPoint].IdNext;
		}
		S32 CurGroup = -1;
		S32	BetterNb = 0;
		Float	BetterDist = 1e6f;
		for (S32 j=0 ; j<TabConnectGroup.GetSize() ; j++)
		{
			Vec3f Normal = TabGroups[TabConnectGroup[j]].Normal;
			Vec3f Center = TabGroups[TabConnectGroup[j]].Center;
			Float d1 = (vMax-Center) * Normal;
			Float d2 = (Center-vMin) * Normal;
			Float d = Max(Abs(d1),Abs(d2));

			S32 Nb = TabGroups[TabConnectGroup[j]].Nb;
			if ((Nb > BetterNb) && (d < 0.2f))
			{
				BetterNb = Nb;
				BetterDist = d;
				CurGroup = TabConnectGroup[j];
			}
		}
		if (CurGroup < 0)
			continue;
		if (TabConnectGroup.GetSize() == 1)
		{
			if (BetterDist > 0.2f)
			continue;
		}
		else if (TabConnectGroup.GetSize() == 2)
		{
			if (BetterDist > 0.1f)
				continue;
		}
		else if (BetterDist > 0.05f)
			continue;

		// Do It.
		TabInfosPlanar[TabGroups[CurGroup].IdLast].IdNext = FirstPoint;
		TabGroups[CurGroup].IdLast = LastPoint;

		CurPoint = FirstPoint;
		while (CurPoint >= 0)
		{
			TabInfosPlanar[CurPoint].IsDone = 3;
			TabInfosPlanar[CurPoint].GroupId = CurGroup;
			CurPoint = TabInfosPlanar[CurPoint].IdNext;
		}
	}

	// Do planar.
	for (S32 g=0 ; g<TabGroups.GetSize() ; g++)
	{
		S32 CurPoint = TabGroups[g].IdFirst;
		S32 FirstPoint = CurPoint;

		Vec3f SumNormalN = TabGroups[g].Normal;
		Vec3f CenterN = TabGroups[g].Center;

		// Set to the plane.
		Vec3f vFront,vLeft;
		SumNormalN.Get2OrthoVector(vFront,vLeft);

		// Compute Min Max.
		CurPoint = FirstPoint;
		Float	xMin = 1e8f;
		Float	xMax = -1e8f;
		Float	yMin = 1e8f;
		Float	yMax = -1e8f;
		
		while (CurPoint >= 0)
		{
			Vec3f	pos = m_TabPoints[CurPoint];			
			Float fx = pos * vLeft;
			Float fy = pos * vFront;

			if (fx < xMin)
				xMin = fx;
			if (fx > xMax)
				xMax = fx;

			if (fy < yMin)
				yMin = fy;
			if (fy > yMax)
				yMax = fy;

			CurPoint = TabInfosPlanar[CurPoint].IdNext;
		}

		xMin -= 0.805f;	// 2 cases + security.
		yMin -= 0.805f;	// 2 cases + security.

		Float	Size = 0.4f;
		Float	InvSize = 1.f / Size;

		S32		NbCellX = (S32)((xMax-xMin) * InvSize) + 3; // 3 cases : 2 plus la "courrante"
		S32		NbCellY = (S32)((yMax-yMin) * InvSize) + 3; // 3 cases : 2 plus la "courrante"

		Float	MinXToMiddle = -xMin + Size*0.5f;
		Float	MinYToMiddle = -yMin + Size*0.5f;

		TabRepro.SetSize(NbCellX*NbCellY,TRUE);
		TabRepro.Null();

		CurPoint = FirstPoint;
		while (CurPoint >= 0)
		{
			if (TabInfosPlanar[CurPoint].IsDone != 3)
			{
				Vec3f pos = m_TabPoints[CurPoint];
				Vec3f delta = CenterN - pos;

				Float dx = pos * vLeft;
				Float dy = pos * vFront;
				Float dh = pos * SumNormalN;

				S32 ix = (S32)((dx + MinXToMiddle) * InvSize);
				S32 iy = (S32)((dy + MinYToMiddle) * InvSize);
				ReprojectInfos &CurInfos = TabRepro[ix + iy*NbCellX];

				Vec3f	&Normal = m_TabPointsToolNormal[CurPoint].Normal;

				CurInfos.Nb++;	
				CurInfos.Dist += dh;

			}
			CurPoint = TabInfosPlanar[CurPoint].IdNext;
		}

		for (S32 j=0 ; j<TabRepro.GetSize() ; j++)
		{
			ReprojectInfos &CurInfos = TabRepro[j];
			if (CurInfos.Nb)
			{
				Float iRatio = 1.f / (Float)CurInfos.Nb;
				CurInfos.Dist *= iRatio;	
			}
		}

		// Do planar... 
		{
			CurPoint = FirstPoint;
			while (CurPoint >= 0)
			{
				Vec3f pos = m_TabPoints[CurPoint];
				Vec3f delta = CenterN - pos;

				Float dx = pos * vLeft;
				Float dy = pos * vFront;

				dx = (dx - xMin) * InvSize;
				dy = (dy - yMin) * InvSize;

				S32 ix = (S32)dx;
				S32 iy = (S32)dy;

				EXCEPTIONC_Z(ix >= 1,"BAD POINT PlanarFilter_AccurateNew");
				EXCEPTIONC_Z(iy >= 1,"BAD POINT PlanarFilter_AccurateNew");
				EXCEPTIONC_Z(ix < (NbCellX-2),"BAD POINT PlanarFilter_AccurateNew");
				EXCEPTIONC_Z(iy < (NbCellY-2),"BAD POINT PlanarFilter_AccurateNew");

				ReprojectInfos *pI00 = &TabRepro[ix + iy*NbCellX];
				ReprojectInfos *pI01 = pI00+1;
				ReprojectInfos *pI10 = pI00+NbCellX;
				ReprojectInfos *pI11 = pI10+1;

				if ((pI00->Nb + pI01->Nb + pI10->Nb + pI11->Nb) == 0)
				{
					TabInfosPlanar[CurPoint].IsDone = 0;
					CurPoint = TabInfosPlanar[CurPoint].IdNext;
					continue;
				}
				if (!pI00->Nb)
					pI00->ComputeDistFromOtthers(NbCellX);
				if (!pI01->Nb)
					pI01->ComputeDistFromOtthers(NbCellX);
				if (!pI10->Nb)
					pI10->ComputeDistFromOtthers(NbCellX);
				if (!pI11->Nb)
					pI11->ComputeDistFromOtthers(NbCellX);

				Float ratioX = dx - ix;
				Float ratioY = dy - iy;

				Float d00 = pI00->Dist;
				Float d01 = pI01->Dist;
				Float d10 = pI10->Dist;
				Float d11 = pI11->Dist;

				Float d0 = d00 + (d01-d00) * ratioX;
				Float d1 = d10 + (d11-d10) * ratioX;

				Float d = d0 + (d1-d0) * ratioY;
				Float dh = pos * SumNormalN;

				pos += (d-dh) * SumNormalN;

				MovePoint(CurPoint, pos);
				TabInfosPlanar[CurPoint].IsDone = 2;

				// Next Point !
				CurPoint = TabInfosPlanar[CurPoint].IdNext;
			}
		}
	}
}

/**************************************************************************/

void	Playspace_Mesh::ApplyQuat(Quat &_Transfo)
{
	S32	nbPt = m_TabPoints.GetSize();

	m_TabRefPoint.Empty();
	m_pFreeTabPoint = NULL;
	m_QuadTreePt.SetSize(MESH_QUADTREE_TOTAL_SIZE,TRUE);
	memset(m_QuadTreePt.GetArrayPtr(), 0, MESH_QUADTREE_TOTAL_SIZE*sizeof(Playspace_Mesh::RefPoint*));

	Mat4x4	MatTransfo;
	_Transfo.GetMatrix(MatTransfo);

	// First Check Point.
	for (S32 i = 0; i < nbPt; i++)
	{
		// Verifu Point infos.
		Vec4f pt(m_TabPoints[i]);
		m_TabPoints[i] = VecFloat4x4Transform3(MatTransfo,pt);
		InsertQuadTreePoint(i, m_TabPoints[i]);
		if (m_HavePointsToolNormal)
		{
			Vec4f nm(m_TabPointsToolNormal[i].Normal);
			m_TabPointsToolNormal[i].Normal = VecFloat4x4Transform3(MatTransfo,nm);
		}
	}

	if (m_HaveFaceToolNormal)
	{
		S32	nbFaces = m_TabQuad.GetSize();
		for (S32 i = 0; i < nbFaces; i++)
		{
			Vec4f pt(m_TabFaceToolNormal[i].Center);
			m_TabFaceToolNormal[i].Center = VecFloat4x4Transform3(MatTransfo,pt);
			Vec4f nm(m_TabFaceToolNormal[i].Normal);
			m_TabFaceToolNormal[i].Normal = VecFloat4x4Transform3(MatTransfo,nm);
		}
	}
}

/**************************************************************************/

Bool	Playspace_Mesh::CheckValidity()
{
#ifdef _DEBUG
	S32	nbPt = m_TabPoints.GetSize();
	S32	nbQuad = m_TabQuad.GetSize();

	// First Check Point.
	for (S32 i = 0; i < nbPt; i++)
	{
		if (	(m_TabPoints[i].x != m_TabPoints[i].x)
			||	(m_TabPoints[i].y != m_TabPoints[i].y)
			||  (m_TabPoints[i].z != m_TabPoints[i].z)
			)
			EXCEPTIONC_Z(0, "Bad Point value");

		// Verify num Faces.
		S32 NbLinkFace = m_TabPointsLinks[i].GetNbFaces();
		for (S32 j = 0; j < NbLinkFace; j++)
		{
			S32 NumF = m_TabPointsLinks[i].GetFace(j);
			EXCEPTIONC_Z((NumF < nbQuad), "bad num quad %d");

			// Verify reference.
			Playspace_Mesh::Face &CurFace = m_TabQuad[NumF];
			S32	NbFacePt = 4;
			if (CurFace.IsTri)
				NbFacePt = 3;
			Bool IsHere = FALSE;
			for (S32 k = 0; k<NbFacePt; k++)
			{
				if (CurFace.TabPoints[k] == i)
				{
					IsHere = TRUE;
					break;
				}
			}
			EXCEPTIONC_Z(IsHere,"bad Reference Face on Point!");
		}
	}
	// Check Faces.
	for (S32 i = 0; i < nbQuad; i++)
	{
		Playspace_Mesh::Face &CurFace = m_TabQuad[i];
		S32	NbFacePt = 4;
		if (CurFace.IsTri)
			NbFacePt = 3;
		for (S32 k = 0; k<NbFacePt; k++)
		{
			S32 numPt = CurFace.TabPoints[k];
			EXCEPTIONC_Z((numPt < nbPt), "bad num point %d");
			Playspace_Mesh::PointsLinks	&PtInfos = m_TabPointsLinks[numPt];
			S32 NbPtFace = PtInfos.GetNbFaces();
			Bool IsHere = FALSE;
			for (S32 j = 0; j<NbPtFace; j++)
			{
				if (PtInfos.GetFace(j) == i)
				{
					IsHere = TRUE;
					break;
				}
			}
			EXCEPTIONC_Z(IsHere, "bad Reference Point on Face!");
		}
	}
#endif
	return TRUE;
}

/**************************************************************************/

S32 ComputeDirFromNormal(const Vec3f& _normal) 
{
	if( Abs(_normal.x) > Abs(_normal.z) ) // x dir
	{
		if( _normal.x < 0.f )
			return 2;

		return 3;
	}
	
	// z dir
	if( _normal.z < 0.f )
		return 4;

	return 5;
}

/**************************************************************************/

static Vec2i g_neightboor[4] = { Vec2i(-1,0),Vec2i(0,-1),Vec2i(0,1),Vec2i(1,0) };
#define MAP2DCELL 0.08f
#define MAP2DCELLAREA 0.0064f
#define MAP2DHEIGHTCONEX 0.04f
#define MAP2DPLATFORMLIMIT 0.15f

void	Playspace_Mesh::ComputeStats(Float AngleTolerance, Float YGround, Float YCeiling, Float ZHorizMax, const Vec3f& _minAvailable, const Vec3f& _maxAvailable, PlaySpaceMesh_Stats& _OutStats)
{
	ComputeFacesToolNormal();
	S32 NbFaces = m_TabQuad.GetSize();

	Float ZHorizMin = YGround - .15f;
	Float SurfaceMin = 0.07f*0.07f;
	Float MinCos = Cos(AngleTolerance);
	Float MinSin = Abs(Sin(AngleTolerance));
	_OutStats.Reset();

	S32 sizeX = FLOORINT((_maxAvailable.x - _minAvailable.x)/MAP2DCELL) + 1;
	S32 sizeY = FLOORINT((_maxAvailable.y - _minAvailable.y)/MAP2DCELL) + 1;
	S32 sizeZ = FLOORINT((_maxAvailable.z - _minAvailable.z)/MAP2DCELL) + 1;
	S32 maxSize = Max(sizeX,Max(sizeY,sizeZ));
	S32 sizeTotal = maxSize*maxSize;
	// 0 : floor
	// 1 : ceilling
	// 2 : wall x neg
	// 3 : wall x pos
	// 4 : wall z neg
	// 5 : wall z pos
	FloatDA height2D[6];
	for(S32 dir = 0; dir<_countof(height2D); dir++)
	{
		height2D[dir].SetSize(sizeTotal);
		for(S32 i=0; i<sizeTotal; i++)
			height2D[dir][i] = 0.f;
	}

	Float y2DLimit = (_maxAvailable.y - _minAvailable.y) * 0.75f + _minAvailable.y;

	for (S32 i = 0; i<NbFaces ; i++)
	{
		Playspace_Mesh::Face &CurFace = m_TabQuad[i];

		if (!CurFace.IsSeenQuality)
			continue;
		
		if (CurFace.IsVirtual)
			continue;

		Playspace_Mesh::ToolFaceNormal &CurFaceInfos = m_TabFaceToolNormal[i];
		if (CurFaceInfos.Surface < SurfaceMin)
			continue;

		if (CurFace.IsPaintMode != 2)	// Keep Only painted faces.
		{
			if (CurFace.IsPaintMode == 1)
				_OutStats.ColoredCells[0]++;

			continue;
		}

		_OutStats.ColoredCells[CurFace.IsSeenQuality]++;

		if (CurFaceInfos.Normal.y >= MinCos)
		{
			_OutStats.UpSurface += CurFaceInfos.Surface;
			if ((CurFaceInfos.Center.y > ZHorizMin) && (CurFaceInfos.Center.y < ZHorizMax))
			{
				_OutStats.HorizSurface += CurFaceInfos.Surface;
			}

			// map 2D setup
			if(CurFaceInfos.Center.y < y2DLimit)
			{
				S32 idx = FLOORINT((CurFaceInfos.Center.z - _minAvailable.z)/MAP2DCELL) * maxSize;
				idx += FLOORINT((CurFaceInfos.Center.x - _minAvailable.x)/MAP2DCELL);
				if( idx >= 0 && idx < sizeTotal )
				{
					height2D[0][idx] = Max(height2D[0][idx], CurFaceInfos.Center.y - _minAvailable.y); // floor and platform
				}
			}
		}
		else if( CurFaceInfos.Normal.y <= -MinCos)
		{
			_OutStats.DownSurface += CurFaceInfos.Surface;

			// map 2D setup
			if(CurFaceInfos.Center.y > y2DLimit)
			{
				S32 idx = FLOORINT((CurFaceInfos.Center.z - _minAvailable.z)/MAP2DCELL) * maxSize;
				idx += FLOORINT((CurFaceInfos.Center.x - _minAvailable.x)/MAP2DCELL);
				if( idx >= 0 && idx < sizeTotal )
				{
					height2D[1][idx] = Max(height2D[1][idx],_maxAvailable.y  - CurFaceInfos.Center.y);
				}
			}
		}
		else if (Abs(CurFaceInfos.Normal.y) <= MinSin)
		{
			_OutStats.WallSurface += CurFaceInfos.Surface;

			S32 dir = ComputeDirFromNormal(CurFaceInfos.Normal);
			if( dir <= 3 )
			{
				S32 idx = FLOORINT((CurFaceInfos.Center.z - _minAvailable.z)/MAP2DCELL) * maxSize;
				idx += FLOORINT((CurFaceInfos.Center.y - _minAvailable.y)/MAP2DCELL);

				if( dir == 2 ) // x < 0
				{
					if( CurFaceInfos.Center.x > _minAvailable.x + 1.f )
					{
						if( idx >= 0 && idx < sizeTotal )
						{
							height2D[dir][idx] = Max(height2D[dir][idx],_maxAvailable.x  - CurFaceInfos.Center.x);
						}
					}
				}
				else // dir == 3  // x > 0
				{
					if( CurFaceInfos.Center.x < _maxAvailable.x - 1.f )
					{
						if( idx >= 0 && idx < sizeTotal )
						{
							height2D[dir][idx] = Max(height2D[dir][idx],CurFaceInfos.Center.x- _minAvailable.x);
						}
					}
				}
			}
			else
			{
				S32 idx = FLOORINT((CurFaceInfos.Center.x - _minAvailable.x)/MAP2DCELL) * maxSize;
				idx += FLOORINT((CurFaceInfos.Center.y - _minAvailable.y)/MAP2DCELL);

				if( dir == 4 ) // z < 0
				{
					if( CurFaceInfos.Center.z > _minAvailable.z + 1.f )
					{
						if( idx >= 0 && idx < sizeTotal )
						{
							height2D[dir][idx] = Max(height2D[dir][idx],_maxAvailable.z  - CurFaceInfos.Center.z);
						}
					}
				}
				else // dir == 5 // z > 0
				{
					if( CurFaceInfos.Center.z < _maxAvailable.z - 1.f )
					{
						if( idx >= 0 && idx < sizeTotal )
						{
							height2D[dir][idx] = Max(height2D[dir][idx],CurFaceInfos.Center.z- _minAvailable.z);
						}
					}
				}
			}
		}
		_OutStats.TotalSurface += CurFaceInfos.Surface;
	}

	// analyze map2D
	Float floorSurface = 0.f;
	Float wallSurface = 0.f;
	S32DA map2Did;
	map2Did.SetSize(sizeTotal);
	DynArray_Z<Vec2i,32> neightboorStack;
	neightboorStack.SetSize(sizeTotal);
	for(S32 dir=0; dir<_countof(height2D); dir++ )
	{
		// init stack & tag map
		S32 stackIdx = 0;
		for(S32 i=0; i<sizeTotal; i++)
			map2Did[i] = 0;

		S32 nextID = 1;
		for(S32 z = 0; z < maxSize; z++)
			for(S32 x = 0; x < maxSize; x++)		
			{
				S32 idx = z * maxSize + x;
				if( map2Did[idx] == 0 && height2D[dir][idx] > Float_Eps)
				{
					S32 curID = nextID;
					nextID++;

					map2Did[idx] = curID;	

					S32 curDir = dir;
					if (dir == 0 && height2D[dir][idx] > (YGround + MAP2DPLATFORMLIMIT - _minAvailable.y))
						curDir = 6;

					S32 areaIdx = _OutStats.Areas[curDir].Add(MAP2DCELLAREA);

					// check virtual ceiling
					if (dir == 0)
					{
						floorSurface += MAP2DCELLAREA;
						if (height2D[1][idx] < Float_Eps)
							_OutStats.VirtualCeilingSurface += MAP2DCELLAREA;
					}
					else if (dir != 1)
					{
						wallSurface += MAP2DCELLAREA;
					}

					neightboorStack[stackIdx].Set(x,z);
					stackIdx++;

					while(stackIdx > 0 )
					{
						stackIdx--;
						S32 curX = neightboorStack[stackIdx].x;
						S32 curZ = neightboorStack[stackIdx].y;					

						for(S32 n=0; n<_countof(g_neightboor); n++)
						{
							S32 nx = curX + g_neightboor[n].x;
							S32 nz = curZ + g_neightboor[n].y;
							if( nx >= 0 && nz >= 0 && nx < maxSize && nz < maxSize )
							{
								S32 nidx = nz * maxSize + nx;
								if( map2Did[nidx] == 0 && height2D[dir][nidx] > Float_Eps && Abs(height2D[dir][idx] - height2D[dir][nidx]) <= MAP2DHEIGHTCONEX)
								{
									map2Did[nidx] = curID;
									_OutStats.Areas[curDir][areaIdx] += MAP2DCELLAREA;
									neightboorStack[stackIdx].Set(nx,nz);
									stackIdx++;

									// check virtual ceiling
									if (dir == 0)
									{
										floorSurface += MAP2DCELLAREA;
										if (height2D[1][idx] < Float_Eps)
											_OutStats.VirtualCeilingSurface += MAP2DCELLAREA;
									}
									else if(dir != 1)
									{
										wallSurface += MAP2DCELLAREA;
									}
								}
							}
						}
					}
				}
			}
	}

	// estimatate virtual wall
	_OutStats.VirtualWallSurface = Sqrt(floorSurface) * (YCeiling - YGround);
	_OutStats.VirtualWallSurface = Max(0.f, _OutStats.VirtualWallSurface - wallSurface);

	//Frame Stamp
	_OutStats.FrameStamp = GetGlobalFrameCount();
}

/**************************************************************************/

void Playspace_Mesh::Subdivide(float maxLength)
{
	SetEditMode(TRUE);
	S32	nbQuad = m_TabQuad.GetSize();

	for (S32 i = 0; i < nbQuad; i++)
	{
		// Accelerate Allocation.
		if (m_TabQuad.GetReserved() < 512)
			m_TabQuad.SetReserve(m_TabQuad.GetReservedSize() + 100000);
		if (m_TabPoints.GetReserved() < 512)
			m_TabPoints.SetReserve(m_TabPoints.GetReservedSize() + 100000);

		// End accelerate.
		Playspace_Mesh::Face &CurFace = m_TabQuad[i];
		EXCEPTIONC_Z((CurFace.IsTri), "can only subdivide triangles");

		bool CurFaceIsVirtual = CurFace.IsVirtual;
		bool CurFaceIsExternal = CurFace.IsExternal;

		S32 idx0 = CurFace.TabPoints[0];
		S32 idx1 = CurFace.TabPoints[1];
		S32 idx2 = CurFace.TabPoints[2];

		Vec3f v01 = m_TabPoints[idx1] - m_TabPoints[idx0];
		Vec3f v02 = m_TabPoints[idx2] - m_TabPoints[idx0];
		Vec3f v12 = m_TabPoints[idx2] - m_TabPoints[idx1];

		float norm01 = v01.GetNorm2();	// p0 p1
		float norm02 = v02.GetNorm2();	// p0 p2
		float norm12 = v12.GetNorm2();	// p1 p2

		float maxLength2 = maxLength * maxLength;

		S32 mask = 0;
		if (norm01 > maxLength2)
			mask |= 1;
		if (norm02 > maxLength2)
			mask |= 2;
		if (norm12 > maxLength2)
			mask |= 4;

		if (!mask)
			continue;

		if (mask == 7)
		{
			Vec3f middle = (m_TabPoints[idx1] + m_TabPoints[idx0]) * 0.5f;		// middle of [p0 p1]
			S32 idxNewPoint01 = AddPoint(middle);
			middle = (m_TabPoints[idx2] + m_TabPoints[idx0]) * 0.5f;			// middle of [p0 p2]
			S32 idxNewPoint02 = AddPoint(middle);
			middle = (m_TabPoints[idx2] + m_TabPoints[idx1]) * 0.5f;			// middle of [p1 p2]
			S32 idxNewPoint12 = AddPoint(middle);

			CurFace.TabPoints[1] = idxNewPoint01;		// CurFace : (p0 middle01 middle02)
			CurFace.TabPoints[2] = idxNewPoint02;
			CurFace.TabPoints[3] = idxNewPoint02;
			EXCEPTIONC_Z(idxNewPoint01 != idxNewPoint02, "LAAAAAAAAAA");
			EXCEPTIONC_Z(idxNewPoint01 != idxNewPoint12, "LAAAAAAAAAA");
			EXCEPTIONC_Z(idxNewPoint02 != idxNewPoint12, "LAAAAAAAAAA");

			// Add Face
			S32 idxNewFace = m_TabQuad.Add();
			Playspace_Mesh::Face &newFace1 = m_TabQuad[idxNewFace];
			newFace1.IsTri = TRUE;
			newFace1.IsVirtual = CurFaceIsVirtual;
			newFace1.IsExternal = CurFaceIsExternal;
			newFace1.TabPoints[0] = idx1;			// 1st newFace : (p1 middle12 middle01)
			newFace1.TabPoints[1] = idxNewPoint12;
			newFace1.TabPoints[2] = idxNewPoint01;
			newFace1.TabPoints[3] = idxNewPoint01;
			// Add Face
			idxNewFace = m_TabQuad.Add();
			Playspace_Mesh::Face &newFace2 = m_TabQuad[idxNewFace];
			newFace2.IsTri = TRUE;
			newFace2.IsVirtual = CurFaceIsVirtual;
			newFace2.IsExternal = CurFaceIsExternal;
			newFace2.TabPoints[0] = idx2;			// 2nd newFace : (p2 middle02 middle12)
			newFace2.TabPoints[1] = idxNewPoint02;
			newFace2.TabPoints[2] = idxNewPoint12;
			newFace2.TabPoints[3] = idxNewPoint12;
			// Add Face
			idxNewFace = m_TabQuad.Add();
			Playspace_Mesh::Face &newFace3 = m_TabQuad[idxNewFace];
			newFace3.IsTri = TRUE;
			newFace3.IsVirtual = CurFaceIsVirtual;
			newFace3.IsExternal = CurFaceIsExternal;
			newFace3.TabPoints[0] = idxNewPoint01;	// 3rd newFace : (middle01 middle12 middle02)
			newFace3.TabPoints[1] = idxNewPoint12;
			newFace3.TabPoints[2] = idxNewPoint02;
			newFace3.TabPoints[3] = idxNewPoint02;
			nbQuad += 3;
			i--;		// to recompute the same triangle
			continue;
		}

		S32 tmp;
		float tmpNorm;
		Vec3f tmpVec;

		if ((mask == 1) || (mask == 2) || (mask == 4))
		{
			if (mask == 2)			// only p0 p2 is too long
			{
				tmp = idx2;
				idx2 = idx1;	// rotation:  (p0 p1 p2) -> (p2 p0 p1)
				idx1 = idx0;
				idx0 = tmp;
			}
			else if (mask == 4)		// only p1 p2 is too long
			{
				tmp = idx0;
				idx0 = idx1;	// rotation:  (p0 p1 p2) -> (p1 p2 p0)
				idx1 = idx2;
				idx2 = tmp;
			}

			// here p0 p1 is too long

			Vec3f middle = (m_TabPoints[idx1] + m_TabPoints[idx0]) * 0.5f;		// middle of [p0 p1]
			S32 idxNewPoint = AddPoint(middle);

			if (idxNewPoint == idx2)
			{
				RemoveFace(i, FALSE);
				nbQuad--;
				i--;
				continue;
			}

			CurFace.TabPoints[0] = idx0;
			CurFace.TabPoints[1] = idxNewPoint;		// Curface : (p0 middle p2)
			CurFace.TabPoints[2] = idx2;
			CurFace.TabPoints[3] = idx2;

			// Add Face
			S32 idxNewFace = m_TabQuad.Add();
			Playspace_Mesh::Face &newFace = m_TabQuad[idxNewFace];
			newFace.IsTri = TRUE;
			newFace.IsVirtual = CurFaceIsVirtual;
			newFace.IsExternal = CurFaceIsExternal;
			newFace.TabPoints[0] = idxNewPoint;		// newFace : (middle p1 p2)
			newFace.TabPoints[1] = idx1;
			newFace.TabPoints[2] = idx2;
			newFace.TabPoints[3] = idx2;
			nbQuad++;
		}
		else	// mask is 3 5 or 6
		{
			if (mask == 5)			// p0 p1 and p1 p2 are too long
			{
				tmp = idx0;
				idx0 = idx1;	// rotation:  (p0 p1 p2) -> (p1 p2 p0)
				idx1 = idx2;
				idx2 = tmp;

				tmpNorm = norm01;
				norm01 = norm12;
				norm12 = norm02;
				norm02 = tmpNorm;

				tmpVec = v01;
				v01 = v12;
				v12 = -v02;
				v02 = -tmpVec;
			}
			else if (mask == 6)		// p0 p2 and p1 p2 are too long
			{
				tmp = idx2;
				idx2 = idx1;	// rotation:  (p0 p1 p2) -> (p2 p0 p1)
				idx1 = idx0;
				idx0 = tmp;

				tmpNorm = norm01;
				norm01 = norm02;
				norm02 = norm12;
				norm12 = tmpNorm;

				tmpVec = v01;
				v01 = -v02;
				v02 = -v12;
				v12 = tmpVec;
			}

			// here p0 p1 and p0 p2 are too long

			S32 nb = 1;

			Vec3f middle0;
			Vec3f middle1;

			S32 idxMiddle0;
			S32 idxMiddle1;

			S32 idxNewFace0;
			S32 idxNewFace1;

			if (norm01 < norm02)
			{
				while (norm01 > maxLength2)
				{
					v01 *= 0.5f;
					v02 *= 0.5f;
					norm01 *= 0.25f;
					nb <<= 1;
				}

				for (S32 j = nb - 1; j > 0; j--)
				{
					middle0 = m_TabPoints[idx0] + (v01 * j);
					middle1 = m_TabPoints[idx0] + (v02 * j);

					idxMiddle0 = AddPoint(middle0);
					idxMiddle1 = AddPoint(middle1);

					if (idxMiddle0 == idxMiddle1)
					{
						idx0 = idxMiddle0;
						break;
					}

					// Add Faces
					idxNewFace0 = m_TabQuad.Add();
					Playspace_Mesh::Face &newFace0 = m_TabQuad[idxNewFace0];
					newFace0.IsTri = TRUE;
					newFace0.IsVirtual = CurFaceIsVirtual;
					newFace0.IsExternal = CurFaceIsExternal;
					newFace0.TabPoints[0] = idx1;
					newFace0.TabPoints[1] = idxMiddle1;
					newFace0.TabPoints[2] = idxMiddle0;
					newFace0.TabPoints[3] = idxMiddle0;

					idxNewFace1 = m_TabQuad.Add();
					Playspace_Mesh::Face &newFace1 = m_TabQuad[idxNewFace1];
					newFace1.IsTri = TRUE;
					newFace1.IsVirtual = CurFaceIsVirtual;
					newFace1.IsExternal = CurFaceIsExternal;
					newFace1.TabPoints[0] = idx2;
					newFace1.TabPoints[1] = idxMiddle1;
					newFace1.TabPoints[2] = idx1;
					newFace1.TabPoints[3] = idx1;

					idx1 = idxMiddle0;
					idx2 = idxMiddle1;
					nbQuad += 2;
				}

				Playspace_Mesh::Face &CurFace = m_TabQuad[i];
				CurFace.TabPoints[0] = idx0;
				CurFace.TabPoints[1] = idx1;
				CurFace.TabPoints[2] = idx2;
				CurFace.TabPoints[3] = idx2;
			}
			else		// norm01 >= norm2
			{
				while (norm02 > maxLength2)
				{
					v01 *= 0.5f;
					v02 *= 0.5f;
					norm02 *= 0.25f;
					nb <<= 1;
				}

				for (S32 j = nb - 1; j > 0; j--)
				{
					middle0 = m_TabPoints[idx0] + (v01 * j);
					middle1 = m_TabPoints[idx0] + (v02 * j);

					idxMiddle0 = AddPoint(middle0);
					idxMiddle1 = AddPoint(middle1);

					if (idxMiddle0 == idxMiddle1)
					{
						idx0 = idxMiddle0;
						break;
					}

					// Add Faces
					idxNewFace0 = m_TabQuad.Add();
					Playspace_Mesh::Face &newFace0 = m_TabQuad[idxNewFace0];
					newFace0.IsTri = TRUE;
					newFace0.IsVirtual = CurFaceIsVirtual;
					newFace0.IsExternal = CurFaceIsExternal;
					newFace0.TabPoints[0] = idx1;
					newFace0.TabPoints[1] = idx2;
					newFace0.TabPoints[2] = idxMiddle0;
					newFace0.TabPoints[3] = idxMiddle0;

					idxNewFace1 = m_TabQuad.Add();
					Playspace_Mesh::Face &newFace1 = m_TabQuad[idxNewFace1];
					newFace1.IsTri = TRUE;
					newFace1.IsVirtual = CurFaceIsVirtual;
					newFace1.IsExternal = CurFaceIsExternal;
					newFace1.TabPoints[0] = idx2;
					newFace1.TabPoints[1] = idxMiddle1;
					newFace1.TabPoints[2] = idxMiddle0;
					newFace1.TabPoints[3] = idxMiddle0;

					idx1 = idxMiddle0;
					idx2 = idxMiddle1;
					nbQuad += 2;
				}

				Playspace_Mesh::Face &CurFace = m_TabQuad[i];
				CurFace.TabPoints[0] = idx0;
				CurFace.TabPoints[1] = idx1;
				CurFace.TabPoints[2] = idx2;
				CurFace.TabPoints[3] = idx2;
			}
			
			i--;		// to recompute the same triangle
		}
	}
	SetEditMode(FALSE);
	m_TabQuad.Minimize();
	m_TabPoints.Minimize();
}

/**************************************************************************/
////////////////////// SPHERE CONVEX /////////////////////////


void Playspace_Mesh::ProceduralSphere(S32 _depth, float _radius, Vec3f _center, Bool _faceNormalOrientedOutside /* = TRUE*/)
{
	Flush(TRUE);
	SetEditMode(TRUE);
	
	ProceduralSphere_InitGeneration(_depth, _faceNormalOrientedOutside);

	S32 NbPts = m_TabPoints.GetSize();
	for(S32 i = 0; i < NbPts; i++)
		m_TabPoints[i] = m_TabPoints[i] * _radius + _center;  

}

void Playspace_Mesh::ProceduralSphere_InitGeneration(S32 _depth, Bool _faceNormalOrientedOutside)
{
	const double X = 0.525731112119133606;
    const double Z = 0.850650808352039932;
    Vec3f vdata[12] = {
        Vec3f(-X, 0.0, Z), Vec3f( X, 0.0, Z ),Vec3f( -X, 0.0, -Z ), Vec3f( X, 0.0, -Z ),
        Vec3f( 0.0, Z, X ), Vec3f( 0.0, Z, -X ), Vec3f( 0.0, -Z, X ), Vec3f( 0.0, -Z, -X ),
        Vec3f( Z, X, 0.0 ), Vec3f( -Z, X, 0.0 ), Vec3f( Z, -X, 0.0 ), Vec3f( -Z, -X, 0.0 )
    };
    S32 tindices[20][3] = {
        {0, 4, 1}, { 0, 9, 4 }, { 9, 5, 4 }, { 4, 5, 8 }, { 4, 8, 1 },
        { 8, 10, 1 }, { 8, 3, 10 }, { 5, 3, 8 }, { 5, 2, 3 }, { 2, 7, 3 },
        { 7, 10, 3 }, { 7, 6, 10 }, { 7, 11, 6 }, { 11, 0, 6 }, { 0, 1, 6 },
        { 6, 1, 10 }, { 9, 0, 11 }, { 9, 11, 2 }, { 9, 2, 5 }, { 7, 2, 11 }
    };
    for(int i = 0; i < 20; i++)
		ProceduralSphere_Subdivide(vdata[tindices[i][0]], vdata[tindices[i][1]], vdata[tindices[i][2]], _depth, _faceNormalOrientedOutside);

}

void Playspace_Mesh::ProceduralSphere_Subdivide(Vec3f &v1, Vec3f & v2, Vec3f & v3, S32 depth, Bool _faceNormalOrientedOutside)
{

    if(depth == 0) //final -> add vertices and face
	{
		S32 idx1 = AddPoint(v1);
		S32 idx2 = AddPoint(v2);
		S32 idx3 = AddPoint(v3);

		// Add Face !
		S32 NumFace = m_TabQuad.Add();
		Playspace_Mesh::Face &CurFace = m_TabQuad[NumFace];
		CurFace.IsTri = TRUE;
		CurFace.IsVirtual = TRUE;
		CurFace.TabPoints[0] = (_faceNormalOrientedOutside)?idx1:idx2;
		CurFace.TabPoints[1] = (_faceNormalOrientedOutside)?idx2:idx1;
		CurFace.TabPoints[2] = idx3;
		CurFace.TabPoints[3] = idx3;
        return;
    }
	Vec3f v12 = (v1 + v2).Normalize();
    Vec3f v23 = (v2 + v3).Normalize();
    Vec3f v31 = (v3 + v1).Normalize();
	ProceduralSphere_Subdivide(v1, v12, v31, depth - 1, _faceNormalOrientedOutside);
	ProceduralSphere_Subdivide(v2, v23, v12, depth - 1, _faceNormalOrientedOutside);
	ProceduralSphere_Subdivide(v3, v31, v23, depth - 1, _faceNormalOrientedOutside);
	ProceduralSphere_Subdivide(v12, v23, v31, depth - 1, _faceNormalOrientedOutside);
}

/**************************************************************************/

void Playspace_Mesh::PerformConvexHull(Playspace_Mesh & _sphereMesh, Bool _bFlagNewFaceAsVirtual)
{
	SetEditMode(TRUE);
	
	//SV = sphere vertex ,  MV = mesh vertex

	//1st step: build a Dynarray that will map each SV on a MV
	S32 NbSpherePoints = _sphereMesh.m_TabPoints.GetSize();
	DynArray_Z<S32> daNewPointsRef;
	daNewPointsRef.SetSize(NbSpherePoints);

	//2nd step: for each SV, determine the MV maximizing its projection on the SV's normal according to a criteria and move the SV accordingly
	S32 NbMeshPoints = m_TabPoints.GetSize();
	for(S32 i = 0; i<NbSpherePoints;i++)
	{
		
		Vec3f curPoint = _sphereMesh.m_TabPoints[i];
		float maxValue = -2.f;
		float scalarMax = -2.f;
		for(S32 j =0 ; j<NbMeshPoints;j++)
		{
			float scalar = curPoint*m_TabPoints[j];
			if (scalar < 0.f)
				continue;
			float scalar2 = scalar * scalar;
			float dist2 =  m_TabPoints[j].GetNorm2() - scalar2;
			float valueToTest = scalar2/(0.1f+dist2);

			if(maxValue < valueToTest)
			{
				maxValue = valueToTest;
				scalarMax = scalar;
			}
		}
		_sphereMesh.m_TabPoints[i] = scalarMax *_sphereMesh.m_TabPoints[i];

	}


	//3rd step: add each inflated sphere point to the mesh
	for(S32 i = 0; i<NbSpherePoints;i++)
	{
		daNewPointsRef[i] = AddPoint(_sphereMesh.m_TabPoints[i]);
	}


	//4th step: add each inflated sphere face to the mesh
	S32 NbSphereFaces = _sphereMesh.m_TabQuad.GetSize();

	for(S32 i=0;i<NbSphereFaces;i++)
	{
		// Add Face !
		S32 NumFace = m_TabQuad.Add();
		Playspace_Mesh::Face &CurFace = m_TabQuad[NumFace];

		CurFace.IsTri = TRUE;
		CurFace.IsVirtual = _bFlagNewFaceAsVirtual;

		CurFace.TabPoints[0] = daNewPointsRef[_sphereMesh.m_TabQuad[i].TabPoints[0]];
		CurFace.TabPoints[1] = daNewPointsRef[_sphereMesh.m_TabQuad[i].TabPoints[1]];
		CurFace.TabPoints[2] = daNewPointsRef[_sphereMesh.m_TabQuad[i].TabPoints[2]];
		CurFace.TabPoints[3] = daNewPointsRef[_sphereMesh.m_TabQuad[i].TabPoints[3]];
	}
}

/**************************************************************************/

void	Playspace_Mesh::MarkCone(Vec3f &_Pos,Vec3f &_Dir,Float _HalfAngleDeg,Float _DistMax,Bool _FaceInside,Bool _AtLeastOnePoint)
{
	HU8DA	TabPtView;

	// Mark Points.
	Float ConeCos2 = Cos(DegToRad(_HalfAngleDeg));
	ConeCos2 *= ConeCos2;

	Float DistMax2 = _DistMax*_DistMax;

	S32 NbPoints = m_TabPoints.GetSize();
	TabPtView.SetSize(m_TabPoints.GetSize());
	memset(TabPtView.GetArrayPtr(),0,NbPoints);

	for (S32 i=0 ; i<NbPoints ; i++)
	{
		Vec3f Delta = m_TabPoints[i] - _Pos;
		Float ProjDist = Delta * _Dir;
		Float DeltaLen2 = Delta.GetNorm2();
		if (ProjDist < 0.f)
			continue;
		if (ProjDist*ProjDist < (DeltaLen2*ConeCos2))
			continue;
		if (DeltaLen2 >= DistMax2)
			continue;

		TabPtView[i] = 1;
	}

	// Mark Faces.
	S32 NbFaces = m_TabQuad.GetSize();
	if (!_AtLeastOnePoint)
	{
		// Value wanted .
		S32 OkVal = _FaceInside ? 3 : 0;
		for (S32 i=0 ; i<NbFaces ; i++)
		{
			Playspace_Mesh::Face	&CurFace = m_TabQuad[i];
			S32	*pTabPoints = CurFace.TabPoints;
			U8 IsView = TabPtView[pTabPoints[0]] + TabPtView[pTabPoints[1]] + TabPtView[pTabPoints[2]];	// Only 3 pts...
			CurFace.IsMarked = (IsView == OkVal);
		}

	}
	else
	{
		// Value not wanted .
		S32 NotOkVal = _FaceInside ? 0 : 3;
		for (S32 i=0 ; i<NbFaces ; i++)
		{
			Playspace_Mesh::Face	&CurFace = m_TabQuad[i];
			S32	*pTabPoints = CurFace.TabPoints;
			U8 IsView = TabPtView[pTabPoints[0]] + TabPtView[pTabPoints[1]] + TabPtView[pTabPoints[2]];	// Only 3 pts...
			CurFace.IsMarked = (IsView != NotOkVal);
		}
	}
}

/**************************************************************************/
