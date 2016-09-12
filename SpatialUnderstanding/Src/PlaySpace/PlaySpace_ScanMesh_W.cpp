// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include <pch.h>
#include <PlaySpace\PlaySpaceInfos_W.h>
#include <PlaySpace\PlaySpace_ScanMesh_W.h>
#include <PlaySpace\PlaySpaceInfos_Cst_W.h>
#include <Bresenham2D_Z.h>
#include <FIFO_Z.h>

static const Float CONST_BackFaceDist	= 0.02f;
static const Float CONST_FaceMargin		= 0.005f;
static const Float CONST_FaceMargin2	= (CONST_FaceMargin * 2);

/**************************************************************************/

#define BLINDDETECT_MAXDIST		2 // == 2 + 2 Total size => 32 cm hole is filled. 
#define BLINDDETECT_UPPERDIST	BLINDDETECT_MAXDIST+1
#define BLINDDETECT_CANGO		0x80
#define BLINDDETECT_MSK_DIST	0x7F

BlindDetector::BlindDetector()
{
	m_SizeX	= m_SizeY = m_SizeZ = 0;
}

/**************************************************************************/

void BlindDetector::Init(S32 _SizeX, S32 _SizeY, S32 _SizeZ)
{
	m_SizeX = _SizeX;
	m_SizeY = _SizeY;
	m_SizeZ = _SizeZ;

	S32	Size = _SizeX*_SizeY*_SizeZ;
	TabBlindZone.SetSize(Size);
}

/**************************************************************************/

void BlindDetector::Flush()
{
	Init(0,0,0);
}

/**************************************************************************/
Bool BlindDetector::ComputeBlindZoneFast(QuadTree_ScanMesh *_pMapDatas, Vec3i &_StartPos)
{
	memset(TabBlindZone.GetArrayPtr(), 0xFFFFFFFF, TabBlindZone.GetSize());

	// First Add Blocs.
	PlaySpace_Vec3i pos;
	for (S32 x = 0; x<m_SizeX; x++)
	for (S32 z = 0; z<m_SizeZ; z++)
	{
		S32	Delta = ((z * m_SizeX + x) * m_SizeY);
		QuadTree_ScanMesh::Cell	*pCell = _pMapDatas->GetCell(x, z);
		QuadTree_ScanMesh::ObjectChain *pZone = pCell->pFirst;
		while (pZone)
		{
			if (pZone->pFirstRefFace)
			{
				// Add a bloc on BlindZone.
				pos.x = x;
				pos.y = pZone->hMin;
				pos.z = z;
				TabBlindZone[Delta + pZone->hMin] = 0;
			}
			pZone = pZone->pNext;
		}
	}

	return TRUE;
}

/**************************************************************************/

Bool BlindDetector::ComputeBlindZone(QuadTree_ScanMesh *_pMapDatas, Vec3i &_StartPos)
{
	FIFO_Z<PlaySpace_Vec3i, 4096, FALSE>	FIFOCell;
	static S32		TabDelta[] = { 1, 0, 0,
		-1, 0, 0,
		0, 1, 0,
		0, -1, 0,
		0, 0, 1,
		0, 0, -1
	};

	// Fill with MaxDist + 1
	S32 FillDist = BLINDDETECT_UPPERDIST;
	FillDist += (FillDist << 8);
	FillDist += (FillDist << 16);
	memset(TabBlindZone.GetArrayPtr(), FillDist, TabBlindZone.GetSize());

	// Set Border to 1.
	U8 *pStartCell = TabBlindZone.GetArrayPtr();
	S32 ModuloZ = m_SizeY*m_SizeX;
	for (S32 z = 0; z<m_SizeZ; z++)
	{
		U8	*pCurCell = pStartCell;
		for (S32 y = 0; y < m_SizeY; y++)
			*pCurCell++ = 1;

		pCurCell = pStartCell + (m_SizeX - 1) * m_SizeY;
		for (S32 y = 0; y < m_SizeY; y++)
			*pCurCell++ = 1;

		pStartCell += ModuloZ;
	}

	pStartCell = TabBlindZone.GetArrayPtr() + m_SizeY;
	for (S32 x = 2; x<m_SizeX; x++)
	{
		U8	*pCurCell = pStartCell;
		for (S32 y = 0; y < m_SizeY; y++)
			*pCurCell++ = 1;

		pCurCell = pStartCell + (m_SizeZ - 1) * ModuloZ;
		for (S32 y = 0; y < m_SizeY; y++)
			*pCurCell++ = 1;

		pStartCell += m_SizeY;
	}

	// First Add Bloc.
	for (S32 x = 0; x<m_SizeX; x++)
	for (S32 z = 0; z<m_SizeZ; z++)
	{
		S32	Delta = ((z * m_SizeX + x) * m_SizeY);
		QuadTree_ScanMesh::Cell	*pCell = _pMapDatas->GetCell(x, z);
		QuadTree_ScanMesh::ObjectChain *pZone = pCell->pFirst;
		while (pZone)
		{
			if (pZone->pFirstRefFace)
			{
				// Add a bloc on BlindZone.
				PlaySpace_Vec3i *pos = FIFOCell.Push();
				pos->x = x;
				pos->y = pZone->hMin;
				pos->z = z;
				TabBlindZone[Delta + pZone->hMin] = 0;
			}
			pZone = pZone->pNext;
		}
	}

	// Pre Filter CanGo => suppress Empty Zone from Ground to Ceiling.
	pStartCell = TabBlindZone.GetArrayPtr();
	for (S32 z = 0; z<m_SizeZ; z++)
	{
		for (S32 x = 0; x<m_SizeX; x++)
		{
			Bool TotalEmpty = TRUE;
			U8	*pCurCell = pStartCell;
			for (S32 y = 0; y < m_SizeY; y++)
			{
				if (!*pCurCell)
				{
					TotalEmpty = FALSE;
					break;
				}
				pCurCell++;
			}
			if (TotalEmpty)
			{
				pCurCell = pStartCell;
				for (S32 y = 0; y < m_SizeY; y++)
				{
					*pCurCell++ = 1;
				}
			}
			// Next !
			pStartCell += m_SizeY;
		}
	}

	// Now Do the Dist Compute
	// => Largeur d'abord !!! (Important, permet de garantir que la distance est bonne !)
	// => Use diagonal !!!
	for (;;)
	{
		// Pop.
		PlaySpace_Vec3i CurPos;
		if (!FIFOCell.Pop(CurPos))
			break;	// Nothng to do.

		// Manage it.
		S32 x = CurPos.x;
		S32 y = CurPos.y;
		S32 z = CurPos.z;

		U8 *pMyCell = GetCell(x, y, z);
		S32 NextVal = *pMyCell + 1;

		for (S32 dz = -1; dz <= 1; dz++)
		{
			S32 cz = z + dz;
			if ((cz < 0) || (cz >= m_SizeZ))
				continue;
			for (S32 dx = -1; dx <= 1; dx++)
			{
				S32 cx = x + dx;
				if ((cx < 0) || (cx >= m_SizeX))
					continue;
				for (S32 dy = -1; dy <= 1; dy++)
				{
					S32 cy = y + dy;
					if ((cy < 0) || (cy >= m_SizeY))
						continue;

					U8 *pOtherCell = GetCell(cx, cy, cz);
					if (*pOtherCell == BLINDDETECT_UPPERDIST)
					{
						if (NextVal < BLINDDETECT_MAXDIST)
						{
							PlaySpace_Vec3i *pos = FIFOCell.Push();
							pos->x = cx;
							pos->y = cy;
							pos->z = cz;
						}
						*pOtherCell = NextVal;
					}
				}
			}
		}
	}

	// Modify StartPos.
	Vec3i GoodStartPos = _StartPos;
	S32 GCurVal = *GetCell(GoodStartPos.x, GoodStartPos.y, GoodStartPos.z);
	while (GCurVal != BLINDDETECT_UPPERDIST)
	{
		if (GoodStartPos.x > 0)
		{
			S32 OtherVal = *GetCell(GoodStartPos.x - 1, GoodStartPos.y, GoodStartPos.z);
			if (OtherVal > GCurVal)
			{
				GoodStartPos.x--;
				GCurVal = OtherVal;
				continue;
			}
		}
		if (GoodStartPos.x < (m_SizeX - 1))
		{
			S32 OtherVal = *GetCell(GoodStartPos.x + 1, GoodStartPos.y, GoodStartPos.z);
			if (OtherVal > GCurVal)
			{
				GoodStartPos.x++;
				GCurVal = OtherVal;
				continue;
			}
		}
		if (GoodStartPos.y > 0)
		{
			S32 OtherVal = *GetCell(GoodStartPos.x, GoodStartPos.y - 1, GoodStartPos.z);
			if (OtherVal > GCurVal)
			{
				GoodStartPos.y--;
				GCurVal = OtherVal;
				continue;
			}
		}
		if (GoodStartPos.y < (m_SizeY - 1))
		{
			S32 OtherVal = *GetCell(GoodStartPos.x, GoodStartPos.y + 1, GoodStartPos.z);
			if (OtherVal > GCurVal)
			{
				GoodStartPos.y++;
				GCurVal = OtherVal;
				continue;
			}
		}
		if (GoodStartPos.z > 0)
		{
			S32 OtherVal = *GetCell(GoodStartPos.x, GoodStartPos.y, GoodStartPos.z - 1);
			if (OtherVal > GCurVal)
			{
				GoodStartPos.z--;
				GCurVal = OtherVal;
				continue;
			}
		}
		if (GoodStartPos.z < (m_SizeZ - 1))
		{
			S32 OtherVal = *GetCell(GoodStartPos.x, GoodStartPos.y, GoodStartPos.z + 1);
			if (OtherVal > GCurVal)
			{
				GoodStartPos.z++;
				GCurVal = OtherVal;
				continue;
			}
		}
		// Lock ! :(
		return FALSE;
	}

	// Fill The _CanGoArea (can't go into little hole).
	// => Largeur d'abord !!! Important.
	// => Use diagonal !!! => Important to stop 
	PlaySpace_Vec3i *pos = FIFOCell.Push();
	pos->x = GoodStartPos.x;
	pos->y = GoodStartPos.y;
	pos->z = GoodStartPos.z;
	*GetCell(GoodStartPos.x, GoodStartPos.y, GoodStartPos.z) |= BLINDDETECT_CANGO;

	S32 StopYGround = _pMapDatas->m_hGround;
	S32 StopYCeiling = _pMapDatas->m_hCeiling;

	for (;;)
	{
		// Pop.
		PlaySpace_Vec3i CurPos;
		if (!FIFOCell.Pop(CurPos))
			break;	// Nothng to do.

		// Manage it.
		S32 x = CurPos.x;
		S32 y = CurPos.y;
		S32 z = CurPos.z;

		U8		*pMyCell = GetCell(x, y, z);
		S32		CurPrevVal = (*pMyCell & BLINDDETECT_MSK_DIST) - 1;

		for (S32 dz = -1; dz <= 1; dz++)
		{
			S32 cz = z + dz;
			if ((cz < 0) || (cz >= m_SizeZ))
				continue;
			for (S32 dx = -1; dx <= 1; dx++)
			{
				S32 cx = x + dx;
				if ((cx < 0) || (cx >= m_SizeX))
					continue;
				for (S32 dy = -1; dy <= 1; dy++)
				{
					S32 cy = y + dy;
					if ((cy < 0) || (cy >= m_SizeY))
						continue;
					if ((cy < StopYGround) || (cy > StopYCeiling))
						continue;

					U8 *pOtherCell = GetCell(cx, cy, cz);
					S32 OtherVal = *pOtherCell;
					if ((OtherVal == BLINDDETECT_UPPERDIST) || (OtherVal == CurPrevVal))
					{
						if (OtherVal > 1)	// Important because of diagonale.
						{
							PlaySpace_Vec3i *lpos = FIFOCell.Push();
							lpos->x = cx;
							lpos->y = cy;
							lpos->z = cz;
						}
						*pOtherCell = OtherVal | BLINDDETECT_CANGO;
					}
				}
			}
		}
	}

	// Finally : Fill the Visible Area
	const  S32 FlagEmpty = 0x01;
	const  S32 FlagPX = 0x02;
	const  S32 FlagNX = 0x04;
	const  S32 FlagPY = 0x08;
	const  S32 FlagNY = 0x10;
	const  S32 FlagPZ = 0x20;
	const  S32 FlagNZ = 0x40;
	const  S32 FlagTotal = FlagPX + FlagPY + FlagPZ + FlagNX + FlagNY + FlagNZ + FlagEmpty;

	S32	Size = TabBlindZone.GetSize();
	U8 *pCurCell = TabBlindZone.GetArrayPtr();
	for (S32 t = 0; t<Size; t++)
	{
		if (*pCurCell)
		{
			*pCurCell = (*pCurCell & BLINDDETECT_CANGO) ? FlagTotal : FlagEmpty;
		}
		pCurCell++;
	}

	EXCEPTIONC_Z(pCurCell <= (TabBlindZone.GetArrayPtr() + Size), "ComputeBlindZone : Out of buffer");
	Bool SomethingToDo = TRUE;
	while (SomethingToDo)
	{
		SomethingToDo = FALSE;
		S32 SizeBase = m_SizeX * m_SizeY;
		U8 *pCurCellP = TabBlindZone.GetArrayPtr();
		U8 *pCurCellN = TabBlindZone.GetArrayPtr() + Size;
		for (S32 z = 0; z<m_SizeZ; z++)
		{
			for (S32 x = 0; x<m_SizeX; x++)
			{
				for (S32 y = 0; y<m_SizeY; y++)
				{
					// Positive propagation.
					S32 CurVal = *pCurCellP;
					if (CurVal)
					{
						if (y && (pCurCellP[-1] & FlagPY))
							CurVal |= FlagPY;
						if (x && (pCurCellP[-m_SizeY] & FlagPX))
							CurVal |= FlagPX;
						if (z && (pCurCellP[-SizeBase] & FlagPZ))
							CurVal |= FlagPZ;
						*pCurCellP = CurVal;
					}
					pCurCellP++;

					// Negative propagation
					pCurCellN--;
					CurVal = *pCurCellN;
					if (CurVal)
					{
						if (y && (pCurCellN[1] & FlagNY))
							CurVal |= FlagNY;
						if (x && (pCurCellN[m_SizeY] & FlagNX))
							CurVal |= FlagNX;
						if (z && (pCurCellN[SizeBase] & FlagNZ))
							CurVal |= FlagNZ;
						*pCurCellN = CurVal;
					}
				}
			}
		}
		EXCEPTIONC_Z(pCurCellP == (TabBlindZone.GetArrayPtr() + Size), "ComputeBlindZone : Out of buffer");
		EXCEPTIONC_Z(pCurCellN == (TabBlindZone.GetArrayPtr()), "ComputeBlindZone : Out of buffer");

		// Transform specific value into CANGO.
		pCurCell = TabBlindZone.GetArrayPtr();
		for (S32 t = 0; t<Size; t++)
		{
			S32 CurVal = *pCurCell;
			if ((CurVal > 1) && (CurVal != FlagTotal))
			{
				S32 NbBits = 0;
				if (CurVal & FlagNX)
					NbBits++;
				if (CurVal & FlagPX)
					NbBits++;
				if (CurVal & FlagNY)
					NbBits++;
				if (CurVal & FlagPY)
					NbBits++;
				if (CurVal & FlagNZ)
					NbBits++;
				if (CurVal & FlagPZ)
					NbBits++;
				if (NbBits > 1)
				{
					SomethingToDo = TRUE;
					*pCurCell = FlagTotal;
				}
			}
			pCurCell++;
		}
	}

	return TRUE;
}

/**************************************************************************/

void BlindDetector::Draw(Vec3f &_PosMin, Float _SizeCell)
{
	//	S32 NbSphere = 200;
	U8 *pCurCell = TabBlindZone.GetArrayPtr();
	S32 NumStart;
	U8  ValStart;
	for (S32 z = 0; z<m_SizeZ; z++)
	{
		for (S32 x = 0; x<m_SizeX; x++)
		{
			NumStart = -1;
			ValStart = 0;
			for (S32 y = 0; y <= m_SizeY; y++)
			{
				U8 val = 0;
				if (y < m_SizeY)
					val = *pCurCell++;

				// End Draw ?
				if (NumStart >= 0)
				{
					Bool DrawIt = FALSE;

					if (val != ValStart)
						DrawIt = TRUE;
					else if (y == (m_SizeY - 1))
					{
						y++;
						DrawIt = TRUE;
					}

					// DrawIt
					if (DrawIt)
					{
						// End.
						Vec3f	start;
						start.x = (0.5f + (Float)x)*_SizeCell + _PosMin.x;
						start.y = ((Float)NumStart)*_SizeCell + _PosMin.y;
						start.z = (0.5f + (Float)z)*_SizeCell + _PosMin.z;

						Vec3f	end;
						end.x = (0.5f + (Float)x)*_SizeCell + _PosMin.x;
						end.y = ((Float)y)*_SizeCell + _PosMin.y;
						end.z = (0.5f + (Float)z)*_SizeCell + _PosMin.z;
						if (ValStart)
						{
							DRAW_DEBUG_LINE3D(start, end, COLOR_RED, 0.02f, .displayDuration(1000.f));
						}
						else
						{
							DRAW_DEBUG_LINE3D(start, end, COLOR_BLUE, 0.02f, .displayDuration(1000.f));
						}
						NumStart = -1;
					}
				}

				// New Draw ?
				if ((NumStart<0) && (val <= 1))
				{
					ValStart = val;
					NumStart = y;
				}
			}
		}
	}
}

/**************************************************************************/

HMapMeshInfos3D::HMapMeshInfos3D()
{
	m_pMapMeshDatas = NULL;
	m_pMapResultDatas = NULL;
	m_LastDeviceVolume = 0;
}

/**************************************************************************/

HMapMeshInfos3D::~HMapMeshInfos3D()
{
	Flush();
}


/**************************************************************************/

void	HMapMeshInfos3D::Flush()
{
	if (m_pMapMeshDatas)
	{
		Delete_Z m_pMapMeshDatas;
		m_pMapMeshDatas = NULL;
	}
	if (m_pMapResultDatas)
	{
		Delete_Z m_pMapResultDatas;
		m_pMapResultDatas = NULL;
	}


	m_BlindDetector.Flush();
	m_ScanReport.Flush();
}

/**************************************************************************/

void	HMapMeshInfos3D::Init(Playspace_Area &_Area, Float _YGround, Float _YCeiling)
{
	HMapBaseInfos3D::Init(_Area);

	// Mesh.
	if (m_pMapMeshDatas)
	{
		Delete_Z m_pMapMeshDatas;
		m_pMapMeshDatas = NULL;
	}

	m_pMapMeshDatas = New_Z	QuadTree_ScanMesh(m_NbCellX, m_NbCellZ);
	m_pMapMeshDatas->m_hMin = 0;
	m_pMapMeshDatas->m_hMax = (S32)((_Area.Max.y - _Area.Min.y) / _Area.SizeVoxel);
	m_pMapMeshDatas->m_hGround = (S32)((_YGround - _Area.Min.y) / _Area.SizeVoxel);
	m_pMapMeshDatas->m_hCeiling = (S32)((_YCeiling - _Area.Min.y) / _Area.SizeVoxel);

	// Result.
	if (m_pMapResultDatas)	
	{
		Delete_Z m_pMapResultDatas;
		m_pMapResultDatas = NULL;
	}
	m_pMapResultDatas = New_Z	QuadTree_KeepScan(_Area.NbCellX+1, _Area.NbCellY+1);
}

/**************************************************************************/

void	HMapMeshInfos3D::Move(Playspace_Area &_Area,Float _YGround,Float _YCeiling)
{
	// Move or Init ?
	if (!m_pMapResultDatas || !m_pMapMeshDatas)
	{
		Init(_Area,_YGround,_YCeiling);
		return;
	}

	// No modification ?
	Vec3f DeltaMin = _Area.Min - m_PosMin;
	if ((_Area.NbCellX == m_NbCellX) && (_Area.NbCellY == m_NbCellZ))
	{
		if (DeltaMin.GetNorm() < 0.01f)
			return;
	}

	// Check snap.
	Float	InvSizeCell = 1.f / m_SizeCell;
	Vec3f	fDeltaMinVoxel = DeltaMin * InvSizeCell;
	
	EXCEPTIONC_Z(Abs(ROUNDF(fDeltaMinVoxel.x) - fDeltaMinVoxel.x) < 0.001f,"HMapMeshInfos3D::Move => Not snapped pos.");
	EXCEPTIONC_Z(Abs(ROUNDF(fDeltaMinVoxel.y) - fDeltaMinVoxel.y) < 0.001f,"HMapMeshInfos3D::Move => Not snapped pos.");
	EXCEPTIONC_Z(Abs(ROUNDF(fDeltaMinVoxel.z) - fDeltaMinVoxel.z) < 0.001f,"HMapMeshInfos3D::Move => Not snapped pos.");

	Vec3i iDeltaMinVoxel(ROUNDINT(fDeltaMinVoxel.x),ROUNDINT(fDeltaMinVoxel.y),ROUNDINT(fDeltaMinVoxel.z));

	// Manage X-Z move.
	if (iDeltaMinVoxel.x || iDeltaMinVoxel.z || (_Area.NbCellX != m_NbCellX) || (_Area.NbCellY != m_NbCellZ))
	{
		// Compute Deltas
		S32 NbCellX;
		S32 DeltaSrcX;
		S32 DeltaDstX;
		if (iDeltaMinVoxel.x > 0)
		{
			DeltaSrcX = iDeltaMinVoxel.x;
			DeltaDstX = 0;
			NbCellX = Min(_Area.NbCellX,m_NbCellX-iDeltaMinVoxel.x);
		}
		else
		{
			DeltaSrcX = 0;
			DeltaDstX = -iDeltaMinVoxel.x;
			NbCellX = Min(m_NbCellX,_Area.NbCellX+iDeltaMinVoxel.x);
		}

		S32 NbCellY;
		S32 DeltaSrcY;
		S32 DeltaDstY;
		if (iDeltaMinVoxel.z > 0)
		{
			DeltaSrcY = iDeltaMinVoxel.z;
			DeltaDstY = 0;
			NbCellY = Min(_Area.NbCellY,m_NbCellZ-iDeltaMinVoxel.z);
		}
		else
		{
			DeltaSrcY = 0;
			DeltaDstY = -iDeltaMinVoxel.z;
			NbCellY = Min(m_NbCellZ,_Area.NbCellY+iDeltaMinVoxel.z);
		}

		// Result X-Z Move.
		DynArray_Z<QuadTree_KeepScan::Cell, 256, TRUE, TRUE>	NewResultBoard;
		NewResultBoard.SetSize((_Area.NbCellX+1)*(_Area.NbCellY+1));
		NewResultBoard.Null();

		for (S32 y = 0 ; y<=NbCellY ; y++)	// = because +1
		{
			QuadTree_KeepScan::Cell *pSrcCell = m_pMapResultDatas->m_CellBoard.GetArrayPtr() + (y+DeltaSrcY)*m_pMapResultDatas->m_SizeX + DeltaSrcX;
			QuadTree_KeepScan::Cell *pDstCell = NewResultBoard.GetArrayPtr() + (y+DeltaDstY)*(_Area.NbCellX+1) + DeltaDstX;

			for (S32 x = 0 ; x<=NbCellX ; x++)
			{
				pDstCell->pFirst = pSrcCell->pFirst;
				pSrcCell->pFirst = NULL;
				pDstCell++;
				pSrcCell++;
			}
		}
		S32 Size = m_pMapResultDatas->m_CellBoard.GetSize();
		QuadTree_KeepScan::Cell *pCellR = m_pMapResultDatas->m_CellBoard.GetArrayPtr();
		for (S32 i=0 ; i < Size; i++)
		{
			if (pCellR->pFirst)
				m_pMapResultDatas->FlushCell(pCellR);
			pCellR++;
		}
		m_pMapResultDatas->m_CellBoard.Swap(NewResultBoard);
		m_pMapResultDatas->m_SizeX = _Area.NbCellX+1;
		m_pMapResultDatas->m_SizeY = _Area.NbCellY+1;		
		NewResultBoard.Flush();

		// Suppress Useless datas.

		// Mesh Datas Move.
		DynArray_Z<QuadTree_ScanMesh::Cell, 256, TRUE, TRUE>	NewMeshBoard;
		NewMeshBoard.SetSize((_Area.NbCellX)*(_Area.NbCellY));
		NewMeshBoard.Null();

		for (S32 y = 0 ; y<NbCellY ; y++)	// = because +1
		{
			QuadTree_ScanMesh::Cell *pSrcCell = m_pMapMeshDatas->m_CellBoard.GetArrayPtr() + (y+DeltaSrcY)*m_NbCellX + DeltaSrcX;
			QuadTree_ScanMesh::Cell *pDstCell = NewMeshBoard.GetArrayPtr() + (y+DeltaDstY)*_Area.NbCellX + DeltaDstX;

			for (S32 x = 0 ; x<NbCellX ; x++)
			{
				pDstCell->pFirst = pSrcCell->pFirst;
				pSrcCell->pFirst = NULL;
				pDstCell++;
				pSrcCell++;
			}
		}
		Size = m_pMapMeshDatas->m_CellBoard.GetSize();
		QuadTree_ScanMesh::Cell *pCellM = m_pMapMeshDatas->m_CellBoard.GetArrayPtr();
		for (S32 i=0 ; i < Size; i++)
		{
			QuadTree_ScanMesh::ObjectChain *pZone = pCellM->pFirst;
			while (pZone)
			{
				QuadTree_ScanMesh::ObjectChain *pNext = pZone->pNext;
				m_pMapMeshDatas->DeleteZone(pZone);
				pZone = pNext;
			}
			pCellM->pFirst = NULL;
			pCellM++;
		}
		m_pMapMeshDatas->m_CellBoard.Swap(NewMeshBoard);
		m_pMapMeshDatas->m_SizeX = _Area.NbCellX;
		m_pMapMeshDatas->m_SizeY = _Area.NbCellY;
		NewMeshBoard.Flush();
	}

	// Manage H Move.
	if (iDeltaMinVoxel.y)
	{
		// Result H Move.
		S32 Size = m_pMapResultDatas->m_CellBoard.GetSize();
		QuadTree_KeepScan::Cell *pCellR = m_pMapResultDatas->m_CellBoard.GetArrayPtr();
		for (S32 i=0 ; i < Size; i++)
		{
			QuadTree_KeepScan::ObjectChain *pZone = pCellR->pFirst;
			while (pZone)
			{
				pZone->hPos -= iDeltaMinVoxel.y;
				pZone = pZone->pNext;
			}
			pCellR++;
		}

		// Mesh Datas H Move.
		Size = m_pMapMeshDatas->m_CellBoard.GetSize();
		QuadTree_ScanMesh::Cell *pCellM = m_pMapMeshDatas->m_CellBoard.GetArrayPtr();
		for (S32 i=0 ; i < Size; i++)
		{
			QuadTree_ScanMesh::ObjectChain *pZone = pCellM->pFirst;
			while (pZone)
			{
				pZone->hMin -= iDeltaMinVoxel.y;
				pZone->hMax -= iDeltaMinVoxel.y;
				pZone = pZone->pNext;
			}
			pCellM++;
		}
	}

	// Finalize Move.
	HMapBaseInfos3D::Move(_Area.Min,_Area.Max,_Area.NbCellX,_Area.NbCellH,_Area.NbCellY);   // TUTUTUTUTUTUTUTUTUTUTUTUTUTUTUTU
	m_pMapMeshDatas->m_hMin = 0;
	m_pMapMeshDatas->m_hMax = (S32)((_Area.Max.y - _Area.Min.y) / _Area.SizeVoxel);
	m_pMapMeshDatas->m_hGround = (S32)((_YGround - _Area.Min.y) / _Area.SizeVoxel);
	m_pMapMeshDatas->m_hCeiling = (S32)((_YCeiling - _Area.Min.y) / _Area.SizeVoxel);
}
	
/**************************************************************************/

void	HMapMeshInfos3D::AddMesh(Playspace_Mesh *_pMesh,Bool _FastMode,Bool _KeepMode)
{
	EXCEPTIONC_Z(m_pMapMeshDatas != NULL, "HMapRayInfos3D::AddMeshAsRefFace : No m_pMapMeshDatas");

	if (_FastMode)
		_pMesh->ComputeFacesAndPointToolNormalForMarkedFaces();
	else
	{
		_pMesh->ComputePointsLinks(FALSE);
		_pMesh->ComputeFacesToolNormal(FALSE);
		_pMesh->ComputePointsToolNormal(FALSE);	// Finalement OUI : on ne garde pas les PlanarFilter_Accurate Normal car les erreurs sont fausses.
	}

	// Flush it !
	if (!_KeepMode)
	{
		m_pMapMeshDatas->Empty();
		m_pMapResultDatas->Empty();
	}

	// Add Mesh.
	S32		NbFaces = _pMesh->m_TabQuad.GetSize();
	Float	InvSizeCell = 1.f / m_SizeCell;
	Float	CosFlat = 0.9f;

	for (S32 i = 0; i < NbFaces; i++)
	{
		Playspace_Mesh::Face &Face = _pMesh->m_TabQuad[i];
		EXCEPTIONC_Z(Face.IsTri, "HMapMeshInfos3D::AddMeshAsFaces => Only Tri");
		if (_FastMode && !Face.IsMarked)
			continue;	// Ignore non marked faces.
		if (_pMesh->m_TabFaceToolNormal[i].Surface < (0.001f * 0.001f))
			continue;	// Igonre very small faces.

		EXCEPTIONC_Z(_pMesh->m_TabFaceToolNormal[i].Surface >= 0.f,"ERROR NORMAL NORMAL");
		Vec3f	Normal = _pMesh->m_TabFaceToolNormal[i].Normal;	// On ajoute que dans la case du centre.

		Vec3f p0 = _pMesh->m_TabPoints[Face.TabPoints[0]];
		Vec3f p1 = _pMesh->m_TabPoints[Face.TabPoints[1]];
		Vec3f p2 = _pMesh->m_TabPoints[Face.TabPoints[2]];

		Bool IsFlat = TRUE;
		for (S32 i = 0; i<3; i++)
		{
			EXCEPTIONC_Z(_pMesh->m_TabPointsToolNormal[Face.TabPoints[i]].Surface >= 0.f,"ERROR NORMAL PT");

			if (_pMesh->m_TabPointsToolNormal[Face.TabPoints[i]].Error < CosFlat)
			{
				IsFlat = FALSE;
				break;
			}
		}
		Vec3f	MinPos = p0;
		Vec3f	MaxPos = p0;
		MinPos = Min(MinPos, p1);
		MaxPos = Max(MaxPos, p1);
		MinPos = Min(MinPos, p2);
		MaxPos = Max(MaxPos, p2);

		// Add Margin.
		MinPos.x -= CONST_FaceMargin;
		MinPos.y -= CONST_FaceMargin;
		MinPos.z -= CONST_FaceMargin;

		MaxPos.x += CONST_FaceMargin;
		MaxPos.y += CONST_FaceMargin;
		MaxPos.z += CONST_FaceMargin;

		Vec3i iMin,iMax;
		iMin.x = FLOORF((MinPos.x - m_PosMin.x) * InvSizeCell);
		iMin.y = FLOORF((MinPos.y - m_PosMin.y) * InvSizeCell);
		iMin.z = FLOORF((MinPos.z - m_PosMin.z) * InvSizeCell);

		iMax.x = FLOORF((MaxPos.x - m_PosMin.x) * InvSizeCell);
		iMax.y = FLOORF((MaxPos.y - m_PosMin.y) * InvSizeCell);
		iMax.z = FLOORF((MaxPos.z - m_PosMin.z) * InvSizeCell);

		PlaySpace_ScanMeshFace *pCreatedFace = NULL;

		// Fast Insertion ?
		Vec3i iDelta = iMax - iMin;
		if (!iDelta.x && !iDelta.y)
		{
			if ((iMin.x < 0) || (iMin.x >= m_NbCellX))
				continue;
			if ((iMin.y < 0) || (iMin.y >= m_NbCellY))
				continue;
			// Linear Z => No test
			for (S32 z = iMin.z; z <= iMax.z; z++)
			{
				if ((z < 0) || (z >= m_NbCellZ))
					continue;
				PlaySpace_HVoxelZone *pZone = m_pMapMeshDatas->GetZone(iMin.x, iMin.y, z, TRUE);
				if (_KeepMode && pZone->IsKeepBlock)
					continue;
				m_pMapMeshDatas->AddFace(p0,p1,p2,Normal,Face,IsFlat,pZone,pCreatedFace);
			}
		}
		else if (!iDelta.x && !iDelta.z)
		{
			if ((iMin.x < 0) || (iMin.x >= m_NbCellX))
				continue;
			if ((iMin.z < 0) || (iMin.z >= m_NbCellZ))
				continue;
			// Linear Z => No test
			for (S32 y = iMin.y; y <= iMax.y; y++)
			{
				if ((y < 0) || (y >= m_NbCellY))
					continue;
				PlaySpace_HVoxelZone *pZone = m_pMapMeshDatas->GetZone(iMin.x, y, iMin.z, TRUE);
				if (_KeepMode && pZone->IsKeepBlock)
					continue;
				m_pMapMeshDatas->AddFace(p0,p1,p2,Normal,Face,IsFlat,pZone,pCreatedFace);
			}
		}
		else if (!iDelta.y && !iDelta.z)
		{
			if ((iMin.y < 0) || (iMin.y >= m_NbCellY))
				continue;
			if ((iMin.z < 0) || (iMin.z >= m_NbCellZ))
				continue;
			// Linear Z => No test
			for (S32 x = iMin.x; x <= iMax.x; x++)
			{
				if ((x < 0) || (x >= m_NbCellX))
					continue;
				PlaySpace_HVoxelZone *pZone = m_pMapMeshDatas->GetZone(x, iMin.y, iMin.z, TRUE);
				if (_KeepMode && pZone->IsKeepBlock)
					continue;
				m_pMapMeshDatas->AddFace(p0,p1,p2,Normal,Face,IsFlat,pZone,pCreatedFace);
			}
		}
		else
		{
			Vec3f	vMin;
			Vec3f	vMax;
			vMin.z = iMin.z * m_SizeCell + m_PosMin.z - CONST_FaceMargin;
			vMax.z = vMin.z + m_SizeCell + CONST_FaceMargin2;
			for (S32 z = iMin.z; z <= iMax.z; z++, vMin.z += m_SizeCell, vMax.z += m_SizeCell)
			{
				if ((z < 0) || (z >= m_NbCellZ))
					continue;
				vMin.y = iMin.y * m_SizeCell + m_PosMin.y - CONST_FaceMargin;
				vMax.y = vMin.y + m_SizeCell + CONST_FaceMargin2;
				for (S32 y = iMin.y; y <= iMax.y; y++, vMin.y += m_SizeCell, vMax.y += m_SizeCell)
				{
					if ((y < 0) || (y >= m_NbCellY))
						continue;
					vMin.x = iMin.x * m_SizeCell + m_PosMin.x - CONST_FaceMargin;
					vMax.x = vMin.x + m_SizeCell + CONST_FaceMargin2;
					for (S32 x = iMin.x; x <= iMax.x; x++, vMin.x += m_SizeCell, vMax.x += m_SizeCell)
					{
						if ((x < 0) || (x >= m_NbCellX))
							continue;

						// Box Vs Tri.
						if (!AABoxVsTriangle(vMin, vMax, p0, p1, p2))
							continue;

						// Get Zone.
						PlaySpace_HVoxelZone *pZone = m_pMapMeshDatas->GetZone(x, y, z,TRUE);
						if (_KeepMode && pZone->IsKeepBlock)
							continue;
						m_pMapMeshDatas->AddFace(p0,p1,p2,Normal,Face,IsFlat,pZone,pCreatedFace);
					}
				}
			}
		}
	}
}

/**************************************************************************/

void	HMapMeshInfos3D::ClearConeReport(Vec3f &_Pos,Vec3f &_Dir,Float _HalfAngleDeg,Float _DistMax)
{
	// Angle.
	Vec2f  ConeSC;
	SinCos(ConeSC,DegToRad(_HalfAngleDeg));

	Float ConeCos2 = ConeSC.y;
	ConeCos2 *= ConeCos2;

	Float ConeSin = ConeSC.x;

	// Modify Cone for Sphere Margin.
	Float	HalfSizeCell = m_SizeCell*0.5f;
	Float	RadiusSize = HalfSizeCell * 1.732f;
	Float	OriginDistMargin = RadiusSize / ConeSin;
	Vec3f	OriginPos = _Pos + _Dir * OriginDistMargin;

	Float	DistMaxCone = _DistMax - OriginDistMargin - RadiusSize;
	Float	DistMaxCone2 = DistMaxCone * DistMaxCone;

	// First Flush inside cone blocks + Empty blocs.
	Vec3f	BasePos(m_PosMin.x +HalfSizeCell , m_PosMin.y +HalfSizeCell , m_PosMin.z +HalfSizeCell);

	Vec3f			Pos3D;
	for (S32 x = 0; x<m_NbCellX; x++)
	{
		Pos3D.x = BasePos.x + x * m_SizeCell;
		for (S32 z = 0; z<m_NbCellZ; z++)
		{
			QuadTree_ScanMesh::Cell	*pCell = m_pMapMeshDatas->GetCell(x, z);
			if (!pCell->pFirst)
				continue;

			Pos3D.z = BasePos.z + z * m_SizeCell;

			QuadTree_ScanMesh::ObjectChain **ppZone = &(pCell->pFirst);
			do {
				S32 iH = (*ppZone)->hMin;
				if (!(*ppZone)->pFirstRefFace || (iH<0) || (iH >= m_NbCellY))
				{
					// Relink and Supprress bloc !
					QuadTree_ScanMesh::ObjectChain *pDelete = *ppZone;
					*ppZone = (*ppZone)->pNext;
					m_pMapMeshDatas->DeleteZone(pDelete);
					continue;
				}

				// 3D Pos.
				Pos3D.y = BasePos.y + (*ppZone)->hMin * m_SizeCell;

				Vec3f Delta = Pos3D - OriginPos;
				Float ProjDist = Delta * _Dir;
				Float DeltaLen2 = Delta.GetNorm2();

				if (   (ProjDist >= 0.f)
					&& (ProjDist*ProjDist >= (DeltaLen2*ConeCos2))
					&& (DeltaLen2 <= DistMaxCone2)
					)
				{
					// Inside Cone.

					// Relink.
					QuadTree_ScanMesh::ObjectChain *pDelete = *ppZone;
					*ppZone = (*ppZone)->pNext;

					// Recycle RefFaces and Supprress bloc !
					m_pMapMeshDatas->DeleteZone(pDelete);
					continue;
				}

				// Current is mark As Keep.
				(*ppZone)->IsKeepBlock = 1;

				// Next.
				ppZone = &((*ppZone)->pNext);
			} while (*ppZone);
		}
	}

	// Flush inside cone result.
	S32 ResultMaxX = m_NbCellX+1;
	S32 ResultMaxZ = m_NbCellZ+1;
	for (S32 x = 0; x<ResultMaxX; x++)
	{
		for (S32 z = 0; z<ResultMaxZ; z++)
		{
			QuadTree_KeepScan::Cell	*pCell = m_pMapResultDatas->GetCell(x, z);
			if (!pCell->pFirst)
				continue;
			
			QuadTree_KeepScan::ObjectChain **ppZone = &(pCell->pFirst);
			do {
				// Used ?
				S32 iH = (*ppZone)->hPos;
				if (!(*ppZone)->bIsUsed || (iH<0) || (iH >= m_NbCellY))
				{
					// Relink and Supprress bloc !
					QuadTree_KeepScan::ObjectChain *pDelete = *ppZone;
					*ppZone = (*ppZone)->pNext;
					m_pMapResultDatas->Delete(pDelete);
					continue;
				}

				// 3D Pos.
				Vec3f Delta = (*ppZone)->CubicPos - OriginPos;
				Float ProjDist = Delta * _Dir;
				Float DeltaLen2 = Delta.GetNorm2();

				if (   (ProjDist >= 0.f)
					&& (ProjDist*ProjDist >= (DeltaLen2*ConeCos2))
					&& (DeltaLen2 <= DistMaxCone2)
					)
				{
					// Relink and Supprress bloc !
					QuadTree_KeepScan::ObjectChain *pDelete = *ppZone;
					*ppZone = (*ppZone)->pNext;
					m_pMapResultDatas->Delete(pDelete);
					continue;
				}

				// Set as UnUsed.
				(*ppZone)->bIsUsed = FALSE;

				// Next.
				ppZone = &((*ppZone)->pNext);
			} while (*ppZone);
		}
	}
}

/**************************************************************************/

Bool	HMapMeshInfos3D::IsEmptyZone(S32 _x, S32 _y, S32 _z)
{
	QuadTree_ScanMesh::Cell	*pCell = m_pMapMeshDatas->GetCell(_x, _z);

	QuadTree_ScanMesh::ObjectChain *pZone = pCell->pFirst;
	Bool	bOrientOkMin = TRUE;
	S32		hMin = m_pMapMeshDatas->m_hMin;
	Bool	bOrientOkMax = TRUE;
	S32		hMax = m_pMapMeshDatas->m_hMax;

	while (pZone)
	{
		if (pZone->pFirstRefFace)
		{
			// Une zone collide.
			S32 zh = pZone->hMin;
			if (_y < zh)
			{
				zh--;
				if (zh < hMax)
				{
					// New Max.
					hMax = zh;
					// Orient OK ?
					PlaySpace_ScanMeshRefFace *pRefFace = pZone->pFirstRefFace;
					bOrientOkMax = FALSE;
					while (pRefFace)
					{
						if (pRefFace->pFace->Normal.y <= 0.f)
						{
							bOrientOkMax = TRUE;
							break;
						}
						pRefFace = pRefFace->pNext;
					}
				}
			}
			else if (_y > zh)
			{
				zh++;
				if (zh > hMin)
				{
					// New Max.
					hMin = zh;
					// Orient OK ?
					PlaySpace_ScanMeshRefFace *pRefFace = pZone->pFirstRefFace;
					bOrientOkMin = FALSE;
					while (pRefFace)
					{
						if (pRefFace->pFace->Normal.y >= 0.f)
						{
							bOrientOkMin = TRUE;
							break;
						}
						pRefFace = pRefFace->pNext;
					}
				}
			}
			else
			{
				return FALSE;
			}
		}
		else if ((_y >= pZone->hMin) && (_y <= pZone->hMax))
		{
			EXCEPTIONC_Z(FALSE, "LAAAAA");
		}
		pZone = pZone->pNext;
	}

	if (hMin == m_pMapMeshDatas->m_hMin)
	{
		if (hMax == m_pMapMeshDatas->m_hMax)
			return FALSE;
		if (!bOrientOkMax)
			return FALSE;
	}
	else if ((hMax == m_pMapMeshDatas->m_hMax) && !bOrientOkMin)
	{
		// Ceci créé le pb des colonnes vers le haut sur le bord des objets :(
		// Pourrait être fixé avec la notion de VU ou PAS VU.
		// Pour le moment, fix crado => si Plafond Y loin de la zone, alors, c'est ok.

		if ((m_pMapMeshDatas->m_hCeiling - hMin) < 2)
			return FALSE;
	}

	return TRUE;
}

/**************************************************************************/

PlaySpace_HVoxelZone	*HMapMeshInfos3D::ConstructEmptyZone(S32 _x, S32 _y, S32 _z)
{
	U8 *pStartCell = m_BlindDetector.GetCell(_x,0,_z);
	if (_y < 0)
		return NULL;

	S32 MaxY = m_pMapMeshDatas->m_hMax;
	if (_y >= MaxY)
		return NULL;

	if (pStartCell[_y] <= 1)
		return NULL;

	// Search Min
	S32		hMin = _y;
	S32		hMax = _y;

	hMin--;
	while (hMin >= 0)
	{
		if (pStartCell[hMin] <= 1)
			break;
		hMin--;
	}
	hMin++;

	// Search Max
	hMax++;
	while (hMax < MaxY)
	{
		if (pStartCell[hMax] <= 1)
			break;
		hMax++;
	}
	hMax--;

	// Create Zone !
	QuadTree_ScanMesh::ObjectChain *pDatas = m_pMapMeshDatas->New();
	pDatas->hMin = hMin;
	pDatas->hMax = hMax;
	pDatas->pFirstRefFace = NULL;
	pDatas->pNext = NULL;
	m_pMapMeshDatas->Add(_x, _z, pDatas);

	return pDatas;
}

/**************************************************************************/

void	HMapMeshInfos3D::ProcessZone(const Vec3i &_Pos, S32 _hmin, S32 _hmax, DynArray_Z<Vec3i, 256, FALSE, FALSE>	&_ProcessList)
{
	S32 h = _hmin;
	S32 lx = _Pos.x;
	S32 lz = _Pos.z;

	while (h <= _hmax)
	{
		PlaySpace_HVoxelZone *pLinkedZone = m_pMapMeshDatas->GetZone(lx, h, lz,FALSE);
		if (!pLinkedZone)
		{
			pLinkedZone = ConstructEmptyZone(lx, h, lz);
			if (pLinkedZone)
			{
				Vec3i &newPos = _ProcessList[_ProcessList.Add()];
				newPos.x = lx;
				newPos.y = h;
				newPos.z = lz;
			}
		}
		if (pLinkedZone)
			h = pLinkedZone->hMax + 1;
		else
			h++;
	}
}

/**************************************************************************/

S32		HMapMeshInfos3D::ProcessFlood(const Vec3i &_StartPos)
{
	// Do The process of flooding.
	S32 Volume = 0;
	DynArray_Z<Vec3i, 256, FALSE, FALSE>	ProcessList;	// Profondeur d'abord... donc pas énorme.
	ConstructEmptyZone(_StartPos.x, _StartPos.y, _StartPos.z);
	ProcessList.Add(_StartPos);
	for (;;)
	{
		// Pop.
		S32 LastElem = ProcessList.GetSize() - 1;
		if (LastElem < 0)
			break;	// Nothng to do.

		Vec3i CurPos = ProcessList[LastElem];
		ProcessList.SetSize(LastElem, TRUE);

		// Manage it.
		PlaySpace_HVoxelZone *pZone = m_pMapMeshDatas->GetZone(CurPos.x, CurPos.y, CurPos.z,FALSE);
		EXCEPTIONC_Z(pZone != NULL, "LAAAAA ERROR");

		// Compute Volume.
		Volume += pZone->hMax - pZone->hMin + 1;
		// X+1
		if (CurPos.x < (m_NbCellX - 1))
		{
			CurPos.x++;
			ProcessZone(CurPos, pZone->hMin, pZone->hMax, ProcessList);
			CurPos.x--;
		}

		// X-1
		if (CurPos.x  > 0)
		{
			CurPos.x--;
			ProcessZone(CurPos, pZone->hMin, pZone->hMax, ProcessList);
			CurPos.x++;
		}

		// Z+1
		if (CurPos.z < (m_NbCellZ - 1))
		{
			CurPos.z++;
			ProcessZone(CurPos, pZone->hMin, pZone->hMax, ProcessList);
			CurPos.z--;
		}

		// Z-1
		if (CurPos.z  > 0)
		{
			CurPos.z--;
			ProcessZone(CurPos, pZone->hMin, pZone->hMax, ProcessList);
			CurPos.z++;
		}
	}

	return Volume;
}

/**************************************************************************/

Bool	HMapMeshInfos3D::ProcessOnePass(Segment_Z &_ViewSeg,Bool _FastMode)
{
	// First Get The StartPoint.
	Vec3f	Start = _ViewSeg.Org;
	Vec3f	Delta = _ViewSeg.Dir * m_SizeCell;
	Float	InvSizeCell = 1.f / m_SizeCell;

	S32		NbPass = 10.f * InvSizeCell;
	Vec3i	StartPos;

	Bool	HaveFoundSomething = FALSE;

	for (S32 i = 0; i<NbPass; i++, Start += Delta)
	{
		StartPos.x = (S32)((Start.x - m_PosMin.x) * InvSizeCell);
		StartPos.y = (S32)((Start.y - m_PosMin.y) * InvSizeCell);
		StartPos.z = (S32)((Start.z - m_PosMin.z) * InvSizeCell);

		if ((StartPos.x < 0) || (StartPos.x >= m_NbCellX))
			continue;
		if ((StartPos.y < 0) || (StartPos.y >= m_pMapMeshDatas->m_hMax))
			continue;
		if ((StartPos.z < 0) || (StartPos.z >= m_NbCellZ))
			continue;

		// Stop by collision ?
		PlaySpace_HVoxelZone *pZone = m_pMapMeshDatas->GetZone(StartPos.x, StartPos.y, StartPos.z,FALSE);

		if (IsEmptyZone(StartPos.x, StartPos.y, StartPos.z))
		{
			// YES !!!
			HaveFoundSomething = TRUE;
			break;
		}
	}

	if (!HaveFoundSomething)
		return FALSE;
	// Compute BlindZone.
	m_BlindDetector.Init(m_NbCellX,m_NbCellY,m_NbCellZ);
	if (!_FastMode)
	{
		if (!m_BlindDetector.ComputeBlindZone(m_pMapMeshDatas, StartPos))
			return FALSE;
	}
	else
	{
		m_BlindDetector.ComputeBlindZoneFast(m_pMapMeshDatas, StartPos);
	}

	ProcessFlood(StartPos);

	// Free BlidDetector...
	m_BlindDetector.Flush();

	return TRUE;
}


/**************************************************************************/

Bool	HMapMeshInfos3D::ProcessFromDevice(Segment_Z &_ViewSeg,Playspace_SR_W *_pPlayspaceSR)
{
	// Init Blind Map.
	m_BlindDetector.Init(m_NbCellX,m_NbCellY,m_NbCellZ);
	
	memset(m_BlindDetector.TabBlindZone.GetArrayPtr(), 0, m_BlindDetector.TabBlindZone.GetSize());
	if (!_pPlayspaceSR->m_BlindMap.IsInit())
		return FALSE;

	// Replicate DeviceBlind to BlindMap.
	Vec3f	Pos;
	Float	HalfSizeCell = m_SizeCell * 0.5f;
	Float	InvSizeCell = 1.f / m_SizeCell;
	Vec3f	BasePos(m_PosMin.x + HalfSizeCell,m_PosMin.y + HalfSizeCell,m_PosMin.z + HalfSizeCell);
	Float	TabZone[256];

	for (S32 z=0 ; z<m_NbCellZ ; z++)
	{
		Pos = BasePos;
		Pos.z += (Float)z * m_SizeCell;
		for (S32 x=0 ; x<m_NbCellX ; x++)
		{
			// Access to BlindMap.
			S32		NbZone = _pPlayspaceSR->m_BlindMap.GetSeenZoneList(Pos.x,Pos.z,m_PosMin.y,m_PosMax.y,TabZone,256);
			if (NbZone)
			{
				Float	*PtrZone = TabZone;
				U8		*pColumn = m_BlindDetector.GetCell(x,0,z);
				for (S32 i=0 ; i<NbZone ; i+=2)
				{
					// Create Zone !
					Float fh = *PtrZone++;
					S32 h = (S32)((fh - m_PosMin.y + 0.01f) * InvSizeCell);
					if (h<0)
						h = 0;
					S32 sh = h;

					fh = *PtrZone++;
					h = (S32)((fh - m_PosMin.y - 0.01f) * InvSizeCell);
					if (h>=m_NbCellY)
						h = m_NbCellY-1;
					S32 eh = h;

					for (h=sh ; h<=eh ; h++)
						pColumn[h] = 0xFF;	// Set à Empty and Visible.
				}
			}

			// Next.
			Pos.x += m_SizeCell;
		}
	}

	// Add Face Blocs (After because seen zone transform visible block into Empty).
	PlaySpace_Vec3i pos;
	for (S32 x = 0; x<m_NbCellX; x++)
	for (S32 z = 0; z<m_NbCellZ; z++)
	{
		S32	Delta = ((z * m_NbCellX + x) * m_NbCellY);
		QuadTree_ScanMesh::Cell	*pCell = m_pMapMeshDatas->GetCell(x, z);
		QuadTree_ScanMesh::ObjectChain *pZone = pCell->pFirst;
		while (pZone)
		{
			if (pZone->pFirstRefFace)
			{
				// Add a bloc on BlindZone.
				pos.x = x;
				pos.y = pZone->hMin;
				pos.z = z;
				m_BlindDetector.TabBlindZone[Delta + pZone->hMin] = 0;
			}
			pZone = pZone->pNext;
		}
	}

	// On expland un peu la blind.
	S32 SizeBase = m_BlindDetector.m_SizeX * m_BlindDetector.m_SizeY;
	S32 SizeTotal = SizeBase * m_BlindDetector.m_SizeZ;
	U8 *pCurCellP = m_BlindDetector.TabBlindZone.GetArrayPtr();
	U8 *pCurCellN = m_BlindDetector.TabBlindZone.GetArrayPtr() + SizeTotal;
	for (S32 z = 0; z < m_BlindDetector.m_SizeZ; z++)
	{
		for (S32 x = 0; x < m_BlindDetector.m_SizeX; x++)
		{
			for (S32 y = 0; y < m_BlindDetector.m_SizeY; y++)
			{
				// Positiv propagation.
				if (*pCurCellP == 1)
				{
					if (y && (pCurCellP[-1] == 0xFF))
						*pCurCellP = 0x7F;
					else if (x && (pCurCellP[-m_BlindDetector.m_SizeY] == 0xFF))
						*pCurCellP = 0x7F;
					else if (z && (pCurCellP[-SizeBase] == 0xFF))
						*pCurCellP = 0x7F;
				}
				pCurCellP++;

				// Negativ Propagation
				pCurCellN--;
				if (*pCurCellN == 1)
				{
					if (y && (pCurCellN[1] == 0xFF))
						*pCurCellN = 0x7F;
					else if (x && (pCurCellN[m_BlindDetector.m_SizeY] == 0xFF))
						*pCurCellN = 0x7F;
					else if (z && (pCurCellN[SizeBase] == 0xFF))
						*pCurCellN = 0x7F;
				}
			}
		}
	}

	// First Get The StartPoint.
	Vec3f	Start = _ViewSeg.Org;
	Vec3f	Delta = _ViewSeg.Dir * m_SizeCell;
	S32		NbPass = 10.f * InvSizeCell;

	Vec3i	StartPos;

	Bool	HaveFoundSomething = FALSE;
	for (S32 i = 0; i<NbPass; i++, Start += Delta)
	{
		StartPos.x = (S32)((Start.x - m_PosMin.x) * InvSizeCell);
		StartPos.y = (S32)((Start.y - m_PosMin.y) * InvSizeCell);
		StartPos.z = (S32)((Start.z - m_PosMin.z) * InvSizeCell);

		if ((StartPos.x < 0) || (StartPos.x >= m_NbCellX))
			continue;
		if ((StartPos.y < 0) || (StartPos.y >= m_pMapMeshDatas->m_hMax))
			continue;
		if ((StartPos.z < 0) || (StartPos.z >= m_NbCellZ))
			continue;

		U8	*pVoxel = m_BlindDetector.GetCell(StartPos.x,StartPos.y,StartPos.z);

		if (*pVoxel <= 1)
			continue;

		HaveFoundSomething = TRUE;
		break;
	}

	if (!HaveFoundSomething)
		return FALSE;

	S32 Volume = ProcessFlood(StartPos);
	if ((Volume < 2000) && (Volume < (m_LastDeviceVolume >>2)))
		return FALSE;
	m_LastDeviceVolume = Volume;

	// Free BlidDetector...
	m_BlindDetector.Flush();

	return TRUE;
}

/**************************************************************************/

// ne pas changer ces constantes, sinon les algos 'BUBBLE' ne fonctionnent plus

#define BUBBLE_IN		0x1
#define BUBBLE_XP		0x2
#define BUBBLE_XN		0x4
#define BUBBLE_ZP		0x8
#define BUBBLE_ZN		0x10
#define BUBBLE_YP		0x20
#define BUBBLE_YN		0x40

#define BUBBLE_MASK		0x7E

/* used by ProcessVisibleFromLocation */
#define FLAG_OCCUPIED	0x1
#define FLAG_VISIBLE	0x2

#define BUBBLE_WALL			0x100
#define BUBBLE_WALL_XP		0x200
#define BUBBLE_WALL_XN		0x400
#define BUBBLE_WALL_ZP		0x800
#define BUBBLE_WALL_ZN		0x1000

#define BUBBLE_BORDER	0x1E
#define BUBBLE_MASK2D	0x1F1E

#define BUBBLE_Y		(BUBBLE_YP | BUBBLE_YN)
#define BUBBLE_X		(BUBBLE_XP | BUBBLE_XN)
#define BUBBLE_Z		(BUBBLE_ZP | BUBBLE_ZN)

#define BUBBLE_XY		(BUBBLE_YP | BUBBLE_YN | BUBBLE_XP | BUBBLE_XN)
#define BUBBLE_YZ		(BUBBLE_YP | BUBBLE_YN | BUBBLE_ZP | BUBBLE_ZN)
#define BUBBLE_XZ		(BUBBLE_XP | BUBBLE_XN | BUBBLE_ZP | BUBBLE_ZN)

#define BUBBLE_YN_XN_ZN		(BUBBLE_YN | BUBBLE_XN | BUBBLE_ZN)
#define BUBBLE_XN_ZN		(BUBBLE_XN | BUBBLE_ZN)
#define BUBBLE_YP_XN_ZN		(BUBBLE_YP | BUBBLE_XN | BUBBLE_ZN)
#define BUBBLE_YN_ZN		(BUBBLE_YN | BUBBLE_ZN)
#define BUBBLE_YP_ZN		(BUBBLE_YP | BUBBLE_ZN)
#define BUBBLE_YN_XP_ZN		(BUBBLE_YN | BUBBLE_XP | BUBBLE_ZN)
#define BUBBLE_XP_ZN		(BUBBLE_XP | BUBBLE_ZN)
#define BUBBLE_YP_XP_ZN		(BUBBLE_YP | BUBBLE_XP | BUBBLE_ZN)

#define BUBBLE_YN_XN		(BUBBLE_YN | BUBBLE_XN)
#define BUBBLE_YP_XN		(BUBBLE_YP | BUBBLE_XN)
#define BUBBLE_YN_XP		(BUBBLE_YN | BUBBLE_XP)
#define BUBBLE_YP_XP		(BUBBLE_YP | BUBBLE_XP)

#define BUBBLE_YN_XN_ZP		(BUBBLE_YN | BUBBLE_XN | BUBBLE_ZP)
#define BUBBLE_XN_ZP		(BUBBLE_XN | BUBBLE_ZP)
#define BUBBLE_YP_XN_ZP		(BUBBLE_YP | BUBBLE_XN | BUBBLE_ZP)
#define BUBBLE_YN_ZP		(BUBBLE_YN | BUBBLE_ZP)
#define BUBBLE_YP_ZP		(BUBBLE_YP | BUBBLE_ZP)
#define BUBBLE_YN_XP_ZP		(BUBBLE_YN | BUBBLE_XP | BUBBLE_ZP)
#define BUBBLE_XP_ZP		(BUBBLE_XP | BUBBLE_ZP)
#define BUBBLE_YP_XP_ZP		(BUBBLE_YP | BUBBLE_XP | BUBBLE_ZP)

#define BUBBLE_ALL		(BUBBLE_YP | BUBBLE_YN | BUBBLE_XP | BUBBLE_XN | BUBBLE_ZP | BUBBLE_ZN)


#define WALL_IS_WALL			0x1
#define WALL_IN_FRONT_XP		0x2
#define WALL_IN_FRONT_XN		0x4
#define WALL_IN_FRONT_ZP		0x8
#define WALL_IN_FRONT_ZN		0x10
#define WALL_IN_FRONT_YP		0x20
#define WALL_IN_FRONT_YN		0x40
#define WALL_BEHIND_XP			0x80
#define WALL_BEHIND_XN			0x100
#define WALL_BEHIND_ZP			0x200
#define WALL_BEHIND_ZN			0x400
#define WALL_BEHIND_YP			0x800
#define WALL_BEHIND_YN			0x1000

#define WALL_VIRTUAL_BORDER_YP	0x2000
#define WALL_VIRTUAL_BORDER_YN	0x4000
#define WALL_VIRTUAL_BORDER_XP	0x8000
#define WALL_VIRTUAL_BORDER_XN	0x10000
#define WALL_VIRTUAL_BORDER_ZP	0x20000
#define WALL_VIRTUAL_BORDER_ZN	0x40000

#define WALL_IN_ZONE			0x80000
#define WALL_NEIGHBOUR_FACES	0x100000

#define ZONE_UNUSED				0x1

#define WALL_BEHIND_MASK	0x1F80
#define WALL_IN_FRONT_MASK	0x7E

#define WALL_VIRTUAL_BORDER_ALL		(WALL_VIRTUAL_BORDER_YP | WALL_VIRTUAL_BORDER_YN | WALL_VIRTUAL_BORDER_XP | WALL_VIRTUAL_BORDER_XN | WALL_VIRTUAL_BORDER_ZP | WALL_VIRTUAL_BORDER_ZN)

#define WALL_VIRTUAL_BORDER_ALL_IZ		(WALL_VIRTUAL_BORDER_YP | WALL_VIRTUAL_BORDER_YN | WALL_VIRTUAL_BORDER_XP | WALL_VIRTUAL_BORDER_XN | WALL_VIRTUAL_BORDER_ZP | WALL_VIRTUAL_BORDER_ZN | WALL_IN_ZONE)

#define WALL_VIRTUAL_SHIFT		13

#define WALL_IN_FRONT_ALL_BUT_CEIL	0x3E

#define WALL_IN_FRONT_XPXN		(WALL_IN_FRONT_XP | WALL_IN_FRONT_XN)
#define WALL_IN_FRONT_YPYN		(WALL_IN_FRONT_YP | WALL_IN_FRONT_YN)
#define WALL_IN_FRONT_ZPZN		(WALL_IN_FRONT_ZP | WALL_IN_FRONT_ZN)

#define WALL_BEHIND_XPXN		(WALL_BEHIND_XP | WALL_BEHIND_XN)
#define WALL_BEHIND_YPYN		(WALL_BEHIND_YP | WALL_BEHIND_YN)
#define WALL_BEHIND_ZPZN		(WALL_BEHIND_ZP | WALL_BEHIND_ZN)

void HMapMeshInfos3D::ComputeWallInfos(HU32DA& wallInfos)
{
	if ((m_NbCellX <= 0) || (m_NbCellY <= 0) || (m_NbCellZ <= 0))
		return;

	// Init
	wallInfos.SetSize(m_NbCellX * m_NbCellY * m_NbCellZ);
	U32 *pWallInfos = wallInfos.GetArrayPtr();
	memset(pWallInfos, 0, wallInfos.GetSize() * sizeof(U32));

	Float epsilon = 0.5f;
	S32 zIncr = m_NbCellY * m_NbCellX;

	//////////////////////////////////////   Add Seen Face Blocs   //////////////////////////////////////////
	U32* pCurCell = pWallInfos;
	for (S32 z = 0; z < m_NbCellZ; z++)
	{
		for (S32 x = 0; x < m_NbCellX; x++)
		{
			QuadTree_ScanMesh::Cell	*pCell = m_pMapMeshDatas->GetCell(x, z);
			QuadTree_ScanMesh::ObjectChain *pZone = pCell->pFirst;
			while (pZone)
			{
				if ((pZone->hMin >= m_pMapMeshDatas->m_hGround) && (pZone->hMin <= m_pMapMeshDatas->m_hCeiling))		// Only if above Ground and under Ceil
				{
					U16 flagWall = 0;
					PlaySpace_ScanMeshRefFace *pRefFace = pZone->pFirstRefFace;
					while (pRefFace)
					{
						PlaySpace_ScanMeshFace *pFace = pRefFace->pFace;
						if (pFace->IsSeenQuality > 0)		// Seen or paint.
						{
							flagWall |= WALL_IS_WALL;
							if (pFace->Normal.x > epsilon)
								flagWall |= (WALL_IN_FRONT_XP | WALL_BEHIND_XP);		// WALL_IN_FRONT_XP propagates with (X+)  and WALL_BEHIND_XP propagates with (X-)
							else if (pFace->Normal.x < -epsilon)
								flagWall |= (WALL_IN_FRONT_XN | WALL_BEHIND_XN);		// WALL_IN_FRONT_XN propagates with (X-)  and WALL_BEHIND_XN propagates with (X+)

							if (pFace->Normal.y > epsilon)
								flagWall |= (WALL_IN_FRONT_YP | WALL_BEHIND_YP);		// WALL_IN_FRONT_YP propagates with (Y+)  and WALL_BEHIND_YP propagates with (Y-)
							else if (pFace->Normal.y < -epsilon)
								flagWall |= (WALL_IN_FRONT_YN | WALL_BEHIND_YN);		// WALL_IN_FRONT_YN propagates with (Y-)  and WALL_BEHIND_YN propagates with (Y+)

							if (pFace->Normal.z > epsilon)
								flagWall |= (WALL_IN_FRONT_ZP | WALL_BEHIND_ZP);		// WALL_IN_FRONT_ZP propagates with (Z+)  and WALL_BEHIND_ZP propagates with (Z-)
							else if (pFace->Normal.z < -epsilon)
								flagWall |= (WALL_IN_FRONT_ZN | WALL_BEHIND_ZN);		// WALL_IN_FRONT_ZN propagates with (Z-)  and WALL_BEHIND_ZN propagates with (Z+)
						}
						pRefFace = pRefFace->pNext;
					}

					if ((flagWall & WALL_IN_FRONT_XPXN) == WALL_IN_FRONT_XPXN)
						flagWall &= ~WALL_IN_FRONT_XPXN;
					if ((flagWall & WALL_BEHIND_XPXN) == WALL_BEHIND_XPXN)
						flagWall &= ~WALL_BEHIND_XPXN;

					if ((flagWall & WALL_IN_FRONT_YPYN) == WALL_IN_FRONT_YPYN)
						flagWall &= ~WALL_IN_FRONT_YPYN;
					if ((flagWall & WALL_BEHIND_YPYN) == WALL_BEHIND_YPYN)
						flagWall &= ~WALL_BEHIND_YPYN;

					if ((flagWall & WALL_IN_FRONT_ZPZN) == WALL_IN_FRONT_ZPZN)
						flagWall &= ~WALL_IN_FRONT_ZPZN;
					if ((flagWall & WALL_BEHIND_ZPZN) == WALL_BEHIND_ZPZN)
						flagWall &= ~WALL_BEHIND_ZPZN;

					pCurCell[pZone->hMin] |= flagWall;
				}
				pZone = pZone->pNext;
			}
			pCurCell += m_NbCellY;
		}
	}
	///////////////////////////////   Propagate wall infos   //////////////////////////////////////
	pCurCell = pWallInfos;
	U32* pCurCellNeg = pWallInfos + wallInfos.GetSize();
	for (S32 z = 0; z < m_NbCellZ; z++)
	{
		for (S32 x = 0; x < m_NbCellX; x++)
		{
			for (S32 y = 0; y < m_NbCellY; y++)
			{
				// Positiv propagation.
				if (!(*pCurCell & WALL_IS_WALL))
				{
					if (y  && (pCurCell[-1] & WALL_IN_FRONT_YP))
						*pCurCell |= WALL_IN_FRONT_YP;

					if (y && (pCurCell[-1] & WALL_BEHIND_YN))
						*pCurCell |= WALL_BEHIND_YN;

					if (x && (pCurCell[-m_NbCellY] & WALL_IN_FRONT_XP))
						*pCurCell |= WALL_IN_FRONT_XP;

					if (x && (pCurCell[-m_NbCellY] & WALL_BEHIND_XN))
						*pCurCell |= WALL_BEHIND_XN;

					if (z && (pCurCell[-zIncr] & WALL_IN_FRONT_ZP))
						*pCurCell |= WALL_IN_FRONT_ZP;

					if (z && (pCurCell[-zIncr] & WALL_BEHIND_ZN))
						*pCurCell |= WALL_BEHIND_ZN;
				}
				pCurCell++;
				// Negativ Propagation
				pCurCellNeg--;
				if (!(*pCurCellNeg & WALL_IS_WALL))
				{
					if (y && (pCurCellNeg[1] & WALL_IN_FRONT_YN))
						*pCurCellNeg |= WALL_IN_FRONT_YN;

					if (y && (pCurCellNeg[1] & WALL_BEHIND_YP))
						*pCurCellNeg |= WALL_BEHIND_YP;

					if (x && (pCurCellNeg[m_NbCellY] & WALL_IN_FRONT_XN))
						*pCurCellNeg |= WALL_IN_FRONT_XN;

					if (x && (pCurCellNeg[m_NbCellY] & WALL_BEHIND_XP))
						*pCurCellNeg |= WALL_BEHIND_XP;

					if (z && (pCurCellNeg[zIncr] & WALL_IN_FRONT_ZN))
						*pCurCellNeg |= WALL_IN_FRONT_ZN;

					if (z && (pCurCellNeg[zIncr] & WALL_BEHIND_ZP))
						*pCurCellNeg |= WALL_BEHIND_ZP;
				}
			}
		}
	}
}

void HMapMeshInfos3D::ProcessConcaveHull2D(HU8DA& tabInfos2D, HU16DA& flagBuffer, Bool onlySeen)
{
	if ((m_NbCellX <= 0) || (m_NbCellY <= 0) || (m_NbCellZ <= 0))
		return;

	// Init
	tabInfos2D.SetSize(m_NbCellX * m_NbCellZ);
	U8 *pTabInfos2D = tabInfos2D.GetArrayPtr();
	memset(pTabInfos2D, 0x7F, tabInfos2D.GetSize());

	//HU16DA flagBuffer;
	flagBuffer.SetSize(m_NbCellX * m_NbCellZ);
	U16* pFlagBuffer = flagBuffer.GetArrayPtr();
	memset(pFlagBuffer, 0, flagBuffer.GetSize() * sizeof(U16));

	// Add Seen Face Blocs.
	U8* pCurCell = pTabInfos2D;
	U16* pFlagBuf = pFlagBuffer;
	for (S32 z = 0; z < m_NbCellZ; z++)
	{
		for (S32 x = 0; x < m_NbCellX; x++)
		{
			QuadTree_ScanMesh::Cell	*pCell = m_pMapMeshDatas->GetCell(x, z);
			QuadTree_ScanMesh::ObjectChain *pZone = pCell->pFirst;
			Bool isColumnSeen = FALSE;
			S32 wallXP = 0, wallXN = 0, wallZP = 0, wallZN = 0;
			while (pZone)
			{
				Bool isSeen = FALSE, okWall = FALSE;
				U8 dirWall = 0;
				if ((pZone->hMin >= m_pMapMeshDatas->m_hGround) && (pZone->hMin <= m_pMapMeshDatas->m_hCeiling))		// Only if above Ground and under Ceil
				{ 
					PlaySpace_ScanMeshRefFace *pRefFace = pZone->pFirstRefFace;
					while (pRefFace)
					{
						PlaySpace_ScanMeshFace *pFace = pRefFace->pFace;
						if (!onlySeen || (pFace->IsSeenQuality > 0))		// Seen or paint.
						{
							isSeen = isColumnSeen = TRUE;
							if (Abs(pFace->Normal.y) < 0.707f)
							{
								if (Abs(pFace->Normal.x) > Abs(pFace->Normal.z))
								{
									if (pFace->Normal.x >= 0.f)
										dirWall |= BUBBLE_XP;
									else
										dirWall |= BUBBLE_XN;
								}
								else
								{
									if (pFace->Normal.z >= 0.f)
										dirWall |= BUBBLE_ZP;
									else
										dirWall |= BUBBLE_ZN;
								}
								okWall = TRUE;
							}
						}
						pRefFace = pRefFace->pNext;
					}
				}
				if (isSeen && okWall)
				{
					if (dirWall & BUBBLE_XP)
						wallXP++;
					if (dirWall & BUBBLE_XN)
						wallXN++;
					if (dirWall & BUBBLE_ZP)
						wallZP++;
					if (dirWall & BUBBLE_ZN)
						wallZN++;
				}
				pZone = pZone->pNext;
			}
			if (isColumnSeen)
				*pCurCell = 0;

			if (wallXP > 10)
			{
				if (wallXN > 10)
					*pFlagBuf |= BUBBLE_WALL;
				else
					*pFlagBuf |= (BUBBLE_WALL | BUBBLE_WALL_XP);
			}
			else if (wallXN > 10)
				*pFlagBuf |= (BUBBLE_WALL | BUBBLE_WALL_XN);

			if (wallZP > 10)
			{
				if (wallZN > 10)
					*pFlagBuf |= BUBBLE_WALL;
				else
					*pFlagBuf |= (BUBBLE_WALL | BUBBLE_WALL_ZP);
			}
			else if (wallZN > 10) 
				*pFlagBuf |= (BUBBLE_WALL | BUBBLE_WALL_ZN);

			pCurCell++;
			pFlagBuf++;
		}
	}
	////////////////////////////////////////////////////////// Propagate wall infos
	pFlagBuf = pFlagBuffer;
	U16* pFlagBufNeg = pFlagBuffer + flagBuffer.GetSize();
	for (S32 z = 0; z < m_NbCellZ; z++)
	{
		for (S32 x = 0; x < m_NbCellX; x++)
		{
			// Positiv propagation.
			if (!(*pFlagBuf & BUBBLE_WALL))
			{
				if ((x > 0) && (pFlagBuf[-1] & BUBBLE_WALL_XN))
					*pFlagBuf |= BUBBLE_WALL_XN;

				if ((z > 0) && (pFlagBuf[-m_NbCellX] & BUBBLE_WALL_ZN))
					*pFlagBuf |= BUBBLE_WALL_ZN;
			}
			pFlagBuf++;
			// Negativ Propagation
			pFlagBufNeg--;
			if (!(*pFlagBufNeg & BUBBLE_WALL))
			{
				if ((x > 0) && (pFlagBufNeg[1] & BUBBLE_WALL_XP))
					*pFlagBufNeg |= BUBBLE_WALL_XP;

				if ((z > 0) && (pFlagBufNeg[m_NbCellX] & BUBBLE_WALL_ZP))
					*pFlagBufNeg |= BUBBLE_WALL_ZP;
			}
		}
	}
	//////////////////////////////////////////////////////////
	S32 d2X = m_NbCellX + m_NbCellX;
	S32 dir[4] = { 1 , -1 , m_NbCellX , -m_NbCellX };

	S32 concIdx2D[4] = { 1 , m_NbCellX , 1 + m_NbCellX , 1 - m_NbCellX };
	U8 border;
	static U8 concTst2D[4] = { BUBBLE_X , BUBBLE_Z , BUBBLE_XZ , BUBBLE_XZ };
	static U8 tst2D[9] = { BUBBLE_XN_ZN , BUBBLE_ZN , BUBBLE_XP_ZN , BUBBLE_XN , 0 , BUBBLE_XP , BUBBLE_XN_ZP , BUBBLE_ZP , BUBBLE_XP_ZP };
	
	S32 tx = m_NbCellX - 1;
	S32 tz = m_NbCellZ - 1;
	//////////////////////////////////////////////////////////
	DynArray_Z<Vec2i, 32, FALSE, FALSE, 32, TRUE> ringBuf;
	ringBuf.SetSize(4 * (m_NbCellX + m_NbCellZ));				// majorant grossier mais sûr

	S32 rbHead = 0;
	S32 rbTail = 0;
	Vec2i *pRingBuf = ringBuf.GetArrayPtr();

	// Fill the ring buffer
	///////////////////////////////////
	pCurCell = pTabInfos2D;
	pFlagBuf = pFlagBuffer;
	for (S32 z = 0; z < m_NbCellZ; z++)
	{
		U8 mask = (z > 0 ? 0 : BUBBLE_ZN) | (z < tz ? 0 : BUBBLE_ZP);
		if (mask)
		{
			for (S32 x = 1; x < tx; x++)
			{
				pFlagBuf[x] |= mask;
				if (pCurCell[x] == 0x7F)
				{
					pRingBuf[rbHead++] = Vec2i(x, z);
					pFlagBuf[x] |= BUBBLE_IN;
				}
			}
		}
		if (pCurCell[0] == 0x7F)
		{
			pRingBuf[rbHead++] = Vec2i(0, z);
			pFlagBuf[0] |= BUBBLE_IN;
		}
		if (pCurCell[tx] == 0x7F)
		{
			pRingBuf[rbHead++] = Vec2i(tx, z);
			pFlagBuf[tx] |= BUBBLE_IN;
		}
		pFlagBuf[0] |= mask | BUBBLE_XN;
		pFlagBuf[tx] |= mask | BUBBLE_XP;
		pCurCell += m_NbCellX;
		pFlagBuf += m_NbCellX;
	}
	///////////////////////////////////
	S32 nb = rbHead;
	while (nb)
	{
		Vec2i pos = ringBuf[rbTail];
		S32 idx = (m_NbCellX * pos.y) + pos.x;
		pCurCell = pTabInfos2D + idx;
		pFlagBuf = pFlagBuffer + idx;
		U16 flag = (*pFlagBuf &= BUBBLE_MASK2D);
		rbTail++;
		if (rbTail == ringBuf.GetSize())
			rbTail = 0;

		nb--;
		border = flag & BUBBLE_BORDER;
		U8 test = BUBBLE_IN;
		for (S32 i = 0; i < 4; i++)
		{
			test <<= 1;
			if (((border & test) == 0) && (pCurCell[dir[i]] == 1))
				border |= test;
		}
		if (border)
		{
			Bool concave = FALSE;
			for (S32 i = 0; i < 4; i++)
			{
				S32 d = concIdx2D[i];
				if (((flag & concTst2D[i]) == 0) && ((pCurCell[d] != 1) && (pCurCell[-d] != 1)))
				{
					concave = TRUE;
					break;
				}
			}
			if ((!concave) || ((border & BUBBLE_XP) && (flag & BUBBLE_WALL_XN)) ||
				((border & BUBBLE_XN) && (flag & BUBBLE_WALL_XP)) ||
				((border & BUBBLE_ZP) && (flag & BUBBLE_WALL_ZN)) ||
				((border & BUBBLE_ZN) && (flag & BUBBLE_WALL_ZP)))
			{
				*pCurCell = 1;
				S32 dIdx0 = -1 - m_NbCellX;
				U8* pTst = tst2D;
				for (S32 dz = -1; dz < 2; dz++)
				{
					S32 dIdx1 = dIdx0;
					for (S32 dx = -1; dx < 2; dx++)
					{
						if ((dIdx1 != 0) && ((flag & *pTst) == 0) && ((pFlagBuf[dIdx1] & BUBBLE_IN) == 0) && (pCurCell[dIdx1] == 0x7F))
						{
							nb++;
							pFlagBuf[dIdx1] |= BUBBLE_IN;
							pRingBuf[rbHead++] = pos + Vec2i(dx, dz);
							if (rbHead == ringBuf.GetSize())
								rbHead = 0;
						}
						dIdx1++;
						pTst++;
					}
					dIdx0 += m_NbCellX;
				}
				EXCEPTIONC_Z((nb < ringBuf.GetSize()), "Apprends à majorer !!!");
			}
		}
	}
}

Bool	HMapMeshInfos3D::ProcessBubbleAlgorithm(Segment_Z &_ViewSeg, Playspace_SR_W *_pPlayspaceSR, Bool onlySeen /*= TRUE*/)
{
	if ((m_NbCellX <= 0) || (m_NbCellY <= 0) || (m_NbCellZ <= 0))
		return FALSE;

	// Init Blind Map.
	m_BlindDetector.Init(m_NbCellX, m_NbCellY, m_NbCellZ);
	U8 *pBlind = m_BlindDetector.TabBlindZone.GetArrayPtr();

	memset(pBlind, 0x7F, m_BlindDetector.TabBlindZone.GetSize());
	Float	InvSizeCell = 1.f / m_SizeCell;

	HU16DA flags2D;
	ProcessConcaveHull2D(m_infos2D, flags2D, onlySeen);

	S32 xMin = 1000, xMax = -1, zMin = 1000, zMax = -1;

	// Add Seen Face Blocs.
	for (S32 x = 0; x < m_NbCellX; x++)
	{
		for (S32 z = 0; z < m_NbCellZ; z++)
		{
			S32	Delta = ((z * m_NbCellX + x) * m_NbCellY);
			QuadTree_ScanMesh::Cell	*pCell = m_pMapMeshDatas->GetCell(x, z);
			QuadTree_ScanMesh::ObjectChain *pZone = pCell->pFirst;
			while (pZone)
			{
				if ((pZone->hMin >= m_pMapMeshDatas->m_hGround)
					&& (pZone->hMin <= m_pMapMeshDatas->m_hCeiling)
					)
				{
					// Only if upper Ground 
					Bool	IsSeen = !onlySeen; // FALSE;
					if (onlySeen)
					{
						PlaySpace_ScanMeshRefFace	*pRefFace = pZone->pFirstRefFace;
						while (pRefFace)
						{
							if (pRefFace->pFace->IsSeenQuality > 0)		// Seen or paint.
							{
								IsSeen = TRUE;
								break;
							}
							pRefFace = pRefFace->pNext;
						}
					}

					if (IsSeen && (m_infos2D[(z * m_NbCellX) + x] != 1))
					{
						// Add a bloc on BlindZone.
						pBlind[Delta + pZone->hMin] = 0;
						xMin = Min<S32>(xMin, x);
						xMax = Max<S32>(xMax, x);
						zMin = Min<S32>(zMin, z);
						zMax = Max<S32>(zMax, z);
					}
				}
				pZone = pZone->pNext;
			}
		}
	}
	//////////////////////////////////////////////////////////
	S32 zIncr = m_NbCellX * m_NbCellY;
	S32 d2Y = m_NbCellY + m_NbCellY;
	S32 d2Z = zIncr + zIncr;
	S32 sizeTotal = zIncr * m_NbCellZ;
	S32 dir[6] = { m_NbCellY , -m_NbCellY, zIncr , -zIncr , 1 , -1 };

	S32 concIdx[13] = { 1 , m_NbCellY , zIncr , 1 + m_NbCellY , 1 - m_NbCellY , 1 + zIncr , 1 - zIncr , zIncr + m_NbCellY , zIncr - m_NbCellY , 1 + m_NbCellY + zIncr , 1 + m_NbCellY - zIncr , 1 - m_NbCellY + zIncr , 1 - m_NbCellY - zIncr };
	U8 border;
	static U8 concTst[13] = { BUBBLE_Y , BUBBLE_X , BUBBLE_Z , BUBBLE_XY , BUBBLE_XY , BUBBLE_YZ , BUBBLE_YZ , BUBBLE_XZ , BUBBLE_XZ , BUBBLE_ALL , BUBBLE_ALL , BUBBLE_ALL , BUBBLE_ALL };
	static U8 tst[27] = { BUBBLE_YN_XN_ZN , BUBBLE_XN_ZN , BUBBLE_YP_XN_ZN , BUBBLE_YN_ZN , BUBBLE_ZN , BUBBLE_YP_ZN , BUBBLE_YN_XP_ZN , BUBBLE_XP_ZN , BUBBLE_YP_XP_ZN ,
							BUBBLE_YN_XN , BUBBLE_XN , BUBBLE_YP_XN , BUBBLE_YN , 0 , BUBBLE_YP , BUBBLE_YN_XP , BUBBLE_XP , BUBBLE_YP_XP ,
							BUBBLE_YN_XN_ZP , BUBBLE_XN_ZP , BUBBLE_YP_XN_ZP , BUBBLE_YN_ZP , BUBBLE_ZP , BUBBLE_YP_ZP , BUBBLE_YN_XP_ZP , BUBBLE_XP_ZP , BUBBLE_YP_XP_ZP };

	static U8 planeYTst[8] = { BUBBLE_XN_ZN, BUBBLE_XN, BUBBLE_XN_ZP, BUBBLE_ZN, BUBBLE_ZP, BUBBLE_XP_ZN, BUBBLE_XP, BUBBLE_XP_ZP };
	static U8 planeXTst[8] = { BUBBLE_YN_ZN, BUBBLE_YN, BUBBLE_YN_ZP, BUBBLE_ZN, BUBBLE_ZP, BUBBLE_YP_ZN, BUBBLE_YP, BUBBLE_YP_ZP };
	static U8 planeZTst[8] = { BUBBLE_YN_XN, BUBBLE_XN, BUBBLE_YP_XN, BUBBLE_YN, BUBBLE_YP, BUBBLE_YN_XP, BUBBLE_XP, BUBBLE_YP_XP };

	S32 planeYIdx[8] = { -m_NbCellY - zIncr, -m_NbCellY, -m_NbCellY + zIncr, -zIncr, zIncr, m_NbCellY - zIncr, m_NbCellY, m_NbCellY + zIncr };
	S32 planeXIdx[8] = { -1 - zIncr, -1, -1 + zIncr, -zIncr, zIncr, 1 - zIncr, 1, 1 + zIncr };
	S32 planeZIdx[8] = { -1 - m_NbCellY, -m_NbCellY, 1 - m_NbCellY, -1, 1, -1 + m_NbCellY, m_NbCellY, 1 + m_NbCellY };

	S32 ty = m_NbCellY - 1;
	S32 tx = m_NbCellX - 1;
	S32 tz = m_NbCellZ - 1;
	Float maxCosErr = Cos(DegToRad(20.0f));

	// First step
	for (S32 x = 0; x < m_NbCellX; x++)
	{
		for (S32 z = 0; z < m_NbCellZ; z++)
		{
			S32	Delta = (z * m_NbCellX + x) * m_NbCellY;
			QuadTree_ScanMesh::Cell	*pCell = m_pMapMeshDatas->GetCell(x, z);
			QuadTree_ScanMesh::ObjectChain *pZone = pCell->pFirst;

			Bool haveGround = FALSE;
			Bool haveCeil = FALSE;
			Bool haveCeilUp = FALSE;
			Bool haveCeilUpUp = FALSE;
			Bool haveMin = FALSE;
			while (pZone)
			{
				if (pZone->pFirstRefFace)
				{
					PlaySpace_ScanMeshRefFace	*pRefFace = pZone->pFirstRefFace;
					Bool isSeen = FALSE;
					while (pRefFace)
					{
						PlaySpace_ScanMeshFace	*pFace = pRefFace->pFace;
						if (!onlySeen || (pFace->IsSeenQuality > 0)) // Seen or paint.
						{
							isSeen = TRUE;
							if (pFace->Normal.y > maxCosErr)
								haveMin = TRUE;
						}
						pRefFace = pRefFace->pNext;
					}
					if (isSeen)
					{
						if (pZone->hMin == m_pMapMeshDatas->m_hGround)
							haveGround = TRUE;

						if (pZone->hMin == m_pMapMeshDatas->m_hCeiling)
							haveCeil = TRUE;

						if (pZone->hMin == (m_pMapMeshDatas->m_hCeiling + 1))
							haveCeilUp = TRUE;

						if (pZone->hMin == (m_pMapMeshDatas->m_hCeiling + 2))
							haveCeilUpUp = TRUE;
					}
				}
				pZone = pZone->pNext;
			}
			if ((haveGround || haveMin) && (m_infos2D[(z * m_NbCellX) + x] != 1))
			{
				xMin = Min<S32>(xMin, x);
				xMax = Max<S32>(xMax, x);
				zMin = Min<S32>(zMin, z);
				zMax = Max<S32>(zMax, z);
				S32 ceiling = m_pMapMeshDatas->m_hCeiling;
				if (haveCeil)
					ceiling = m_pMapMeshDatas->m_hCeiling;
				else if (haveCeilUp)
					ceiling = m_pMapMeshDatas->m_hCeiling + 1;
				else if (haveCeilUpUp)
					ceiling = m_pMapMeshDatas->m_hCeiling + 2;

				for (S32 i = ceiling; i < m_NbCellY; i++)
					pBlind[Delta + i] = 0;
			}

		}
	}

	HU8DA flagBuffer;
	flagBuffer.SetSize(sizeTotal);
	U8* pFlagBuffer = flagBuffer.GetArrayPtr();
	memset(pFlagBuffer, 0, flagBuffer.GetSize());

	Vec3iDA ringBuf;
	ringBuf.SetSize(6 * ((m_NbCellX * m_NbCellY) + (m_NbCellY * m_NbCellZ) + (m_NbCellX * m_NbCellZ)));		// majorant grossier mais sûr

	S32 rbHead = 0;
	S32 rbTail = 0;
	Vec3i *pRingBuf = ringBuf.GetArrayPtr();

	// Fill the ring buffer
	///////////////////////////////////
	U8* pCell = pBlind;
	U8* pFlagBuf = pFlagBuffer;
	for (S32 z = 0; z < m_NbCellZ; z++)
	{
		U8 zmask = (z > 0 ? 0 : BUBBLE_ZN) | (z < tz ? 0 : BUBBLE_ZP);
		for (S32 x = 0; x < m_NbCellX; x++)
		{
			U8 mask = zmask | (x > 0 ? 0 : BUBBLE_XN) | (x < tx ? 0 : BUBBLE_XP);
			if (mask)
			{
				for (S32 y = 1; y < ty; y++)
				{
					pFlagBuf[y] |= mask;
					if (pCell[y] == 0x7F)
					{
						pRingBuf[rbHead++] = Vec3i(x, y, z);
						pFlagBuf[y] |= BUBBLE_IN;
					}
				}
			}
			if (pCell[0] == 0x7F)
			{
				pRingBuf[rbHead++] = Vec3i(x, 0, z);
				pFlagBuf[0] |= BUBBLE_IN;
			}
			if (pCell[ty] == 0x7F)
			{
				pRingBuf[rbHead++] = Vec3i(x, ty, z);
				pFlagBuf[ty] |= BUBBLE_IN;
			}
			pFlagBuf[0] |= mask | BUBBLE_YN;
			pFlagBuf[ty] |= mask | BUBBLE_YP;
			pCell += m_NbCellY;
			pFlagBuf += m_NbCellY;
		}
	}
	///////////////////////////////////
	HU32DA	wallFlags;
	ComputeWallInfos(wallFlags);
	U32* pWallInfos = wallFlags.GetArrayPtr();
	U32* pWall = pWallInfos;
	///////////////////////////////////
	S32 nbElem = rbHead;
	S32 nbIter = 0;
	while (nbElem)
	{
		nbIter++;
		Vec3i pos = ringBuf[rbTail];
		S32 idx = (zIncr * pos.z) + (m_NbCellY * pos.x) + pos.y;
		pCell = pBlind + idx;
		pFlagBuf = pFlagBuffer + idx;
		U8 flag = (*pFlagBuf &= BUBBLE_MASK);
		rbTail++;
		if (rbTail == ringBuf.GetSize())
			rbTail = 0;

		nbElem--;
		border = flag;
		U8 test = BUBBLE_IN;
		for (S32 i = 0; i < 6; i++)
		{
			test <<= 1;
			if (((border & test) == 0) && (pCell[dir[i]] == 1))
				border |= test;
		}
		if (border)
		{
			U32 wall = pWallInfos[idx];
			Bool eat = FALSE;
			if (!(wall & WALL_IN_FRONT_ALL_BUT_CEIL))
			{
				if ((border & BUBBLE_YP) && (wall & WALL_BEHIND_YN))
				{
					if (flag & BUBBLE_YP)
						eat = TRUE;
					else
					{
						S32 nbEat = 0;
						for (S32 i = 0; i < 8; i++)
						{
							if (((flag & planeYTst[i]) == 0) && (pCell[1 + planeYIdx[i]] == 1))
								nbEat++;
						}
						eat = (nbEat > 3);
					}
				}
				if (!eat && (border & BUBBLE_YN) && (wall & WALL_BEHIND_YP))
				{
					if (flag & BUBBLE_YN)
						eat = TRUE;
					else
					{
						S32 nbEat = 0;
						for (S32 i = 0; i < 8; i++)
						{
							if (((flag & planeYTst[i]) == 0) && (pCell[-1 + planeYIdx[i]] == 1))
								nbEat++;
						}
						eat = (nbEat > 3);
					}
				}
				if (!eat && (border & BUBBLE_XP) && (wall & WALL_BEHIND_XN))
				{
					if (flag & BUBBLE_XP)
						eat = TRUE;
					else
					{
						S32 nbEat = 0;
						for (S32 i = 0; i < 8; i++)
						{
							if (((flag & planeXTst[i]) == 0) && (pCell[m_NbCellY + planeXIdx[i]] == 1))
								nbEat++;
						}
						eat = (nbEat > 3);
					}
				}
				if (!eat && (border & BUBBLE_XN) && (wall & WALL_BEHIND_XP))
				{
					if (flag & BUBBLE_XN)
						eat = TRUE;
					else
					{
						S32 nbEat = 0;
						for (S32 i = 0; i < 8; i++)
						{
							if (((flag & planeXTst[i]) == 0) && (pCell[-m_NbCellY + planeXIdx[i]] == 1))
								nbEat++;
						}
						eat = (nbEat > 3);
					}
				}
				if (!eat && (border & BUBBLE_ZP) && (wall & WALL_BEHIND_ZN))
				{
					if (flag & BUBBLE_ZP)
						eat = TRUE;
					else
					{
						S32 nbEat = 0;
						for (S32 i = 0; i < 8; i++)
						{
							if (((flag & planeZTst[i]) == 0) && (pCell[zIncr + planeZIdx[i]] == 1))
								nbEat++;
						}
						eat = (nbEat > 3);
					}
				}
				if (!eat && (border & BUBBLE_ZN) && (wall & WALL_BEHIND_ZP))
				{
					if (flag & BUBBLE_ZN)
						eat = TRUE;
					else
					{
						S32 nbEat = 0;
						for (S32 i = 0; i < 8; i++)
						{
							if (((flag & planeZTst[i]) == 0) && (pCell[-zIncr + planeZIdx[i]] == 1))
								nbEat++;
						}
						eat = (nbEat > 3);
					}
				}
			}

			if (!eat)
			{
				if ((border & BUBBLE_YN) && (wall & WALL_BEHIND_YP) && (wall & (WALL_BEHIND_XP | WALL_BEHIND_XN | WALL_BEHIND_ZP | WALL_BEHIND_ZN)))
					eat = TRUE;
			}

			if (!eat)
			{
				Bool concave = FALSE;
				for (S32 i = 0; i < 13; i++)
				{
					S32 d = concIdx[i];
					if (((flag & concTst[i]) == 0) && ((pCell[d] != 1) && (pCell[-d] != 1)))
					{
						concave = TRUE;
						break;
					}
				}
				eat = !concave;
			}
			if (!eat)
			{
				eat = ((border & BUBBLE_YP) && ((flag & BUBBLE_YN) == 0) && (pCell[-1] == 0)) ||
					((border & BUBBLE_YN) && ((flag & BUBBLE_YP) == 0) && (pCell[1] == 0)) ||
					((border & BUBBLE_XP) && ((flag & BUBBLE_XN) == 0) && (pCell[-m_NbCellY] == 0)) ||
					((border & BUBBLE_XN) && ((flag & BUBBLE_XP) == 0) && (pCell[m_NbCellY] == 0)) ||
					((border & BUBBLE_ZP) && ((flag & BUBBLE_ZN) == 0) && (pCell[-zIncr] == 0)) ||
					((border & BUBBLE_ZN) && ((flag & BUBBLE_ZP) == 0) && (pCell[zIncr] == 0));
			}
			if (eat)
			{
				*pCell = 1;
				S32 dIdx0 = -1 - m_NbCellY - zIncr;
				U8* pTst = tst;
				for (S32 dz = -1; dz < 2; dz++)
				{
					S32 dIdx1 = dIdx0;
					for (S32 dx = -1; dx < 2; dx++)
					{
						S32 dIdx2 = dIdx1;
						for (S32 dy = -1; dy < 2; dy++)
						{
							if ((dIdx2 != 0) && ((flag & *pTst) == 0) && ((pFlagBuf[dIdx2] & BUBBLE_IN) == 0) && (pCell[dIdx2] == 0x7F))
							{
								nbElem++;
								pFlagBuf[dIdx2] |= BUBBLE_IN;
								pRingBuf[rbHead++] = pos + Vec3i(dx, dy, dz);
								if (rbHead == ringBuf.GetSize())
									rbHead = 0;
							}
							dIdx2++;
							pTst++;
						}
						dIdx1 += m_NbCellY;
					}
					dIdx0 += zIncr;
				}
				EXCEPTIONC_Z((nbElem < ringBuf.GetSize()), "Apprends à majorer !!!");
			}
		}
	}
	/////////////////////////////////////////  Post process : fix mushrooms  ///////////////////////////////////////////
	
	// Add Upper Ceiling and Under Ground.
	for (S32 x = 0; x < m_NbCellX; x++)
	{
		for (S32 z = 0; z < m_NbCellZ; z++)
		{
			S32	Delta = ((z * m_NbCellX + x) * m_NbCellY);
			QuadTree_ScanMesh::Cell	*pCell = m_pMapMeshDatas->GetCell(x, z);
			QuadTree_ScanMesh::ObjectChain *pZone = pCell->pFirst;
			while (pZone)
			{
				if ((pZone->hMin < m_pMapMeshDatas->m_hGround) || (pZone->hMin > m_pMapMeshDatas->m_hCeiling))
						pBlind[Delta + pZone->hMin] = 0;

				pZone = pZone->pNext;
			}
		}
	}

	pCell = pBlind;
	pFlagBuf = pFlagBuffer;
	for (S32 z = 0; z < m_NbCellZ; z++)
	{
		for (S32 x = 0; x < m_NbCellX; x++)
		{
			for (S32 y = 0; y < m_NbCellY; y++)
			{
				if (*pCell == 0x7F)
				{
					U8 flag = (*pFlagBuf &= BUBBLE_MASK);
					border = flag;
					U8 test = BUBBLE_IN;
					for (S32 i = 0; i < 6; i++)
					{
						test <<= 1;
						if (((border & test) == 0) && (pCell[dir[i]] == 1))
							border |= test;
					}
					if (border)
					{
						Bool eat = FALSE;
						if ((border & BUBBLE_XP) || (border & BUBBLE_XN))
						{
							S32 nbN = 0;
							if (((flag & BUBBLE_YP) == 0) && (pCell[1] == 0))
								nbN++;
							if (((flag & BUBBLE_YN) == 0) && (pCell[-1] == 0))
								nbN++;
							if (((flag & BUBBLE_ZP) == 0) && (pCell[zIncr] == 0))
								nbN++;
							if (((flag & BUBBLE_ZN) == 0) && (pCell[-zIncr] == 0))
								nbN++;

							eat = (nbN > 1);
						}
						if (!eat && ((border & BUBBLE_YP) || (border & BUBBLE_YN)))
						{
							S32 nbN = 0;
							if (((flag & BUBBLE_XP) == 0) && (pCell[m_NbCellY] == 0))
								nbN++;
							if (((flag & BUBBLE_XN) == 0) && (pCell[-m_NbCellY] == 0))
								nbN++;
							if (((flag & BUBBLE_ZP) == 0) && (pCell[zIncr] == 0))
								nbN++;
							if (((flag & BUBBLE_ZN) == 0) && (pCell[-zIncr] == 0))
								nbN++;

							eat = (nbN > 0);
						}
						if (!eat && ((border & BUBBLE_ZP) || (border & BUBBLE_ZN)))
						{
							S32 nbN = 0;
							if (((flag & BUBBLE_XP) == 0) && (pCell[m_NbCellY] == 0))
								nbN++;
							if (((flag & BUBBLE_XN) == 0) && (pCell[-m_NbCellY] == 0))
								nbN++;
							if (((flag & BUBBLE_YP) == 0) && (pCell[1] == 0))
								nbN++;
							if (((flag & BUBBLE_YN) == 0) && (pCell[-1] == 0))
								nbN++;

							eat = (nbN > 1);
						}
						if (eat)
						{
							*pCell = 0x80;
						}
					}
				}
				pCell++;
				pFlagBuf++;
			}
		}
	}
	pCell = pBlind;
	for (S32 z = 0; z < m_NbCellZ; z++)
	{
		for (S32 x = 0; x < m_NbCellX; x++)
		{
			for (S32 y = 0; y < m_NbCellY; y++)
			{
				if (*pCell == 0x80)
					*pCell = 1;
				
				pCell++;
			}
		}
	}
	/////////////////////////////////////////  end of mushrooms Post process  ///////////////////////////////////////////

	// Re-Add  ALL Face Blocs.
	for (S32 x = 0; x < m_NbCellX; x++)
	{
		for (S32 z = 0; z < m_NbCellZ; z++)
		{
			S32	Delta = ((z * m_NbCellX + x) * m_NbCellY);
			QuadTree_ScanMesh::Cell	*pCell = m_pMapMeshDatas->GetCell(x, z);
			QuadTree_ScanMesh::ObjectChain *pZone = pCell->pFirst;
			while (pZone)
			{
				Bool	IsSeen = FALSE;
				PlaySpace_ScanMeshRefFace	*pRefFace = pZone->pFirstRefFace;
				if (pRefFace)
				{
					// Add a bloc on BlindZone.
					pBlind[Delta + pZone->hMin] = 0;
				}
				pZone = pZone->pNext;
			}
		}
	}

	// First Get The StartPoint.
	Vec3f	Start = _ViewSeg.Org;
	Vec3f	Delta = _ViewSeg.Dir * m_SizeCell;
	S32		NbPass = 10.f * InvSizeCell;

	Vec3i	StartPos;

	Bool	HaveFoundSomething = FALSE;
	for (S32 i = 0; i<NbPass; i++, Start += Delta)
	{
		StartPos.x = (S32)((Start.x - m_PosMin.x) * InvSizeCell);
		StartPos.y = (S32)((Start.y - m_PosMin.y) * InvSizeCell);
		StartPos.z = (S32)((Start.z - m_PosMin.z) * InvSizeCell);

		if ((StartPos.x < 0) || (StartPos.x >= m_NbCellX))
			continue;
		if ((StartPos.y < 0) || (StartPos.y >= m_pMapMeshDatas->m_hMax))
			continue;
		if ((StartPos.z < 0) || (StartPos.z >= m_NbCellZ))
			continue;

		U8	*pVoxel = m_BlindDetector.GetCell(StartPos.x, StartPos.y, StartPos.z);

		if (*pVoxel <= 1)
			continue;

		HaveFoundSomething = TRUE;
		break;
	}

	if (!HaveFoundSomething)
		return FALSE;

	ProcessFlood(StartPos);

	// Free BlidDetector...
	m_BlindDetector.Flush();

	return TRUE;
}

/**************************************************************************/


FINLINE_Z	void	HMapMeshInfos3D::ScanFaceClosest(Vec3f &_Pos, Vec3f &_Dir, PlaySpace_ScanReport &_Report, Float _DistMax)
{
	// Coin le plus proche.
	Float	InvSizeCell = 1.f / m_SizeCell;
	S32 x = (S32)((_Pos.x - m_PosMin.x) * InvSizeCell + 0.5f);
	S32 y = (S32)((_Pos.y - m_PosMin.y) * InvSizeCell + 0.5f);
	S32 z = (S32)((_Pos.z - m_PosMin.z) * InvSizeCell + 0.5f);

	// Compute Zone Min-Max
	S32 StartX, EndX, DeltaX;
	S32 StartY, EndY, DeltaY;
	S32 StartZ, EndZ, DeltaZ;

	// X Min Max
	if (_Dir.x < -0.001f)
	{
		// Neg.
		StartX = Min(m_NbCellX - 1,x);
		EndX = Max((S32)0,x - 2)-1;
		DeltaX = -1;
	}
	else if (_Dir.x > 0.001f)
	{
		// Pos.
		StartX = Max((S32)0,x - 1);
		EndX = Min(m_NbCellX - 1,x + 1)+1;
		DeltaX = 1;
	}
	else
	{
		// Null.
		StartX = Max((S32)0,x - 1);
		EndX = Min(m_NbCellX - 1,x)+1;
		DeltaX = 1;
	}	

	// Y Min Max
	if (_Dir.y < -0.001f)
	{
		// Neg.
		StartY = Min(m_NbCellY - 1,y);
		EndY = Max((S32)0,y - 2)-1;
		DeltaY = -1;
	}
	else if (_Dir.y > 0.001f)
	{
		// Pos.
		StartY = Max((S32)0,y - 1);
		EndY = Min(m_NbCellY - 1,y + 1)+1;
		DeltaY = 1;
	}
	else
	{
		// Null.
		StartY = Max((S32)0,y - 1);
		EndY = Min(m_NbCellY - 1,y)+1;
		DeltaY = 1;
	}

	// Z Min Max
	if (_Dir.z < -0.001f)
	{
		// Neg.
		StartZ = Min(m_NbCellZ - 1,z);
		EndZ = Max((S32)0,z - 2)-1;
		DeltaZ = -1;
	}
	else if (_Dir.z > 0.001f)
	{
		// Pos.
		StartZ = Max((S32)0,z - 1);
		EndZ = Min(m_NbCellZ - 1,z + 1)+1;
		DeltaZ = 1;
	}
	else
	{
		// Null.
		StartZ = Max((S32)0,z - 1);
		EndZ = Min(m_NbCellZ - 1,z)+1;
		DeltaZ = 1;
	}

	m_pMapMeshDatas->NewFaceTag();
	_Report.Closest.Score = _DistMax + 0.001f;

	Vec3f Delta;
	Vec3f CubeMin;
	Vec3f CubeMax;
	for (S32 vz = StartZ; vz != EndZ; vz+=DeltaZ)
	{
		CubeMin.z = vz * m_SizeCell + m_PosMin.z;
		CubeMax.z = CubeMin.z + m_SizeCell;

		if (_Pos.z < CubeMin.z)
			Delta.z = CubeMin.z - _Pos.z;
		else if (_Pos.z > CubeMax.z)
			Delta.z = _Pos.z - CubeMax.z;
		else
			Delta.z = 0.f;
		Delta.z *= Delta.z;

		for (S32 vx = StartX; vx != EndX; vx+=DeltaX)
		{
			CubeMin.x = vx * m_SizeCell + m_PosMin.x;
			CubeMax.x = CubeMin.x + m_SizeCell;

			if (_Pos.x < CubeMin.x)
				Delta.x = CubeMin.x - _Pos.x;
			else if (_Pos.x > CubeMax.x)
				Delta.x = _Pos.x - CubeMax.x;
			else
				Delta.x = 0.f;
			Delta.x *= Delta.x;
			Delta.x += Delta.z;

			QuadTree_ScanMesh::Cell	*pCell = m_pMapMeshDatas->GetCell(vx, vz);
			for (S32 vy = StartY; vy != EndY; vy+=DeltaY)
			{
				// Filter Bloc.
				CubeMin.y = vy * m_SizeCell + m_PosMin.y;
				CubeMax.y = CubeMin.y + m_SizeCell;

				if (_Pos.y < CubeMin.y)
					Delta.y = CubeMin.y - _Pos.y;
				else if (_Pos.y > CubeMax.y)
					Delta.y = _Pos.y - CubeMax.y;
				else
					Delta.y = 0.f;

				Float Score = _Report.Closest.Score;

				if (( Delta.y*Delta.y + Delta.x) > Score*Score)
					continue;

				ScanZoneFaceClosest(_Pos, _Dir, pCell, vy, _Report);
			}
		}
	}

	// Finalize ScanZone Face
	if (_Report.Closest.pFace)
	{
		_Report.Multi_SumNormal.CNormalize();
		_Report.Multi_Error = _Report.Closest.Normal * _Report.Multi_SumNormal;

		// Compute Cos( ArcCos(Multi_Error) * 2)
		//   Cos(2 x Angle) = 2 x Cos(Angle) ^ 2 - 1
		_Report.Multi_Error = 2.f * _Report.Multi_Error * _Report.Multi_Error - 1;
	}
}

/**************************************************************************/

FINLINE_Z	Bool	HMapMeshInfos3D::ScanZoneFaceClosest(Vec3f &_Pos, Vec3f &_Dir,QuadTree_ScanMesh::Cell *_pCell,S32 _y, PlaySpace_ScanReport &_Report)
{
	U16	CurFaceTag = m_pMapMeshDatas->m_CurFaceTag;
	QuadTree_ScanMesh::ObjectChain *pZone = _pCell->pFirst;
	while (pZone)
	{
		if ((_y >= pZone->hMin) && (_y <= pZone->hMax))
		{
			// Found the Zone !
			PlaySpace_ScanMeshRefFace *pRefFace = pZone->pFirstRefFace;
			for (; pRefFace != 0; pRefFace = pRefFace->pNext)
			{
				// Don't manage twice same face.
				PlaySpace_ScanMeshFace *pFace = pRefFace->pFace;

				if (pFace->Tag == CurFaceTag)
					continue;
				pFace->Tag = CurFaceTag;

				// Proj Point on Face.
				Vec3f	&FaceNormal = pFace->Normal;
				Vec3f	&p0 = pFace->p0;
				Vec3f	vDelta = _Pos - p0;
				Float	Dp = vDelta * FaceNormal;
				
				if (Abs(Dp) > (_Report.Closest.Score + 0.002f))		// Fast Reject.
					continue;

				// Get Vertex normal.
				Vec3f	&p1 = pFace->p1;
				Vec3f	&p2 = pFace->p2;

				// Compute Normal.
				Vec3f P01 = p1 - p0;
				Vec3f P12 = p2 - p1;
				Vec3f P20 = p0 - p2;
				Vec3f N0 = FaceNormal ^ P01;
				Vec3f N1 = FaceNormal ^ P12;
				Vec3f N2 = FaceNormal ^ P20;
				
				// Compute closest point to triangle.
				Vec3f	ClosestPt = _Pos - Dp * FaceNormal;

				for (;;)	// Pour éviter les variable non init du goto.
				{
					// Manage Corner.

					S32 Mask = 0;
					{
						Vec3f	DeltaP0 = ClosestPt - p0;
						Float	PosEdge0 = P01 * DeltaP0;

						if (PosEdge0 <= 0.f)
							Mask |= 0x01;
						else if (PosEdge0 >= P01.GetNorm2())
							Mask |= 0x02;
						else 
						{
							Float d0 = N0*DeltaP0;
							if (d0 > 1e-4f)
							{
								// Closest = Project on Edge.
								ClosestPt = ClosestPt - N0 * (d0 / N0.GetNorm2());
								break;
							}
						}
					}

					{
						Vec3f	DeltaP1 = ClosestPt - p1;
						Float	PosEdge1 = P12 * DeltaP1;

						if (PosEdge1 <= 0.f)
							Mask |= 0x10;
						else if (PosEdge1 >= P12.GetNorm2())
							Mask |= 0x20;
						else
						{
							Float d1 = N1*DeltaP1;
							if (d1 > 1e-4f)
							{
								// Closest = Project on Edge.
								ClosestPt = ClosestPt - N1 * (d1 / N1.GetNorm2());
								break;
							}
						}
					}

					{					
						Vec3f	DeltaP2 = ClosestPt - p2;
						Float	PosEdge2 = P20 * DeltaP2;
						if (PosEdge2 <= 0.f)
							Mask |= 0x100;
						else if (PosEdge2 >= P20.GetNorm2())
							Mask |= 0x200;
						else
						{
							Float d2 = N2*DeltaP2;
							if (d2 > 1e-4f)
							{
								// Closest = Project on Edge.
								ClosestPt = ClosestPt - N2 * (d2 / N2.GetNorm2());
								break;
							}
						}
					}

					if ((Mask & 0x201) == 0x201)
					{
						ClosestPt = p0;
						break;
					}
					if ((Mask & 0x12) == 0x12)
					{
						ClosestPt = p1;
						break;
					}
					if ((Mask & 0x120) == 0x120)
					{
						ClosestPt = p2;
						break;
					}

					// Inside triangle.
					break;
				}

				Vec3f	DeltaClosest = ClosestPt - _Pos;
				Float	DistSurf = DeltaClosest * _Dir;
									
				if (DistSurf < 0.f)	// Si la face est vraiment derrière ... pas intéressant.
					continue;

				Float	DistClosest = DeltaClosest.GetNorm();
				Float	CurScore = DistClosest;
				if ((Dp < 0.f) & (DistClosest > 0.001f))
				{
					// In repousse un peu les points... mais peut être source d'erreur.
					Vec3f	Dir = DeltaClosest * (1.f / DistClosest);
					Float	Ratio = Abs(FaceNormal * Dir);
					Float DeltaBack = Ratio * CONST_BackFaceDist;	// BackFace are 2cm back...
					DistClosest -= DeltaBack;
					ClosestPt -= Dir * DeltaBack;
					CurScore += 0.01f; // Score Malus !
				}

				// Manage Multi-Result
				if (_Report.Closest.pFace)
				{
					Float DeltaScore = Abs(CurScore - _Report.Closest.Score);
					if ((DeltaScore < 0.002f) && (_Report.Closest.Inter - ClosestPt).GetNorm2() < (0.002f*0.002f))
					{
						// Add new point.
						_Report.Multi_SumNormal += FaceNormal;

						// Get New RefError Normal ?
						Float DotCur = _Report.Multi_SumNormal * _Report.Closest.Normal;
						Float DotNew = _Report.Multi_SumNormal * FaceNormal;
						if (DotNew < DotCur)
						{
							_Report.Closest.Normal = FaceNormal;
							_Report.Closest.pFace = pFace;
						}

						// Get This one ?
						if (CurScore < _Report.Closest.Score)
						{
							_Report.Closest.Inter = ClosestPt;
							_Report.Closest.DistInter = DistClosest;
							_Report.Closest.Score = CurScore;
						}
						continue;
					}
				}
				if (CurScore < _Report.Closest.Score)
				{
					_Report.Closest.pFace = pFace;
					_Report.Closest.Score = CurScore;
					_Report.Closest.DistInter = DistClosest;
					_Report.Closest.Normal = FaceNormal;
					_Report.Closest.Inter = ClosestPt;
					_Report.Multi_SumNormal = FaceNormal;
				}
			}
			break;
		}
		pZone = pZone->pNext;
	}

	return TRUE;
}

/**************************************************************************/

FINLINE_Z	void	HMapMeshInfos3D::ScanFaceRay(Vec3f &_Pos, Vec3f &_Dir, PlaySpace_ScanReport &_Report,Float _DistMax)
{
	// Coin le plus proche.
	Float	InvSizeCell = 1.f / m_SizeCell;
	Vec3f	LocalPos = (_Pos - m_PosMin) * InvSizeCell; // Only positiv value...
	Vec3f	FloorLocalPos(FLOORF(LocalPos.x), FLOORF(LocalPos.y),FLOORF(LocalPos.z));
	S32 x = (FloorLocalPos.x + 0.5f);
	S32 y = (FloorLocalPos.y + 0.5f);
	S32 z = (FloorLocalPos.z + 0.5f);


	PlaySpace_ScanMeshFace	*pCurFaceLocked = NULL;
	Vec3i	PosFaceLocked;

	m_pMapMeshDatas->NewFaceTag();
	_Report.Ray.Score = _DistMax;

	// Ici traiter mieux les min max...
	if ((x < 0) || (x >= m_NbCellX))
		return;
	if ((y < 0) || (y >= m_NbCellY))
		return;
	if ((z < 0) || (z >= m_NbCellZ))
		return;

	QuadTree_ScanMesh::Cell	*pCell = m_pMapMeshDatas->GetCell(x, z);
	ScanZoneFaceRay(_Pos, _Dir, pCell, y, _Report);
	if (_Report.Ray.pFace)
	{
		// Stop Only if collision inside CurVoxel.
		Vec3f	VoxelResult = ((_Pos + _Dir * _Report.Ray.Score) - m_PosMin) * InvSizeCell; // Using Score for have malus.
		PosFaceLocked.Set(VoxelResult.x, VoxelResult.y, VoxelResult.z);
		pCurFaceLocked = _Report.Ray.pFace;
		if ((PosFaceLocked.x == x) && (PosFaceLocked.y == y) && (PosFaceLocked.z == z))
			return;
	}

	Vec3f AbsDir = _Dir;
	LocalPos = LocalPos - FloorLocalPos;

	S32 dx = 0;
	if (AbsDir.x > 1e-3f)
		dx = 1;
	else if (AbsDir.x < -1e-3f)
	{
		AbsDir.x = -AbsDir.x;
		LocalPos.x = 1.f - LocalPos.x;
		dx = -1;
	}

	S32 dy = 0;
	if (AbsDir.y > 1e-3f)
		dy = 1;
	else if (AbsDir.y < -1e-3f)
	{
		AbsDir.y = -AbsDir.y;
		LocalPos.y = 1.f - LocalPos.y;
		dy = -1;
	}

	S32 dz = 0;
	if (AbsDir.z > 1e-3f)
		dz = 1;
	else if (AbsDir.z < -1e-3f)
	{
		AbsDir.z = -AbsDir.z;
		LocalPos.z = 1.f - LocalPos.z;
		dz = -1;
	}

	Float Dist = 0.f;
	Float DeltaNextX, DeltaNextY, DeltaNextZ;
	DeltaNextX = DeltaNextY = DeltaNextZ = 1e20f;

	while (Dist < _Report.Ray.Score)
	{
		if (dx)
			DeltaNextX = (1.f - LocalPos.x) / AbsDir.x;

		if (dy)
			DeltaNextY = (1.f - LocalPos.y) / AbsDir.y;

		if (dz)
			DeltaNextZ = (1.f - LocalPos.z) / AbsDir.z;

		if (DeltaNextX < DeltaNextY)
		{
			if (DeltaNextX < DeltaNextZ)
			{
				x += dx;
				if ((x < 0) || (x >= m_NbCellX))
					return;
				pCell = m_pMapMeshDatas->GetCell(x, z);
				LocalPos += DeltaNextX * AbsDir;
				LocalPos.x = 0;
				Dist += DeltaNextX;
			}
			else
			{
				z += dz;
				if ((z < 0) || (z >= m_NbCellZ))
					return;
				LocalPos += DeltaNextZ * AbsDir;
				LocalPos.z = 0;
				Dist += DeltaNextZ;
			}
		}
		else if (DeltaNextY < DeltaNextZ)
		{
			y += dy;
			if ((y < 0) || (y >= m_NbCellY))
				return;
			LocalPos += DeltaNextY * AbsDir;
			LocalPos.y = 0;
			Dist += DeltaNextY;
		}
		else
		{
			z += dz;
			if ((z < 0) || (z >= m_NbCellZ))
				return;
			pCell = m_pMapMeshDatas->GetCell(x, z);
			LocalPos += DeltaNextZ * AbsDir;
			LocalPos.z = 0;
			Dist += DeltaNextZ;
		}

		ScanZoneFaceRay(_Pos, _Dir, pCell, y, _Report);
		if (_Report.Ray.pFace)
		{
			// Stop Only if collision inside CurVoxel.
			if (pCurFaceLocked != _Report.Ray.pFace)
			{
				// Stop Only if collision inside CurVoxel.
				Vec3f	VoxelResult = ((_Pos + _Dir * _Report.Ray.Score) - m_PosMin) * InvSizeCell;	// Using Score for have malus.
				PosFaceLocked.Set(VoxelResult.x, VoxelResult.y, VoxelResult.z);
				pCurFaceLocked = _Report.Ray.pFace;
			}
			if ((PosFaceLocked.x == x) && (PosFaceLocked.y == y) && (PosFaceLocked.z == z))
				return;
		}
	}
}

/**************************************************************************/

FINLINE_Z	Bool	HMapMeshInfos3D::ScanZoneFaceRay(Vec3f &_Pos, Vec3f &_Dir, QuadTree_ScanMesh::Cell *_pCell, S32 _y, PlaySpace_ScanReport &_Report)
{
	U16	CurFaceTag = m_pMapMeshDatas->m_CurFaceTag;
	QuadTree_ScanMesh::ObjectChain *pZone = _pCell->pFirst;
	while (pZone)
	{
		if ((_y >= pZone->hMin) && (_y <= pZone->hMax))
		{
			// Found the Zone !
			PlaySpace_ScanMeshRefFace *pRefFace = pZone->pFirstRefFace;
			for (; pRefFace != 0; pRefFace = pRefFace->pNext)
			{
				// Don't manage twice same face.
				PlaySpace_ScanMeshFace *pFace = pRefFace->pFace;

				if (pFace->Tag == CurFaceTag)
					continue;
				pFace->Tag = CurFaceTag;
				// Proj Point on Face.
				Vec3f	&FaceNormal = pFace->Normal;
				Vec3f	&p0 = pFace->p0;

				Vec3f	vDelta = _Pos - p0;
				Float	Dp = vDelta * FaceNormal;
				Float	CosAngle = FaceNormal * _Dir;

				if ((CosAngle < 1e-4f) && (CosAngle > -1e-4f))
					continue;

				Float	RayDistSurf = -Dp / CosAngle;
				if (RayDistSurf < 0.f) // Si la face est vraiment derrière ... pas intéressant.
					continue;

				Float	CurScore = RayDistSurf;
				if (Dp < 0.f) //|| (CosAngle > 0)) => CosAngle is > 0 if RayDistSurf >=0
				{
					RayDistSurf -= CONST_BackFaceDist;	// BackFace are 2cm back...
					CurScore += 0.01f; // Score Malus !
				}
				if (CurScore > _Report.Ray.Score)
					continue;

				// Intersect
				Vec3f	PtInter = _Dir*RayDistSurf + _Pos;

				// Compute Normal.
				Vec3f	&p1 = pFace->p1;
				Vec3f	&p2 = pFace->p2;

				Vec3f N0 = FaceNormal ^ (p1 - p0);
				Float d0 = N0*(PtInter - p0);
				if (d0 > 1e-4f)
					continue;

				Vec3f N1 = FaceNormal ^ (p2 - p1);
				Float d1 = N1*(PtInter - p1);
				if (d1 > 1e-4f)
					continue;

				Vec3f N2 = FaceNormal ^ (p0 - p2);
				Float d2 = N2*(PtInter - p2);
				if (d2 > 1e-4f)
					continue;

				// Inside Triangle.
				_Report.Ray.pFace = pFace;
				_Report.Ray.Score = CurScore;
				_Report.Ray.DistInter = RayDistSurf;
				_Report.Ray.Normal = FaceNormal;
				_Report.Ray.Inter = PtInter;
			}
			break;
		}
		pZone = pZone->pNext;
	}

	return TRUE;
}


/**************************************************************************/

void	HMapMeshInfos3D::PrepareScanReport(Playspace_Mesh &_Mesh,Bool _KeppMode)
{
	S32		NbPoints = _Mesh.m_TabPoints.GetSize();
	m_ScanReport.SetSize(NbPoints, TRUE);

	if (!_KeppMode || !m_pMapResultDatas)
	{
		for (S32 i = 0; i<NbPoints; i++)
			m_ScanReport[i].pKeepScan = NULL;
		return;
	}

	// Get Back OLD  Datas.
	Float	InvSizeCell = 1.f / m_SizeCell;
	Vec3i	iPos;
	Float	DistMax2 = _Mesh.GetSnapDist() * 0.99f;
	DistMax2*=DistMax2;

	for (S32 i = 0; i<NbPoints; i++)
	{
		Vec3f	pos = _Mesh.m_TabPoints[i];
		Float	InvSizeCell = 1.f / m_SizeCell;

		iPos.x = FLOOR((pos.x - m_PosMin.x) * InvSizeCell + 0.001f);
		iPos.y = FLOOR((pos.y - m_PosMin.y) * InvSizeCell + 0.001f);
		iPos.z = FLOOR((pos.z - m_PosMin.z) * InvSizeCell + 0.001f);
		PlaySpace_KeepScanInfos	*pZone = m_pMapResultDatas->GetCubicPoint(iPos,pos,DistMax2,TRUE);
		m_ScanReport[i].pKeepScan = pZone;
		pZone->bIsUsed = TRUE;
	}
}

/**************************************************************************/

#define CMC_DELTA(x,y,z) (13 + x*9 + y*3 + z)
#define FLOOD_DELTA(x,y,z) ((x<<2) + (y<<1) + z)
class CreateMeshCache
{
public:
	U8		IsBlock : 1, IsSpace : 1, IsPaintMode : 2,IsSeenQuality : 2;	// if !IsBlock && !IsSpace => IsNothing.

	FINLINE_Z static void	ClearDatas(CreateMeshCache *_pDatas, S32 _Size)
	{
		memset(_pDatas, 0, _Size * sizeof(CreateMeshCache));
	}
	FINLINE_Z static Float	ExtractInfos(CreateMeshCache *_pInfos, S32 _Delta1, S32 _Delta2, Float _SnapDist)
	{
		if (_pInfos[_Delta2].IsSpace)
			return -_SnapDist;
		else if (!_pInfos[_Delta1].IsSpace)
			return _SnapDist;
		return 0.f;
	}
};

FINLINE_Z static Bool GetBarycenterDelta(CreateMeshCache *_pTabInfos, S32 _DX, S32 _DY, S32 _DZ, Float _Dist, Vec3f &_Delta)
{
	U8		TabFlood[8];
	Vec3f	P000;

	((U32*)TabFlood)[0] = 0;
	((U32*)TabFlood)[1] = 0;

	TabFlood[FLOOD_DELTA(0, 0, 0)] = 1;

	if (!_pTabInfos[CMC_DELTA(_DX, 0, 0)].IsSpace)
		TabFlood[FLOOD_DELTA(1, 0, 0)] = 1;
	if (!_pTabInfos[CMC_DELTA(0, _DY, 0)].IsSpace)
		TabFlood[FLOOD_DELTA(0, 1, 0)] = 1;
	if (!_pTabInfos[CMC_DELTA(0, 0, _DZ)].IsSpace)
		TabFlood[FLOOD_DELTA(0, 0, 1)] = 1;

	S32 NbBlocsSet = 4;
	S32 NbBlocsDetect = 4;

	if (!_pTabInfos[CMC_DELTA(_DX, _DY, 0)].IsSpace)
	{
		NbBlocsDetect++;
		if (TabFlood[FLOOD_DELTA(1, 0, 0)] || TabFlood[FLOOD_DELTA(0, 1, 0)])
		{
			TabFlood[FLOOD_DELTA(1, 1, 0)] = 1;
			NbBlocsSet++;
		}
	}

	if (!_pTabInfos[CMC_DELTA(_DX, 0, _DZ)].IsSpace)
	{
		NbBlocsDetect++;
		if (TabFlood[FLOOD_DELTA(1, 0, 0)] || TabFlood[FLOOD_DELTA(0, 0, 1)])
		{
			TabFlood[FLOOD_DELTA(1, 0, 1)] = 1;
			NbBlocsSet++;
		}
	}

	if (!_pTabInfos[CMC_DELTA(0, _DY, _DZ)].IsSpace)
	{
		NbBlocsDetect++;
		if (TabFlood[FLOOD_DELTA(0, 1, 0)] || TabFlood[FLOOD_DELTA(0, 0, 1)])
		{
			TabFlood[FLOOD_DELTA(0, 1, 1)] = 1;
			NbBlocsSet++;
		}
	}

	if (!_pTabInfos[CMC_DELTA(_DX, _DY, _DZ)].IsSpace)
	{
		NbBlocsDetect++;
		if (TabFlood[FLOOD_DELTA(1, 1, 0)] || TabFlood[FLOOD_DELTA(0, 1, 1)] || TabFlood[FLOOD_DELTA(1, 0, 1)])
		{
			TabFlood[FLOOD_DELTA(1, 1, 1)] = 1;
			NbBlocsSet++;

			// Back FILL !
			if (!TabFlood[FLOOD_DELTA(1, 1, 0)] && !_pTabInfos[CMC_DELTA(_DX, _DY, 0)].IsSpace)
			{
				TabFlood[FLOOD_DELTA(1, 1, 0)] = 1;
				NbBlocsSet++;
			}

			if (!TabFlood[FLOOD_DELTA(1, 0, 1)] && !_pTabInfos[CMC_DELTA(_DX, 0, _DZ)].IsSpace)
			{
				TabFlood[FLOOD_DELTA(1, 0, 1)] = 1;
				NbBlocsSet++;
			}
			if (!TabFlood[FLOOD_DELTA(0, 1, 1)] && !_pTabInfos[CMC_DELTA(0, _DY, _DZ)].IsSpace)
			{
				TabFlood[FLOOD_DELTA(0, 1, 1)] = 1;
				NbBlocsSet++;
			}
		}
	}

	EXCEPTIONC_Z(NbBlocsSet <= NbBlocsDetect, "CreateMesh : Flood Pb");
	if (NbBlocsSet == NbBlocsDetect)
	{
		_Delta = VEC3F_NULL;
		return FALSE;
	}
	Vec3i iGravityPt = VEC3I_NULL;
	for (S32 x = 0; x<2; x++)
	{
		for (S32 y = 0; y<2; y++)
		{
			if (TabFlood[FLOOD_DELTA(x, y, 0)])
			{
				iGravityPt.x += x;
				iGravityPt.y += y;
			}
			if (TabFlood[FLOOD_DELTA(x, y, 1)])
			{
				iGravityPt.x += x;
				iGravityPt.y += y;
				iGravityPt.z++;
			}
		}
	}

	Vec3f GravityPt(iGravityPt.x, iGravityPt.y, iGravityPt.z);
	GravityPt /= NbBlocsSet;
	_Delta.x = GravityPt.x - 1.f;
	_Delta.y = GravityPt.y - 1.f;
	_Delta.z = GravityPt.z - 1.f;
	_Delta.CNormalize(_Dist);

	if (_DX < 0)
		_Delta.x = -_Delta.x;
	if (_DY < 0)
		_Delta.y = -_Delta.y;
	if (_DZ < 0)
		_Delta.z = -_Delta.z;

	return TRUE;
}

void	HMapMeshInfos3D::CreateCubicEnvelop(Playspace_Mesh	&_Mesh,Bool _DontAddVirtual)
{
	// Init Cache.
	CreateMeshCache		*LineCache[3];
	S32					SizeCache = (m_NbCellX + 4) * (m_NbCellY + 4);		// Add buffer at each extremity (2 cells x 2).
	S32					DeltaColumn = m_NbCellY + 4;
	S32					StartColumn = DeltaColumn + DeltaColumn + 2;

	LineCache[0] = New_Z CreateMeshCache[SizeCache];
	CreateMeshCache::ClearDatas(LineCache[0], SizeCache);

	LineCache[1] = New_Z CreateMeshCache[SizeCache];
	CreateMeshCache::ClearDatas(LineCache[1], SizeCache);

	LineCache[2] = New_Z CreateMeshCache[SizeCache];
	CreateMeshCache::ClearDatas(LineCache[2], SizeCache);

	Float SnapDist = _Mesh.GetSnapDist();

	S32		ClearDelta = DeltaColumn + DeltaColumn + 2;
	S32		ClearSize = SizeCache - ClearDelta - ClearDelta;

	// Pass 1 : Create CUBIC Envelopp...
	for (S32 z = -1; z <= m_NbCellZ; z++)
	{
		// Step 1 : Fill Next Line Cache
		S32 NextZ = z + 1;
		CreateMeshCache::ClearDatas(LineCache[2] + ClearDelta, ClearSize);
		CreateMeshCache	*pStartNextLine = LineCache[2] + StartColumn;

		if (NextZ < m_NbCellZ)
		{
			for (S32 x = 0; x<m_NbCellX; x++)
			{
				CreateMeshCache	 *pCurColumn = pStartNextLine + x*DeltaColumn;

				QuadTree_ScanMesh::Cell	*pCell = m_pMapMeshDatas->GetCell(x, NextZ);
				if (!pCell)
					continue;

				QuadTree_ScanMesh::ObjectChain *pCurZone = pCell->pFirst;
				while (pCurZone)
				{
					if (pCurZone->pFirstRefFace)
					{
						// RefFaces !! Set in Cache.
						EXCEPTIONC_Z(pCurZone->hMin >= 0, "CreateMesh Bad Block 1");
						EXCEPTIONC_Z(pCurZone->hMin < m_NbCellY, "CreateMesh Bad Block 2");
						CreateMeshCache &CurCache = pCurColumn[pCurZone->hMin];
						CurCache.IsBlock = TRUE;
						CurCache.IsPaintMode = pCurZone->IsPaintMode;
						CurCache.IsSeenQuality = pCurZone->IsSeenQuality;
					}
					else
					{
						// Space !! Set in Cache.
						EXCEPTIONC_Z(pCurZone->hMin >= 0, "CreateMesh Bad Block 1");
						EXCEPTIONC_Z(pCurZone->hMax < m_NbCellY, "CreateMesh Bad Block 2");
						for (S32 i = pCurZone->hMin; i <= pCurZone->hMax; i++)
							pCurColumn[i].IsSpace = TRUE;
					}
					// Next.
					pCurZone = pCurZone->pNext;
				}
			}
		}

		// Step 2 :  Create Faces for CurLine.
		Vec3f			MyMin, MyMax;
		CreateMeshCache	TabInfos[3 * 3 * 3];
		Vec3f			Face[4];

		for (S32 x = -1; x <= m_NbCellX; x++)
		{
			S32 Delta = x*DeltaColumn + StartColumn - 1;		// -1 because y start at -1...
			CreateMeshCache	 *pPrevColumn = LineCache[0] + Delta;
			CreateMeshCache	 *pCurColumn = LineCache[1] + Delta;
			CreateMeshCache	 *pNextColumn = LineCache[2] + Delta;

			for (S32 y = -1; y <= m_NbCellY; y++, pPrevColumn++, pCurColumn++, pNextColumn++)
			{
				// Nothing to draw if is space.
				if (pCurColumn->IsSpace)
					continue;

				// Noything to draw if there's NO space neighbor.
				S32 IsOk = pPrevColumn->IsSpace;
				IsOk += pNextColumn->IsSpace;
				IsOk += pCurColumn[-1].IsSpace;
				IsOk += pCurColumn[1].IsSpace;
				IsOk += pCurColumn[-DeltaColumn].IsSpace;
				IsOk += pCurColumn[DeltaColumn].IsSpace;

				if (!IsOk)
					continue;

				// Something to DRAW !!!

				MyMin.x = m_PosMin.x + m_SizeCell * x;
				MyMin.y = m_PosMin.y + m_SizeCell * y;
				MyMin.z = m_PosMin.z + m_SizeCell * z;
				MyMax.x = MyMin.x + m_SizeCell;
				MyMax.y = MyMin.y + m_SizeCell;
				MyMax.z = MyMin.z + m_SizeCell;

				// Local Cache Fill.
				for (S32 ix = 0; ix<3; ix++)
				{
					S32 iDeltaSrcX = (ix - 1) * DeltaColumn - 1;
					S32 iDeltaDestX = ix * 9;
					for (S32 iy = 0; iy<3; iy++)
					{
						S32 iDest = iy * 3 + iDeltaDestX;
						S32 iSrc = iy + iDeltaSrcX;
						TabInfos[iDest + 0] = pPrevColumn[iSrc];
						TabInfos[iDest + 1] = pCurColumn[iSrc];
						TabInfos[iDest + 2] = pNextColumn[iSrc];
					}
				}

				// Compute	all points.
				Vec3f	vDelta;

				Vec3f	P000(MyMin.x, MyMin.y, MyMin.z);
				if (GetBarycenterDelta(TabInfos, -1, -1, -1, SnapDist, vDelta))
					P000 += vDelta;

				Vec3f	P100(MyMax.x, MyMin.y, MyMin.z);
				if (GetBarycenterDelta(TabInfos, 1, -1, -1, SnapDist, vDelta))
					P100 += vDelta;

				Vec3f	P010(MyMin.x, MyMax.y, MyMin.z);
				if (GetBarycenterDelta(TabInfos, -1, 1, -1, SnapDist, vDelta))
					P010 += vDelta;

				Vec3f	P110(MyMax.x, MyMax.y, MyMin.z);
				if (GetBarycenterDelta(TabInfos, 1, 1, -1, SnapDist, vDelta))
					P110 += vDelta;

				Vec3f	P001(MyMin.x, MyMin.y, MyMax.z);
				if (GetBarycenterDelta(TabInfos, -1, -1, 1, SnapDist, vDelta))
					P001 += vDelta;

				Vec3f	P101(MyMax.x, MyMin.y, MyMax.z);
				if (GetBarycenterDelta(TabInfos, 1, -1, 1, SnapDist, vDelta))
					P101 += vDelta;

				Vec3f	P011(MyMin.x, MyMax.y, MyMax.z);
				if (GetBarycenterDelta(TabInfos, -1, 1, 1, SnapDist, vDelta))
					P011 += vDelta;

				Vec3f	P111(MyMax.x, MyMax.y, MyMax.z);
				if (GetBarycenterDelta(TabInfos, 1, 1, 1, SnapDist, vDelta))
					P111 += vDelta;

				// UP Face ?
				Bool IsVirtual = (pCurColumn->IsBlock == FALSE);
				if (!_DontAddVirtual || !IsVirtual)
				{
					Bool AddIt = (pCurColumn[1].IsSpace == TRUE);

					if (AddIt)
					{
						Face[3] = P110;
						Face[2] = P010;
						Face[1] = P011;
						Face[0] = P111;
						_Mesh.AddSurface(Face, 4, IsVirtual,pCurColumn->IsPaintMode,pCurColumn->IsSeenQuality);
					}

					// DOWN Face ?
					AddIt = (pCurColumn[-1].IsSpace == TRUE);
					if (AddIt)
					{
						Face[3] = P000;
						Face[2] = P100;
						Face[1] = P101;
						Face[0] = P001;
						_Mesh.AddSurface(Face, 4, IsVirtual,pCurColumn->IsPaintMode,pCurColumn->IsSeenQuality);
					}

					// LEFT Face ?
					AddIt = (pCurColumn[DeltaColumn].IsSpace == TRUE);
					if (AddIt)
					{
						Face[3] = P101;
						Face[2] = P100;
						Face[1] = P110;
						Face[0] = P111;
						_Mesh.AddSurface(Face, 4, IsVirtual,pCurColumn->IsPaintMode,pCurColumn->IsSeenQuality);
					}

					// RIGHT Face ?
					AddIt = (pCurColumn[-DeltaColumn].IsSpace == TRUE);
					if (AddIt)
					{
						Face[3] = P000;
						Face[2] = P001;
						Face[1] = P011;
						Face[0] = P010;
						_Mesh.AddSurface(Face, 4, IsVirtual,pCurColumn->IsPaintMode,pCurColumn->IsSeenQuality);
					}

					// FRONT Face ?
					AddIt = (pNextColumn[0].IsSpace == TRUE);
					if (AddIt)
					{
						Face[3] = P001;
						Face[2] = P101;
						Face[1] = P111;
						Face[0] = P011;
						_Mesh.AddSurface(Face, 4, IsVirtual,pCurColumn->IsPaintMode,pCurColumn->IsSeenQuality);
					}

					// BACK Face ?
					AddIt = (pPrevColumn[0].IsSpace == TRUE);
					if (AddIt)
					{
						Face[3] = P100;
						Face[2] = P000;
						Face[1] = P010;
						Face[0] = P110;
						_Mesh.AddSurface(Face, 4, IsVirtual,pCurColumn->IsPaintMode,pCurColumn->IsSeenQuality);
					}
				}
			}
		}

		// Roll Cache.
		CreateMeshCache	 *pWork = LineCache[0];
		LineCache[0] = LineCache[1];
		LineCache[1] = LineCache[2];
		LineCache[2] = pWork;
	}

	// Free Cache.
	Delete_Z LineCache[2];
	Delete_Z LineCache[1];
	Delete_Z LineCache[0];
}

/**************************************************************************/

FINLINE_Z	void	HMapMeshInfos3D::GetMinMax(Vec3i &_Pos, const Vec3f &_vDir, S32 _Axis, Float &_Min, Float &_Max)
{
	_Min = m_PosMin[_Axis] + m_SizeCell * _Pos[_Axis];
	_Max = _Min + m_SizeCell;

	if (_Pos.x < 0)
		return;
	if (_Pos.x >= m_NbCellX)
		return;
	if (_Pos.z < 0)
		return;
	if (_Pos.z >= m_NbCellZ)
		return;

	QuadTree_ScanMesh::Cell	*pCell = m_pMapMeshDatas->GetCell(_Pos.x, _Pos.z);

	QuadTree_ScanMesh::ObjectChain *pZone = pCell->pFirst;
	S32 _y = _Pos.y;
	while (pZone)
	{
		if ((_y >= pZone->hMin) && (_y <= pZone->hMax))
		{
			// Found the Zone !
			PlaySpace_ScanMeshRefFace *pRefFace = pZone->pFirstRefFace;
			EXCEPTIONC_Z(pRefFace != NULL, "GetMinMax : Bad Input");
			if (pRefFace)
			{
				PlaySpace_ScanMeshFace *pFace = pRefFace->pFace;
				Float FaceMin = pFace->p0[_Axis];
				Float FaceMax = pFace->p0[_Axis];
				while (pRefFace)
				{
					pFace = pRefFace->pFace;
					Float vFNormal = pFace->Normal[_Axis];
					vFNormal *= CONST_BackFaceDist;	// 2cm de degagement.
					Float pAxis = pFace->p0[_Axis];
					FaceMin = Min(pAxis, FaceMin);FaceMax = Max(pAxis, FaceMax);
					pAxis -= vFNormal; // Back
					FaceMin = Min(pAxis, FaceMin);FaceMax = Max(pAxis, FaceMax);

					pAxis = pFace->p1[_Axis];
					FaceMin = Min(pAxis, FaceMin);FaceMax = Max(pAxis, FaceMax);
					pAxis -= vFNormal; // Back
					FaceMin = Min(pAxis, FaceMin);FaceMax = Max(pAxis, FaceMax);

					pAxis = pFace->p2[_Axis];
					FaceMin = Min(pAxis, FaceMin);FaceMax = Max(pAxis, FaceMax);
					pAxis -= vFNormal; // Back
					FaceMin = Min(pAxis, FaceMin);FaceMax = Max(pAxis, FaceMax);

					// Next.
					pRefFace = pRefFace->pNext;
				}
				_Min = Max(FaceMin, _Min);
				_Max = Min(FaceMax, _Max);
			}
			break;
		}
		pZone = pZone->pNext;
	}

	return;
}

/**************************************************************************/

FINLINE_Z	void	HMapMeshInfos3D::SnapMinMax(Playspace_Mesh &_Mesh, SafeArray_Z<S32, 8, FALSE, FALSE> &_TabFace, S32 _NbFaces, Vec3f &_vPos, const Vec3f &_vDir, S32 _Axis, S32 _IsPositivNormal)
{
	Float	LocalMinMax = _IsPositivNormal ? -1e10 : 1e10;
	Float	InvSizeCell = 1.f / m_SizeCell;

	for (S32 iFace = 0; iFace<_NbFaces; iFace++)
	{
		S32 CurNumFace = _TabFace[iFace];
		Playspace_Mesh::ToolFaceNormal &CurFaceInfos = _Mesh.m_TabFaceToolNormal[CurNumFace];
		Vec3f	&Center = CurFaceInfos.Center;

		Vec3i iPos;
		iPos.x = FLOOR((Center.x - m_PosMin.x) * InvSizeCell - _vDir.x * 0.5f);
		iPos.y = FLOOR((Center.y - m_PosMin.y) * InvSizeCell - _vDir.y * 0.5f);
		iPos.z = FLOOR((Center.z - m_PosMin.z) * InvSizeCell - _vDir.z * 0.5f);

		Float fMin, fMax;
		GetMinMax(iPos, _vDir, _Axis, fMin, fMax);

		if (_IsPositivNormal)
			LocalMinMax = Max(LocalMinMax, fMax);
		else
			LocalMinMax = Min(LocalMinMax, fMin);
	}

	if (_IsPositivNormal)
		_vPos[_Axis] = Min(LocalMinMax, _vPos[_Axis]);
	else
		_vPos[_Axis] = Max(LocalMinMax, _vPos[_Axis]);
}

/**************************************************************************/

void	HMapMeshInfos3D::PushPerpendicular(Playspace_Mesh	&_Mesh)
{
	SafeArray_Z<S32, 8, FALSE, FALSE>	TabPX;
	SafeArray_Z<S32, 8, FALSE, FALSE>	TabNX;

	SafeArray_Z<S32, 8, FALSE, FALSE>	TabPY;
	SafeArray_Z<S32, 8, FALSE, FALSE>	TabNY;

	SafeArray_Z<S32, 8, FALSE, FALSE>	TabPZ;
	SafeArray_Z<S32, 8, FALSE, FALSE>	TabNZ;

	// Push Envelopp point into Datas
	PlaySpace_ScanReport CurReport;
	S32		NbPoints = _Mesh.m_TabPoints.GetSize();
	for (S32 i = 0; i<NbPoints; i++)
	{
		PlaySpace_ScanReport &CurReport = m_ScanReport[i];
		if (CurReport.pKeepScan && !CurReport.pKeepScan->bIsNew)
			continue;

		Vec3f	pos = _Mesh.m_TabPoints[i];
		Playspace_Mesh::PointsLinks &PointLinks = _Mesh.m_TabPointsLinks[i];

		S32		NbPX = 0;
		S32		NbNX = 0;

		S32		NbPY = 0;
		S32		NbNY = 0;

		S32		NbPZ = 0;
		S32		NbNZ = 0;
		S32 NbLinkFaces = PointLinks.GetNbFaces();
		for (S32 iFace = 0; iFace<NbLinkFaces; iFace++)
		{
			S32 CurNumFace = PointLinks.GetFace(iFace);
			Playspace_Mesh::ToolFaceNormal &CurFaceInfos = _Mesh.m_TabFaceToolNormal[CurNumFace];

			if (CurFaceInfos.Normal.x > 0.5f)
				TabPX[NbPX++] = CurNumFace;
			else if (CurFaceInfos.Normal.x < -0.5f)
				TabNX[NbNX++] = CurNumFace;

			if (CurFaceInfos.Normal.y > 0.5f)
				TabPY[NbPY++] = CurNumFace;
			else if (CurFaceInfos.Normal.y < -0.5f)
				TabNY[NbNY++] = CurNumFace;

			if (CurFaceInfos.Normal.z > 0.5f)
				TabPZ[NbPZ++] = CurNumFace;
			else if (CurFaceInfos.Normal.z < -0.5f)
				TabNZ[NbNZ++] = CurNumFace;
		}

		// Coin le plus proche.
		Vec3f	DstPosY = pos;
		if (NbPY && !NbNY)
			SnapMinMax(_Mesh, TabPY, NbPY, DstPosY, VEC3F_UP, 1, TRUE);
		if (!NbPY && NbNY)
			SnapMinMax(_Mesh, TabNY, NbNY, DstPosY, VEC3F_DOWN, 1, FALSE);

		Vec3f	DstPosX = pos;
		if (NbPX && !NbNX)
			SnapMinMax(_Mesh, TabPX, NbPX, DstPosX, VEC3F_LEFT, 0, TRUE);
		if (!NbPX && NbNX)
			SnapMinMax(_Mesh, TabNX, NbNX, DstPosX, VEC3F_RIGHT, 0, FALSE);

		Vec3f	DstPosZ = pos;
		if (NbPZ && !NbNZ)
			SnapMinMax(_Mesh, TabPZ, NbPZ, DstPosZ, VEC3F_FRONT, 2, TRUE);
		if (!NbPZ && NbNZ)
			SnapMinMax(_Mesh, TabNZ, NbNZ, DstPosZ, VEC3F_BACK, 2, FALSE);

		pos += (DstPosX - pos) + (DstPosY - pos) + (DstPosZ - pos);

		// Have to Fix Result ?
		Vec3f	OriginalPos = _Mesh.m_TabPoints[i];
		Vec3f	Dir = pos - OriginalPos;
		Float	DeltaLen = Dir.GetNorm();
		if (DeltaLen < 0.001f)
			continue;

		Dir *= 1.f / DeltaLen;
		CurReport.ClearResult();
		ScanFaceRay(OriginalPos, Dir, CurReport, DeltaLen+0.01f);
		if (CurReport.Ray.pFace && (CurReport.Ray.DistInter < DeltaLen))
			pos = OriginalPos + (CurReport.Ray.DistInter - 0.0001f)*Dir;

		_Mesh.MovePoint(i, pos);
	}
}

/**************************************************************************/

FINLINE_Z void ComputeNormalWithoutEdge(Playspace_Mesh &_Mesh, S32 _WithP1, S32 _WithOutP2, Vec3f &_Normal)
{
	Playspace_Mesh::PointsLinks &PointLinks1 = _Mesh.m_TabPointsLinks[_WithP1];

	_Normal= VEC3F_NULL;

	S32 nbf1 = PointLinks1.GetNbFaces();
	for (S32 i = 0; i<nbf1 ; i++)
	{
		S32 NumF = PointLinks1.GetFace(i);
		Playspace_Mesh::Face &CurFace = _Mesh.m_TabQuad[NumF];

		Bool UseNormal = TRUE;
		S32 NbPts = CurFace.IsTri ? 3 : 4;
		for (S32 j = 0; j<NbPts ; j++)
		{
			if (CurFace.TabPoints[j] == _WithOutP2)
			{
				UseNormal = FALSE;
				break;
			}
		}
		if (UseNormal)
			_Normal += _Mesh.m_TabFaceToolNormal[NumF].Normal;
	}

	_Normal.CNormalize();
}

void	HMapMeshInfos3D::PushFromFaces(Playspace_Mesh	&_Mesh)
{
	SafeArray_Z<S32,128,FALSE,FALSE>	TabReference;

	// Step 1 : Scan Datas (Closest + Ray => principal dir).
	PlaySpace_ScanReport	NewReport;
	S32		NbPoints = _Mesh.m_TabPoints.GetSize();
	for (S32 i = 0; i<NbPoints; i++)
	{
		PlaySpace_ScanReport &CurReport = m_ScanReport[i];
		if (CurReport.pKeepScan && !CurReport.pKeepScan->bIsNew)
			continue;

		Vec3f	pos = _Mesh.m_TabPoints[i];
		Playspace_Mesh::ToolPointNormal &PointInfos = _Mesh.m_TabPointsToolNormal[i];
		Vec3f	DirRay = -PointInfos.Normal;

		CurReport.ClearResult();
		ScanFaceRay(pos, DirRay, CurReport, 0.25f);
		ScanFaceClosest(pos, DirRay, CurReport,Min(0.25f, CurReport.Ray.Score));
		CurReport.OriginalPos = pos;
		CurReport.ChoosedResult = 0;
		CurReport.RayDir = DirRay;

		// No more to do.
		if (!CurReport.Closest.pFace)	// if no closest => nothing.
			continue;

		// Compute Closest Principal Dir.
		// Principale dir : n'a d'intérèt que si c'est FLAT... sinon, on ne devrait pas.
		// => Peut être découper en 2.
		Vec3f ActualDir;
		if (CurReport.Closest.DistInter >= 0)	// If closest neg => only possible if point is moved before origin.
			ActualDir = CurReport.Closest.Inter - pos;
		else
			ActualDir = pos - CurReport.Closest.Inter;
		ActualDir.ANormalize();

		Vec3f AlignRayDir = CurReport.Closest.Normal;
		Float ValDir = (ActualDir * AlignRayDir);
		if (ValDir < 0)
		{
			AlignRayDir = -AlignRayDir;
			ValDir = -ValDir;
		}
		if (ValDir < 0.707f)
		{
			// Only closest !
			CurReport.Ray.pFace = NULL;
			continue;
		}

		if (Abs(AlignRayDir.x) > Abs(AlignRayDir.y))
		{
			AlignRayDir.y = 0.f;
			if (Abs(AlignRayDir.x) > Abs(AlignRayDir.z))
			{
				AlignRayDir.z = 0.f;
				AlignRayDir.x = (AlignRayDir.x > 0) ? 1.f : -1.f;
			}
			else
			{
				AlignRayDir.x = 0.f;
				AlignRayDir.z = (AlignRayDir.z > 0) ? 1.f : -1.f;
			}
		}
		else
		{
			AlignRayDir.x = 0.f;
			if (Abs(AlignRayDir.y) > Abs(AlignRayDir.z))
			{
				AlignRayDir.z = 0.f;
				AlignRayDir.y = (AlignRayDir.y > 0) ? 1.f : -1.f;
			}
			else
			{
				AlignRayDir.y = 0.f;
				AlignRayDir.z = (AlignRayDir.z > 0) ? 1.f : -1.f;
			}
		}

		// Is Ray Dir already OK ?
		if (CurReport.Ray.pFace)
		{
			if (DirRay * AlignRayDir < 0.99f)
			{
				// ReScan.
				NewReport.ClearResult();
				ScanFaceRay(pos, AlignRayDir, NewReport,0.25f);
				if (NewReport.Ray.pFace)
					CurReport.Ray = NewReport.Ray;
				else
					CurReport.Ray.pFace = NULL;
			}
		}
		CurReport.RayDir = AlignRayDir;
	}

	// Step 2 : Choose between Ray dir and Closest.
	Float	HalfSizeCell = m_SizeCell * 0.5f;
	Float	InvSizeCell = 1.f / m_SizeCell;
	Float	CosFlat = 0.9f;
				
	for (S32 i = 0; i<NbPoints; i++)
	{
		Vec3f	pos = _Mesh.m_TabPoints[i];

		PlaySpace_ScanReport &CurReport = m_ScanReport[i];
		if (CurReport.pKeepScan && !CurReport.pKeepScan->bIsNew)
		{
			// Keep old scan.
			_Mesh.MovePoint(i, CurReport.pKeepScan->ResultPos);
			continue;
		}

		// Nothing to do.
		if (!CurReport.Closest.pFace)	// If no closest : nothing.
		{
			CurReport.ChoosedResult = 0;
			CurReport.IsPointFlat = FALSE;
			continue;
		}

		Bool IsPointFlat = IsReportFlat(CurReport, NULL, CosFlat);
		CurReport.IsPointFlat = IsPointFlat;

		if (CurReport.Ray.pFace && IsPointFlat)
		{
			// Now Test Neighbor.
			Bool IsFlat = TRUE;

			Playspace_Mesh::PointsLinks &PointLinks = _Mesh.m_TabPointsLinks[i];
			Vec3f &MyRayDir = CurReport.RayDir;
			S32 NbLinkFaces = PointLinks.GetNbFaces();
			for (S32 j = 0; j <NbLinkFaces ; j++)
			{
				Playspace_Mesh::Face &CurFace = _Mesh.m_TabQuad[PointLinks.GetFace(j)];
				S32 NbPt = CurFace.IsTri ? 3 : 4;
				for (S32 iPt = 0; iPt<NbPt; iPt++)
				{
					S32 NumPt = CurFace.TabPoints[iPt];
					PlaySpace_ScanReport &OtherReport = m_ScanReport[NumPt];
					if (!OtherReport.Ray.pFace)
					{
						IsFlat = FALSE;
						break;
					}

					Vec3f &OtherRayDir = OtherReport.RayDir;
//					if (!IsReportFlat(OtherReport, &RefNormal, CosFlat))
					if ((OtherRayDir * MyRayDir) < 0.99f)		// est ce suffisant : pour le moment oui...
					{
						IsFlat = FALSE;
						break;
					}
				}
				if (!IsFlat)
					break;
			}
			// If Flat : DO IT.
			if (IsFlat)
			{
				_Mesh.MovePoint(i, CurReport.Ray.Inter);
				CurReport.ChoosedResult = 1;
				if (CurReport.pKeepScan)
				{
					CurReport.pKeepScan->bIsNew = FALSE;
					CurReport.pKeepScan->ResultPos = CurReport.Ray.Inter;
				}
				continue;
			}
		}

		// Closest is better.
		Vec3f	MyFuturPos = CurReport.Closest.Inter;
		_Mesh.MovePoint(i, MyFuturPos);
		CurReport.ChoosedResult = 2;
		if (CurReport.pKeepScan)
		{
			CurReport.pKeepScan->bIsNew = FALSE;
			CurReport.pKeepScan->ResultPos = MyFuturPos;
		}
	}
	return;
}

/**************************************************************************/

void	HMapMeshInfos3D::CreateMesh(Playspace_Mesh	&_Mesh,Bool _FastMode,Bool _KeepMode)
{

	_Mesh.Flush(TRUE);
	_Mesh.SetEditMode(TRUE);
	
	// Step 1 : Create Cubic Envelop
	m_pMapMeshDatas->InitFaceTag();
	CreateCubicEnvelop(_Mesh,_FastMode);
	S32 NbPtEnvelop = _Mesh.m_TabPoints.GetSize();

	// Manage KeepMode.
	PrepareScanReport(_Mesh,_KeepMode);

	// Add Links.
	_Mesh.ComputePointsLinks(TRUE);
	_Mesh.ComputeFacesToolNormal(TRUE);	// Compute Face too.
	_Mesh.ComputePointsToolNormal(TRUE);	// Compute Face too.
	Float t3 = GetAbsoluteTime();

	// Push to mesh Faces.
	PushPerpendicular(_Mesh);

	PushFromFaces(_Mesh);

	if (!_FastMode)
	{
		//*m_pDebugMesh2 = _Mesh;
		PlanarVirtualZones(_Mesh);
	}
																						
	// Triangularize BAD QUAD.
	RemoveBadQuads(_Mesh);
	Float t6 = GetAbsoluteTime();

	// Suppress Edit Mode ?
	_Mesh.InvalidateFaceToolNormal();
	_Mesh.InvalidatePointsToolNormal();
	Float t7 = GetAbsoluteTime();
}

/**************************************************************************/

void	HMapMeshInfos3D::PlanarVirtualZones(Playspace_Mesh	&_Mesh)
{
	Float t0 = GetAbsoluteTime();
	_Mesh.ComputePointsLinks(FALSE);
	_Mesh.ComputeFacesToolNormal(TRUE);	// Compute Face too.

	Float t1 = GetAbsoluteTime();
	S32 nbPass1 = 5;
	S32 nbPass2 = 5;

	S32 nbPoints = _Mesh.m_TabPoints.GetSize();
	S32 nbFaces = _Mesh.m_TabQuad.GetSize();

	DynArray_Z<Vec3f, 32, FALSE, FALSE, 32, TRUE> TabNorm;
	TabNorm.SetSize(nbFaces);

	DynArray_Z<Vec3f, 32, FALSE, FALSE, 32, TRUE> TabPts;
	TabPts.SetSize(nbPoints);

	SafeArray_Z<S32,1024,FALSE,FALSE> doneFaces;

	HU8DA TabPtsMovable;
	TabPtsMovable.SetSize(nbPoints);
	memset(TabPtsMovable.GetArrayPtr(), 0, TabPtsMovable.GetSize());

	for (S32 i = 0; i < nbFaces; i++)
		TabNorm[i] = _Mesh.m_TabFaceToolNormal[i].Normal;

	for (S32 pass1 = 0; pass1 < nbPass1; pass1++)
	{
		for (S32 i = 0; i < nbFaces; i++)
		{
			Playspace_Mesh::Face& currFace = _Mesh.m_TabQuad[i];
			if (currFace.IsVirtual)
			{
				S32 nb = 4;
				if (currFace.IsTri)
					nb = 3;

				Vec3f norm = TabNorm[i];
				S32 nbDone = 0;
				doneFaces[nbDone++] = i;

				for (S32 j = 0; j < nb; j++)
				{
					S32 pt = currFace.TabPoints[j];
					TabPtsMovable[pt] = 1;
					Playspace_Mesh::PointsLinks& link = _Mesh.m_TabPointsLinks[pt];
					S32 nbFacesPt = link.GetNbFaces();
					for (S32 k = 0; k < nbFacesPt; k++)
					{
						S32 otherFaceIdx = link.GetFace(k);
						if (!_Mesh.m_TabQuad[otherFaceIdx].IsVirtual)
						{
							continue;
						}
						S32 idx = 0;
						while ((idx < nbDone) && (doneFaces[idx] != otherFaceIdx))
							idx++;

						if (idx == nbDone)
						{
							Float dot = TabNorm[i] * TabNorm[otherFaceIdx];
							if (dot >= -0.01f)
							{
								Float coef = (0.3f * dot * dot) + (0.575f * dot) + 0.125f;								// f(1)=1  ;	 ;  f(-1/4)=0
								norm += TabNorm[otherFaceIdx] * coef;
							}
							doneFaces[nbDone++] = otherFaceIdx;
						}
					}
				}
				norm.CNormalize();
				TabNorm[i] = norm;
			}
		}
	}

	Float t2 = GetAbsoluteTime();

	for (S32 i = 0; i < nbPoints; i++)
		TabPts[i] = _Mesh.m_TabPoints[i];

	for (S32 pass2 = 0; pass2 < nbPass2; pass2++)
	{
		for (S32 i = 0; i < nbPoints; i++)
		{
			if (TabPtsMovable[i])
			{
				Playspace_Mesh::PointsLinks& link = _Mesh.m_TabPointsLinks[i];
				S32 nbFacesPt = link.GetNbFaces();
				Vec3f delta = VEC3F_NULL;
				for (S32 j = 0; j < nbFacesPt; j++)
				{
					S32 currFaceIdx = link.GetFace(j);
					Playspace_Mesh::Face& currFace = _Mesh.m_TabQuad[currFaceIdx];
					//EXCEPTIONC_Z(currFace.IsVirtual, "Big badaboum: gros BUG !!!");
					S32 nb = 4;
					Float inv = 0.25f;
					if (currFace.IsTri)
					{
						nb = 3;
						inv = 0.333333333f;
					}

					Vec3f center = VEC3F_NULL;
					for (S32 k = 0; k < nb; k++)
						center += TabPts[currFace.TabPoints[k]];

					center *= inv;
					delta += TabNorm[currFaceIdx] * (TabNorm[currFaceIdx] * (center - TabPts[i]));
				}
				if (nbFacesPt)
					TabPts[i] += delta * (1.0f / nbFacesPt);
			}
		}
	}

	for (S32 i = 0; i < nbPoints; i++)
	{
		if (TabPtsMovable[i])
			_Mesh.MovePoint(i, TabPts[i]);
	}
}

/**************************************************************************/

void HMapMeshInfos3D::RemoveBadQuads(Playspace_Mesh	&_Mesh)
{
	S32 NbFaces = _Mesh.m_TabQuad.GetSize();
	for (S32 i = 0; i<NbFaces; i++)
	{
		Playspace_Mesh::Face *CurFace = &_Mesh.m_TabQuad[i];

		if (CurFace->IsTri)
			continue;

		if ((m_ScanReport[CurFace->TabPoints[0]].ChoosedResult == 0) || (m_ScanReport[CurFace->TabPoints[1]].ChoosedResult == 0) ||
			(m_ScanReport[CurFace->TabPoints[2]].ChoosedResult == 0) || (m_ScanReport[CurFace->TabPoints[3]].ChoosedResult == 0))
			continue;

		if ((m_ScanReport[CurFace->TabPoints[0]].ChoosedResult == 1) && (m_ScanReport[CurFace->TabPoints[1]].ChoosedResult == 1) &&
			(m_ScanReport[CurFace->TabPoints[2]].ChoosedResult == 1) && (m_ScanReport[CurFace->TabPoints[3]].ChoosedResult == 1))
			continue;

		Vec3f vNormal[4];

		if (m_ScanReport[CurFace->TabPoints[0]].ChoosedResult == 1)
			vNormal[0] = m_ScanReport[CurFace->TabPoints[0]].Ray.Normal;
		else if (m_ScanReport[CurFace->TabPoints[0]].ChoosedResult == 2)
			vNormal[0] = m_ScanReport[CurFace->TabPoints[0]].Closest.Normal;

		if (m_ScanReport[CurFace->TabPoints[1]].ChoosedResult == 1)
			vNormal[1] = m_ScanReport[CurFace->TabPoints[1]].Ray.Normal;
		else if (m_ScanReport[CurFace->TabPoints[1]].ChoosedResult == 2)
			vNormal[1] = m_ScanReport[CurFace->TabPoints[1]].Closest.Normal;

		if (m_ScanReport[CurFace->TabPoints[2]].ChoosedResult == 1)
			vNormal[2] = m_ScanReport[CurFace->TabPoints[2]].Ray.Normal;
		else if (m_ScanReport[CurFace->TabPoints[2]].ChoosedResult == 2)
			vNormal[2] = m_ScanReport[CurFace->TabPoints[2]].Closest.Normal;

		if (m_ScanReport[CurFace->TabPoints[3]].ChoosedResult == 1)
			vNormal[3] = m_ScanReport[CurFace->TabPoints[3]].Ray.Normal;
		else if (m_ScanReport[CurFace->TabPoints[3]].ChoosedResult == 2)
			vNormal[3] = m_ScanReport[CurFace->TabPoints[3]].Closest.Normal;

		Vec3f point[4];

		point[0] = _Mesh.m_TabPoints[CurFace->TabPoints[0]];
		point[1] = _Mesh.m_TabPoints[CurFace->TabPoints[1]];
		point[2] = _Mesh.m_TabPoints[CurFace->TabPoints[2]];
		point[3] = _Mesh.m_TabPoints[CurFace->TabPoints[3]];

		Vec3f dir = (point[2] - point[0]) ^ (point[3] - point[1]);
		Vec3f middle = (point[0] + point[1] + point[2] + point[3]) * 0.25f;

		Float Len2 = dir.GetNorm2();
		if (Len2 < 0.00001f)
			continue;

		Float diagDist = (point[1] - point[0]) *dir;
		Float diagDist2 = diagDist * diagDist / Len2;

		Float fdot1 = vNormal[0] * vNormal[2];
		Float fdot2 = vNormal[1] * vNormal[3];

		if (diagDist2 > 0.01f * 0.01f)
		{
			if (fdot2 < fdot1)
			{
				// Add Face
				S32 idxNewFace = _Mesh.m_TabQuad.Add();
				Playspace_Mesh::Face &newFace = _Mesh.m_TabQuad[idxNewFace];
				CurFace = &_Mesh.m_TabQuad[i];
				newFace.IsTri = TRUE;
				newFace.IsVirtual = CurFace->IsVirtual;
				newFace.IsPaintMode = CurFace->IsPaintMode;
				newFace.IsSeenQuality = CurFace->IsSeenQuality;
				newFace.TabPoints[0] = CurFace->TabPoints[0];		// newFace : (p0 p2 p3)
				newFace.TabPoints[1] = CurFace->TabPoints[2];
				newFace.TabPoints[2] = CurFace->TabPoints[3];
				newFace.TabPoints[3] = CurFace->TabPoints[3];

				CurFace->TabPoints[3] = CurFace->TabPoints[2];		// curFace : (p0 p1 p2)
				CurFace->IsTri = TRUE;
			}
			else //if (dot1 < dot2)
			{
				// Add Face
				S32 idxNewFace = _Mesh.m_TabQuad.Add();
				Playspace_Mesh::Face &newFace = _Mesh.m_TabQuad[idxNewFace];
				CurFace = &_Mesh.m_TabQuad[i];
				newFace.IsTri = TRUE;
				newFace.IsVirtual = CurFace->IsVirtual;
				newFace.IsPaintMode = CurFace->IsPaintMode;
				newFace.IsSeenQuality = CurFace->IsSeenQuality;
				newFace.TabPoints[0] = CurFace->TabPoints[1];		// newFace : (p1 p2 p3)
				newFace.TabPoints[1] = CurFace->TabPoints[2];
				newFace.TabPoints[2] = CurFace->TabPoints[3];
				newFace.TabPoints[3] = CurFace->TabPoints[3];

				CurFace->TabPoints[2] = CurFace->TabPoints[3];		// curFace : (p0 p1 p3)
				CurFace->IsTri = TRUE;
			}
		}
		else		// (diagDist2 < 0.0001f)
		{
			Vec3f cross0 = (point[3] - point[0]) ^ (point[1] - point[0]);		// (p0 p3) * (p0 p1)
			Vec3f cross1 = (point[0] - point[1]) ^ (point[2] - point[1]);		// (p1 p0) * (p1 p2)
			Vec3f cross2 = (point[1] - point[2]) ^ (point[3] - point[2]);		// (p2 p1) * (p2 p3)
			Vec3f cross3 = (point[2] - point[3]) ^ (point[0] - point[3]);		// (p3 p2) * (p3 p0)

			Float dot0 = cross0 * dir;
			Float dot1 = cross1 * dir;
			Float dot2 = cross2 * dir;
			Float dot3 = cross3 * dir;

			if (((dot0 < 0.0f) && (dot1 > 0.0f) && (dot2 > 0.0f) && (dot3 > 0.0f)) || ((dot0 > 0.0f) && (dot1 > 0.0f) && (dot2 < 0.0f) && (dot3 > 0.0f)) ||
				((dot0 > 0.0f) && (dot1 < 0.0f) && (dot2 < 0.0f) && (dot3 < 0.0f)) || ((dot0 < 0.0f) && (dot1 < 0.0f) && (dot2 > 0.0f) && (dot3 < 0.0f)))
			{
				// Add Face
				S32 idxNewFace = _Mesh.m_TabQuad.Add();
				Playspace_Mesh::Face &newFace = _Mesh.m_TabQuad[idxNewFace];
				CurFace = &_Mesh.m_TabQuad[i];
				newFace.IsTri = TRUE;
				newFace.IsVirtual = CurFace->IsVirtual;
				newFace.IsPaintMode = CurFace->IsPaintMode;
				newFace.IsSeenQuality = CurFace->IsSeenQuality;
				newFace.TabPoints[0] = CurFace->TabPoints[0];		// newFace : (p0 p2 p3)
				newFace.TabPoints[1] = CurFace->TabPoints[2];
				newFace.TabPoints[2] = CurFace->TabPoints[3];
				newFace.TabPoints[3] = CurFace->TabPoints[3];

				CurFace->TabPoints[3] = CurFace->TabPoints[2];		// curFace : (p0 p1 p2)
				CurFace->IsTri = TRUE;
			}
			else if (((dot0 > 0.0f) && (dot1 < 0.0f) && (dot2 > 0.0f) && (dot3 > 0.0f)) || ((dot0 > 0.0f) && (dot1 > 0.0f) && (dot2 > 0.0f) && (dot3 < 0.0f)) ||
				((dot0 < 0.0f) && (dot1 > 0.0f) && (dot2 < 0.0f) && (dot3 < 0.0f)) || ((dot0 < 0.0f) && (dot1 < 0.0f) && (dot2 < 0.0f) && (dot3 > 0.0f)))
			{
				// Add Face
				S32 idxNewFace = _Mesh.m_TabQuad.Add();
				Playspace_Mesh::Face &newFace = _Mesh.m_TabQuad[idxNewFace];
				CurFace = &_Mesh.m_TabQuad[i];
				newFace.IsTri = TRUE;
				newFace.IsVirtual = CurFace->IsVirtual;
				newFace.IsPaintMode = CurFace->IsPaintMode;
				newFace.IsSeenQuality = CurFace->IsSeenQuality;
				newFace.TabPoints[0] = CurFace->TabPoints[1];		// newFace : (p1 p2 p3)
				newFace.TabPoints[1] = CurFace->TabPoints[2];
				newFace.TabPoints[2] = CurFace->TabPoints[3];
				newFace.TabPoints[3] = CurFace->TabPoints[3];

				CurFace->TabPoints[2] = CurFace->TabPoints[3];		// curFace : (p0 p1 p3)
				CurFace->IsTri = TRUE;
			}
		}
	}
	_Mesh.InvalidatePointsLinks();
	return;
}

/**************************************************************************/

Bool	HMapMeshInfos3D::DoScan(Playspace_Area &_Area, Float _YGround, Float _YCeiling,Scan_Mode _Mode,Playspace_SR_W *_pSurfaceSR,Playspace_Mesh *_pSRMesh,Playspace_Mesh &_ResultMesh)
{
	// Start Point : Eye !
	Segment_Z	SegView;
	Util_L::GetViewSegment(&SegView);

	Vec3f	EyePos = SegView.Org;
	Vec3f	EyeDir = SegView.Dir;
	Float	ConeAngle = 30.f;
	Float	ConeDist = 3.f;

	switch (_Mode)
	{
		case HMapMeshInfos3D::ScanModeComplete:
			// Reinit.
			Flush();
			Init(_Area,_YGround,_YCeiling);
			// Add Process and Scan.
			AddMesh(_pSRMesh,FALSE,FALSE);
			if (!ProcessBubbleAlgorithm(SegView,_pSurfaceSR))
				return FALSE;
			CreateMesh(_ResultMesh,FALSE,FALSE);
			break;
		case HMapMeshInfos3D::ScanModeIncremental:
			// Move if needed
			Move(_Area,_YGround,_YCeiling);

			// Manage Cone.
			_pSRMesh->MarkCone(EyePos,EyeDir,ConeAngle,ConeDist,TRUE,TRUE);
			ClearConeReport(EyePos,EyeDir,ConeAngle,ConeDist);
			// Add Process and Scan.
			AddMesh(_pSRMesh,TRUE,TRUE);
			if (!ProcessFromDevice(SegView,_pSurfaceSR))
				return FALSE;
			CreateMesh(_ResultMesh,TRUE,TRUE);
			break;
		case HMapMeshInfos3D::ScanModeIncrementalReInit:
			// Reinit.
			Flush();
			Init(_Area,_YGround,_YCeiling);
			// Add Process and Scan.
			AddMesh(_pSRMesh,FALSE,TRUE);
			if (!ProcessFromDevice(SegView,_pSurfaceSR))
				return FALSE;
			CreateMesh(_ResultMesh,TRUE,TRUE);
			break;
	}

	// Finalize Mesh...
	_ResultMesh.ComputePointsLinks();
	_ResultMesh.ComputeFacesToolNormal();
	_ResultMesh.ComputePointsToolNormal();	// Need normal for draw and other things...

	MESSAGE_Z("Modification %d %d - %d",m_pMapMeshDatas->m_TabRefFaces.GetSize(),m_pMapMeshDatas->m_TabFaces.GetSize(),m_pMapResultDatas->m_ObjectTab.GetSize());
#ifdef _DEBUG
	VerifyMeshDatas();
	VerifyLeak();
#endif
	return TRUE;
}

/**************************************************************************/

void	HMapMeshInfos3D::VerifyLeak()
{
	// Map Mesh Analyse.
	S32 NbFaces = m_pMapMeshDatas->m_TabFaces.GetSize();
	S32 NbRefFaces = m_pMapMeshDatas->m_TabRefFaces.GetSize();
	S32 NbZone = m_pMapMeshDatas->m_ObjectTab.GetSize();

	PlaySpace_ScanMeshFace *pFace = m_pMapMeshDatas->m_pFirstFace;
	S32 NbFaces_Free = 0;
	while (pFace)
	{
		NbFaces_Free++;
		pFace = pFace->pFNext;
	}

	PlaySpace_ScanMeshRefFace *pRefFace = m_pMapMeshDatas->m_pFirstRefFace;
	S32 NbRefFaces_Free = 0;
	while (pRefFace)
	{
		NbRefFaces_Free++;
		pRefFace = pRefFace->pNext;
	}

	QuadTree_ScanMesh::ObjectChain *pZone = m_pMapMeshDatas->m_pFirstFreeObject;
	S32 NbZone_Free = 0;
	while (pZone)
	{
		NbZone_Free++;
		pZone = pZone->pNext;
	}

	S32 NbFacesReferenced = 0;
	for (S32 i=0 ; i<NbFaces ; i++)
	{
		NbFacesReferenced += m_pMapMeshDatas->m_TabFaces[i].UserCount;
		m_pMapMeshDatas->m_TabFaces[i].Tag = 0;
	}

	S32 NbZone_Use = 0;
	S32 NbRefFaces_Use = 0;
	for (S32 i=0 ; i<m_pMapMeshDatas->m_CellBoard.GetSize() ; i++)
	{
		pZone = m_pMapMeshDatas->m_CellBoard[i].pFirst;
		while (pZone)
		{
			NbZone_Use++;
			pRefFace = pZone->pFirstRefFace;
			while (pRefFace)
			{
				NbRefFaces_Use++;
				pRefFace->pFace->Tag = 1;
				pRefFace = pRefFace->pNext;
			}
			pZone = pZone->pNext;
		}
	}

	S32 NbFacesUse = 0;
	for (S32 i=0 ; i<NbFaces ; i++)
	{
		if (m_pMapMeshDatas->m_TabFaces[i].Tag)
		{
			NbFacesUse++;
			m_pMapMeshDatas->m_TabFaces[i].Tag = 0;
		}
	}

	EXCEPTIONC_Z(NbFacesReferenced == NbRefFaces_Use,"HMapMeshInfos3D Bad ref count");
	EXCEPTIONC_Z(NbFaces == (NbFacesUse + NbFaces_Free),"HMapMeshInfos3D Leak Faces");
	EXCEPTIONC_Z(NbRefFaces == (NbRefFaces_Use + NbRefFaces_Free),"HMapMeshInfos3D Leak RefFaces");
	EXCEPTIONC_Z(NbZone == (NbZone_Use + NbZone_Free),"HMapMeshInfos3D Leak Zone");

	// Keep result Analyse.
	S32 NbResult = m_pMapResultDatas->m_ObjectTab.GetSize();
	QuadTree_KeepScan::ObjectChain *pResult = m_pMapResultDatas->m_pFirstFreeObject;
	S32 NbResult_Free = 0;
	while (pResult)
	{
		NbResult_Free++;
		pResult = pResult->pNext;
	}
	S32 NbResult_Use = 0;
	for (S32 i=0 ; i<m_pMapResultDatas->m_CellBoard.GetSize() ; i++)
	{
		pResult = m_pMapResultDatas->m_CellBoard[i].pFirst;
		while (pResult)
		{
			NbResult_Use++;
			pResult = pResult->pNext;
		}
	}
	EXCEPTIONC_Z(NbResult == (NbResult_Use + NbResult_Free),"HMapMeshInfos3D Leak Result Keep");
}

/**************************************************************************/

void	HMapMeshInfos3D::VerifyMeshDatas()
{
	if (!m_pMapMeshDatas)
		return;
	for (S32 x = 0 ; x<m_NbCellX ; x++)
	{
		for (S32 z = 0 ; z<m_NbCellZ ; z++)
		{
			QuadTree_ScanMesh::Cell *pCell = m_pMapMeshDatas->m_CellBoard.GetArrayPtr() + (z*m_NbCellX) + x;
			if (!pCell->pFirst)
				continue;
			
			QuadTree_ScanMesh::ObjectChain *pZone = pCell->pFirst;

			Vec3f vMin,vMax;

			vMin.x = (x * m_SizeCell) + m_PosMin.x - 0.01f;
			vMin.z = (z * m_SizeCell) + m_PosMin.z - 0.01f;

			vMax.x = vMin.x + m_SizeCell + 0.02f;
			vMax.z = vMin.z + m_SizeCell + 0.02f;

			while (pZone)
			{
				if (pZone->pFirstRefFace)
				{
					vMin.y = (pZone->hMin * m_SizeCell) + m_PosMin.y - 0.01f;
					vMax.y = vMin.y + m_SizeCell + 0.02f;

					PlaySpace_ScanMeshRefFace *pRefFace = pZone->pFirstRefFace;
					while (pRefFace)
					{
						Vec3f vMin2 = pRefFace->pFace->p0;
						Vec3f vMax2 = pRefFace->pFace->p0;

						vMin2 = Min(vMin2,pRefFace->pFace->p1);
						vMin2 = Min(vMin2,pRefFace->pFace->p2);

						vMax2 = Max(vMax2,pRefFace->pFace->p1);
						vMax2 = Max(vMax2,pRefFace->pFace->p2);
						Bool IsOk = TRUE;
						if (vMax2.x <= vMin.x)
							IsOk = FALSE;
						if (vMax2.y <= vMin.y)
							IsOk = FALSE;
						if (vMax2.z <= vMin.z)
							IsOk = FALSE;

						if (vMin2.x >= vMax.x)
							IsOk = FALSE;
						if (vMin2.y >= vMax.y)
							IsOk = FALSE;
						if (vMin2.z >= vMax.z)
							IsOk = FALSE;

						EXCEPTIONC_Z(IsOk,"LAAAAA");

						pRefFace = pRefFace->pNext;
					}
				}

				pZone = pZone->pNext;
			}
		}
	}
}

/**************************************************************************/



