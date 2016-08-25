// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef __PLAYSPACE_SCAN_V3_H__
#define __PLAYSPACE_SCAN_V3_H__

#include <PlaySpace\PlaySpace_Surfel_W.h>
#include <PlaySpace\PlaySpace_Tools_W.h>
#include <PlaySpace\PlaySpace_Mesh_W.h>
#include <PlaySpace\PlaySpace_ScanMesh_W.h>

typedef BnkDynArray_Z<PlaySpace_Surfel, 256, TRUE, TRUE>	PlaySpace_SurfelBDA;
class PlaySpace_CellInfos
{
public:
	Float				fCornerX, fCornerZ;
	PlaySpace_Surfel	*pFirst;

	PlaySpace_CellInfos()
	{
		fCornerX = fCornerZ = 0.f;
		pFirst = 0;
	}
	Bool	CheckIt(S32 _x, S32 _z);
	Bool	FindBiggestHole(Float &Down, Float &Up);
};

typedef DynArray_Z<PlaySpace_CellInfos, 256, TRUE, TRUE>	PlaySpace_CellInfosDA;

class PlaySpace_SurfelBoard
{
public:
	S32							m_SizeX;
	S32							m_SizeY;
	S32							m_SizeH;

	Vec3f						m_Origin;
	Float						m_SizeCell;

	PlaySpace_Surfel			*m_pFirstFreeSurfel;
	PlaySpace_SurfelBDA			m_SurfelTab;
	PlaySpace_CellInfosDA		m_CellBoard;

	PlaySpace_SurfelBoard() {};
	~PlaySpace_SurfelBoard() {};

	void	Swap(PlaySpace_SurfelBoard &_Surfels);

	void	Empty();
	void	Flush(Bool _KeepMemory);
	Bool	IsEmpty() {return (m_SurfelTab.GetSize() == 0);}
	void	Init(Playspace_Area &_Area) {Init(_Area.Min,_Area.SizeVoxel,_Area.NbCellX,_Area.NbCellY,_Area.NbCellH);}
	void	Init(Vec3f _Origin,Float _CellSize,S32 _SizeX, S32 _SizeY, S32 _SizeH);

	S32		GetNbZoneId();
	S32		SearchHorizontalLimit(Float &_Height, S32 &_Nb, Bool _Up);		// Return Group Id

	void	ComputeConexity();
	void	ComputeConexityOld(Bool _FilterZonePlane);

	// filter forbidden surfel
	void	FilterSurfel(Float _EyePos,Float _hGround,Float _hCeilling);
	void	FilterUpSurfel(PlaySpace_CellInfos * _pCell, PlaySpace_Surfel * _pSurfel, PlaySpace_Surfel * _pLocalCeillingSurfel, Float _fEyeLimit);
	void	FilterDownSurfel(PlaySpace_CellInfos * _pCell, PlaySpace_Surfel * _pSurfel);
	void	FilterSideSurfel(S32 _x, S32 _z, PlaySpace_Surfel * _pSurfel, Float _fEyeLimit,Float _hGround,Float _hCeilling);
	void	DoFilterSideSurfel(PlaySpace_Surfel * _pSurfel, Float _fEyeLimit, S32 _startIdx, S32 _nbCell, S8 _reverseDir, S32 _delta);
	void	DoFilterSideUndergroundSurfel(PlaySpace_Surfel * _pSurfel, S32 _startIdx, S32 _nbCell, S8 _reverseDir, S32 _delta);
	void	DoFilterSideProximitySurfel(PlaySpace_Surfel * _pSurfel, S32 _startIdx, S32 _nbCell, S32 _delta,Float _hGround,Float _hCeilling);

	Bool	CanGoDown(PlaySpace_Surfel * _pSurfel, Vec3i _Dir, S32& _iZoneId);
	void	BasinFilter(S32 _groundZoneId);

	void	CreateSurfelsFromMesh(Playspace_Mesh &_Mesh);
	void	CreateSurfelsFromMesh_fast(Playspace_Mesh &_Mesh);

	S32		ComputePolyScissorByPlane(S32 nbPoints, Vec3f* points, Vec3f& planeNormal, Vec3f& planePoint, Vec3f* output, S32 maxSizeOutput);
	Float	ComputeTriAreaInAABox(S32 nbPts, Vec3f* p, Vec3f& center, Float demiSize);
	U8		ResolveFaceInCell(Playspace_Mesh &_Mesh,S32 face, Vec3f& cellCenter, Bool& IsOkToAdd, Bool& bNoGamePlay);


	FINLINE_Z PlaySpace_Surfel *NewSurfel()
	{
		if (m_pFirstFreeSurfel)
		{
			PlaySpace_Surfel *pNewSurfel = m_pFirstFreeSurfel;
			m_pFirstFreeSurfel = m_pFirstFreeSurfel->pNext;
			pNewSurfel->Init();
			return pNewSurfel;
		}
		PlaySpace_Surfel *pNewSurfel = &m_SurfelTab.Add();
		pNewSurfel->Init();
		return pNewSurfel;
	}

	FINLINE_Z void DeleteSurfel(PlaySpace_Surfel *_pSurfel)
	{
		_pSurfel->pNext = m_pFirstFreeSurfel;
		m_pFirstFreeSurfel = _pSurfel;
	}

	FINLINE_Z void	SortAddSurfel(PlaySpace_CellInfos *_pCell, PlaySpace_Surfel *_pSurfel)
	{
		Float y = _pSurfel->Point.y;
		PlaySpace_Surfel *pPrev = _pCell->pFirst;
		if (!pPrev || (y <pPrev->Point.y))
		{
			_pSurfel->pNext = pPrev;
			_pCell->pFirst = _pSurfel;
			return;
		}
		PlaySpace_Surfel *pCur = pPrev->pNext;
		while (pCur && (y >pCur->Point.y))
		{
			pPrev = pCur;
			pCur = pCur->pNext;
		}

		pPrev->pNext = _pSurfel;
		_pSurfel->pNext = pCur;
	}

	FINLINE_Z PlaySpace_CellInfos	*GetCell(S32 x, S32 z)
	{
		EXCEPTIONC_Z(x >= 0, "PlaySpace_SurfaceInfosV3::Get Out of playspace");
		EXCEPTIONC_Z(z >= 0, "PlaySpace_SurfaceInfosV3::Get Out of playspace");
		EXCEPTIONC_Z(x < m_SizeX, "PlaySpace_SurfaceInfosV3::Get Out of playspace");
		EXCEPTIONC_Z(z < m_SizeY, "PlaySpace_SurfaceInfosV3::Get Out of playspace");
		return &(m_CellBoard[z * m_SizeX + x]);
	}

	FINLINE_Z PlaySpace_Surfel		*GetSurfel(S32 x, S32 y, S32 z)
	{
		if (x < 0) return NULL;
		if (z < 0) return NULL;
		if (x >= m_SizeX) return NULL;
		if (z >= m_SizeY) return NULL;
		PlaySpace_Surfel	*pCur = m_CellBoard[z * m_SizeX + x].pFirst;
		while (pCur)
		{
			if (y == pCur->y)
				return pCur;
			pCur = pCur->pNext;
		}
		return NULL;
	}
	FINLINE_Z PlaySpace_Surfel		*GetSurfel(PlaySpace_CellInfos *_pCell, S32 _y)
	{
		PlaySpace_Surfel	*pCur = _pCell->pFirst;
		while (pCur)
		{
			if (_y == pCur->y)
				return pCur;
			pCur = pCur->pNext;
		}
		return NULL;
	}
	FINLINE_Z PlaySpace_Surfel		*GetSurfel(PlaySpace_CellInfos *_pCell, S32 _y, S32 _iDir)
	{
		PlaySpace_Surfel	*pCur = _pCell->pFirst;
		while (pCur)
		{
			if ((_y == pCur->y) && (_iDir == pCur->iDir))
				return pCur;
			pCur = pCur->pNext;
		}
		return NULL;
	}
	FINLINE_Z void		TryAddSurfel(Vec3i &_iPos, S32 _iDir, Vec3f &_Pos, Vec3f &_Normal, Bool _IsVirtual, Bool _NoGameplay = FALSE,Bool _BadSurfel = FALSE, Bool _IsExternal = FALSE)
	{
		PlaySpace_CellInfos	*pCell = GetCell(_iPos.x, _iPos.z);
		PlaySpace_Surfel	*pSurfel = GetSurfel(pCell, _iPos.y, _iDir);
		if (pSurfel)
		{
			if (_NoGameplay)	// On ne remplace pas par une nogameplay
				return;
			if (!pSurfel->NoGameplay() && _IsVirtual) // On ne remplace pas une virtuel sauf si la courrante est nogameplay
				return;

			// Remove !	(Because not necessary sorted).
			RemoveSurfel(pCell, pSurfel);
		}

		// Create and Init.
		pSurfel = NewSurfel();

		pSurfel->x = _iPos.x;
		pSurfel->y = _iPos.y;
		pSurfel->z = _iPos.z;

		pSurfel->Normal = _Normal;
		pSurfel->Point = _Pos;

		pSurfel->Quality = 255;
		pSurfel->ZoneId = -1;
		pSurfel->iDir = _iDir;

		if (_IsVirtual)
			pSurfel->Flags |= PlaySpace_Surfel::SURFFLAG_VIRTUAL;
		if (_NoGameplay)
			pSurfel->Flags |= PlaySpace_Surfel::SURFFLAG_NOGAMEPLAY;
		if (_BadSurfel)
			pSurfel->Flags |= PlaySpace_Surfel::SURFFLAG_NOGAMEPLAY | PlaySpace_Surfel::SURFFLAG_BADSURFEL;
		if(_IsExternal)
			pSurfel->Flags |= PlaySpace_Surfel::SURFFLAG_EXTERNAL;

		// Don't Exist => Create.
		SortAddSurfel(pCell, pSurfel);
	}

	FINLINE_Z	void	FlushCell(PlaySpace_CellInfos *_pCell)
	{
		PlaySpace_Surfel	*pCur = _pCell->pFirst;
		if (pCur)
		{
			while (pCur->pNext)
				pCur = pCur->pNext;

			pCur->pNext = m_pFirstFreeSurfel;
			m_pFirstFreeSurfel = _pCell->pFirst;

			_pCell->pFirst = NULL;
		}
	}

	FINLINE_Z	void	RemoveSurfel(PlaySpace_CellInfos *_pCell, PlaySpace_Surfel *_pSurfel, Bool _DeleteSurfel = TRUE)
	{
		if (_pCell->pFirst == _pSurfel)
			_pCell->pFirst = _pSurfel->pNext;
		else
		{
			PlaySpace_Surfel	*pCur = _pCell->pFirst;
			while (pCur->pNext != _pSurfel)
				pCur = pCur->pNext;
			EXCEPTIONC_Z(pCur != 0, "PlaySpace_SurfaceInfosV3::RemoveSurfel Not found...");
			pCur->pNext = _pSurfel->pNext;
		}

		// Set As Free.
		if (_DeleteSurfel)
			DeleteSurfel(_pSurfel);
	}

	S32		GetSurfelByIDistAndIDir(S32 _x, S32 _y, S32 _z, S32 _NbCell, S32 _iDir, PlaySpace_Surfel **_TabResult, S32 _NbMax);
	S32		GetSurfelByIDistAndDirWall(S32 _x, S32 _y, S32 _z, S32 _NbCell, Vec3f& _norm, Float _cosMax, Float _cosMaxTolerance, PlaySpace_Surfel **_TabResult, S32 _NbMax);
};

class PlaySpace_SurfaceInfosV3
{
public:
	U32							m_MeshCycleID;
	Playspace_Mesh				m_Mesh;
	Playspace_Mesh				m_SimplifiedMesh;
	
	PlaySpace_SurfelBoard		m_Surfels;

	Bool						m_IsInRayMode;
	HMapMeshInfos3D				m_MapMesh3D;	// This the Mesh Version of V3

	PlaySpace_SurfaceInfosV3();
	~PlaySpace_SurfaceInfosV3();

	void	ReInitScan(Playspace_Area &_Area, Float _YGround, Float _YCeiling);
	void	ReInitScan(Vec3f &_Min, Vec3f &_Max, Float _YGround, Float _YCeiling, Float _CellSize, S32 _SizeX, S32 _SizeH, S32 _SizeY);
	void	CreateMesh(Playspace_Mesh &_Mesh,Bool _FastMode=FALSE,Bool _KeepMode=FALSE);
};

#endif //__PLAYSPACE_SCAN_V3_H__
