// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef __PLAYSPACE_SCAN_MESH_H__
#define __PLAYSPACE_SCAN_MESH_H__

#include <PlaySpace\PlaySpace_Surfel_W.h>
#include <PlaySpace\PlaySpace_Tools_W.h>
#include <PlaySpace\PlaySpace_ScanBase_W.h>
#include <PlaySpace\Playspace_SR_W.h>

class PlaySpace_Vec3i
{
public:
	S32	x : 10, y : 10, z : 10;
};

class QuadTree_ScanMesh;

class BlindDetector
{
public:
	HU8DA		TabBlindZone;
	S32			m_SizeX;
	S32			m_SizeY;
	S32			m_SizeZ;
	BlindDetector();
	
	void Init(S32 _SizeX, S32 _SizeY, S32 _SizeZ);
	void Flush();
	FINLINE_Z	U8 *GetCell(S32 _X, S32 _Y, S32 _Z)
	{
		return &TabBlindZone[((_Z * m_SizeX + _X) * m_SizeY) + _Y];
	}
	Bool ComputeBlindZone(QuadTree_ScanMesh *_pMapDatas, Vec3i &_StartPos);
	Bool ComputeBlindZoneFast(QuadTree_ScanMesh *_pMapDatas, Vec3i &_StartPos);

	void Draw(Vec3f &_PosMin, Float _SizeCell);
};

class PlaySpace_ScanMeshFace
{
public:
static const U16 MaxUserCount = 0xFFF0;
	Vec3f	p0,p1,p2;
	Vec3f	Normal;
	U16		UserCount;
	U16		Tag;
	U8		IsPaintMode:2,IsSeenQuality:2,IsFlat:1;
	PlaySpace_ScanMeshFace	*pFNext;
};

class PlaySpace_ScanMeshRefFace
{
public:
	PlaySpace_ScanMeshFace		*pFace;
	PlaySpace_ScanMeshRefFace	*pNext;
};

class PlaySpace_HVoxelZone
{
public:
	U8			IsPaintMode:2,IsSeenQuality:2,IsKeepBlock:1;
	S16			hMin;
	S16			hMax;
	PlaySpace_ScanMeshRefFace	*pFirstRefFace;
};

class QuadTree_ScanMesh : public PlaySpace_CellBoard<PlaySpace_HVoxelZone>
{
public:
	S32		m_hGround;
	S32		m_hCeiling;
	S32		m_hMin;
	S32		m_hMax;

	U16		m_CurFaceTag;
	FINLINE_Z void			InitFaceTag()
	{
		// Reset.
		m_CurFaceTag = 1;
		S32 Size = m_TabFaces.GetSize();
		for (S32 i=0 ; i<Size ; i++)
			m_TabFaces[i].Tag = 0;
	}
	FINLINE_Z void			NewFaceTag()
	{
		m_CurFaceTag++;
		if (!m_CurFaceTag)
			InitFaceTag();
	}
	PlaySpace_ScanMeshRefFace									*m_pFirstRefFace;
	BnkDynArray_Z<PlaySpace_ScanMeshRefFace, 8192, TRUE, TRUE>	m_TabRefFaces;

	PlaySpace_ScanMeshFace										*m_pFirstFace;
	BnkDynArray_Z<PlaySpace_ScanMeshFace, 8192, TRUE, TRUE>		m_TabFaces;

	QuadTree_ScanMesh(S32 _SizeX, S32 _SizeY) : PlaySpace_CellBoard(_SizeX, _SizeY) { m_pFirstRefFace = NULL;m_pFirstFace = NULL;};

	FINLINE_Z PlaySpace_HVoxelZone	*GetZone(S32 _x, S32 _y, S32 _z,Bool _CreateIfNeeded)
	{
		Cell	*pCell = GetCell(_x, _z);

		QuadTree_ScanMesh::ObjectChain *pZone = pCell->pFirst;
		while (pZone)
		{
			if ((_y >= pZone->hMin) && (_y <= pZone->hMax))
				return pZone;
			pZone = pZone->pNext;
		}
		if (_CreateIfNeeded)
		{
			pZone = New();
			pZone->IsPaintMode = 0;
			pZone->IsSeenQuality = 0;
			pZone->IsKeepBlock = 0;
			pZone->hMin = _y;
			pZone->hMax = _y;
			pZone->pFirstRefFace = NULL;
			pZone->pNext = NULL;
			Add(_x, _z, pZone);
		}
		return pZone;
	}
	FINLINE_Z void DeleteZone(QuadTree_ScanMesh::ObjectChain *_pZone)
	{
		// Delete Refs
		if (_pZone->pFirstRefFace)
		{
			PlaySpace_ScanMeshRefFace *pRefFace = _pZone->pFirstRefFace;
			PlaySpace_ScanMeshRefFace *pCurHead = m_pFirstRefFace;
			m_pFirstRefFace = pRefFace;
			for (;;)
			{
				PlaySpace_ScanMeshFace *pFace = pRefFace->pFace;
				pFace->UserCount--;
				if (!pFace->UserCount)
				{
					// Kick Face.
					pFace->pFNext = m_pFirstFace;
					m_pFirstFace = pFace;
				}
				if (pRefFace->pNext)
				{
					// Continue..
					pRefFace = pRefFace->pNext;
					continue;
				}
				pRefFace->pNext = pCurHead;
				break;
			}
		}
		// Delete Zone.
		Delete(_pZone);
	}

	FINLINE_Z PlaySpace_ScanMeshRefFace *NewRefFace()
	{
		if (m_pFirstRefFace)
		{
			PlaySpace_ScanMeshRefFace *pNewRefFace = m_pFirstRefFace;
			m_pFirstRefFace = m_pFirstRefFace->pNext;
			pNewRefFace->pNext = NULL;
			return pNewRefFace;
		}
		PlaySpace_ScanMeshRefFace *pNewRefFace = &m_TabRefFaces.Add();
		pNewRefFace->pNext = NULL;
		return pNewRefFace;
	}


	FINLINE_Z PlaySpace_ScanMeshFace *NewFace()
	{
		if (m_pFirstFace)
		{
			PlaySpace_ScanMeshFace *pNewFace = m_pFirstFace;
			m_pFirstFace = m_pFirstFace->pFNext;
			pNewFace->pFNext = NULL;
			return pNewFace;
		}
		PlaySpace_ScanMeshFace *pNewFace = &m_TabFaces.Add();
		pNewFace->pFNext = NULL;
		return pNewFace;
	}

	FINLINE_Z void	AddFace(Vec3f &_p0,Vec3f &_p1,Vec3f &_p2,Vec3f &_Normal,Playspace_Mesh::Face &_Face,Bool _IsFlat,PlaySpace_HVoxelZone *_pZone,PlaySpace_ScanMeshFace *&_pCreatedFace)
	{
		// Create Face.
		if (!_pCreatedFace || (_pCreatedFace->UserCount >= PlaySpace_ScanMeshFace::MaxUserCount))
		{
			_pCreatedFace = NewFace();
			_pCreatedFace->p0 = _p0;
			_pCreatedFace->p1 = _p1;
			_pCreatedFace->p2 = _p2;
			_pCreatedFace->Normal = _Normal;

			_pCreatedFace->UserCount = 0;
			_pCreatedFace->Tag = 0;
			_pCreatedFace->IsPaintMode = _Face.IsPaintMode;
			_pCreatedFace->IsSeenQuality = _Face.IsSeenQuality;
			_pCreatedFace->IsFlat = _IsFlat;
		}
			
		// Add Ref.
		PlaySpace_ScanMeshRefFace *pRefFace = NewRefFace();
		_pCreatedFace->UserCount++;
		pRefFace->pFace = _pCreatedFace;
		pRefFace->pNext = _pZone->pFirstRefFace;

		// Update Zone
		_pZone->IsPaintMode = Max(_pZone->IsPaintMode,_Face.IsPaintMode);
		_pZone->IsSeenQuality = Max(_pZone->IsSeenQuality,_Face.IsSeenQuality);
		_pZone->pFirstRefFace = pRefFace;
	}
};

class PlaySpace_KeepScanInfos
{
public:
	Vec3f	CubicPos;
	Vec3f	ResultPos;
	S16		hPos;
	Bool	bIsUsed;
	Bool	bIsNew;
};

class QuadTree_KeepScan : public PlaySpace_CellBoard<PlaySpace_KeepScanInfos>
{
public:
	QuadTree_KeepScan(S32 _SizeX, S32 _SizeY) : PlaySpace_CellBoard(_SizeX, _SizeY) {;};

	FINLINE_Z PlaySpace_KeepScanInfos	*GetCubicPoint(Vec3i &_iPos,Vec3f &_CubicPos,Float _DeltaMax2,Bool _CreateIfNeeded)
	{
		Cell	*pCell = GetCell(_iPos.x, _iPos.z);

		QuadTree_KeepScan::ObjectChain *pZone = pCell->pFirst;
		while (pZone)
		{
			if (_iPos.y == pZone->hPos)
			{
				if ((pZone->CubicPos - _CubicPos).GetNorm2() < _DeltaMax2)
					return pZone;
			}
			pZone = pZone->pNext;
		}
		if (_CreateIfNeeded)
		{
			pZone = New();
			pZone->bIsUsed = FALSE;
			pZone->bIsNew = TRUE;
			pZone->hPos = _iPos.y;
			pZone->CubicPos = _CubicPos;
			pZone->ResultPos = VEC3F_NULL;
			pZone->pNext = NULL;
			Add(_iPos.x, _iPos.z, pZone);
		}
		return pZone;
	}
};

class PlaySpace_OneReport
{
public:
	PlaySpace_ScanMeshFace	*pFace;
	Float	Score;
	Float	DistInter;
	Vec3f	Inter;
	Vec3f	Normal;
};

class PlaySpace_ScanReport
{
public:
	PlaySpace_OneReport	Ray;
	PlaySpace_OneReport	Closest;
	Vec3f				OriginalPos;
	Vec3f				RayDir;
	Vec3f				Multi_SumNormal;
	Float				Multi_Error;
	PlaySpace_KeepScanInfos	*pKeepScan;
	U8					ChoosedResult;		// 0 = nothing - 1 = Ray - 2 = Closest.
	Bool				IsPointFlat;

	void	ClearResult()
	{
		Multi_Error = 1.f;
		Ray.pFace = Closest.pFace = NULL;
		Closest.DistInter = Ray.DistInter = 1e20f;
		Closest.Score = Ray.Score = 1e20f;
	}
};

typedef DynArray_Z<PlaySpace_ScanReport, 256, FALSE, FALSE>	PlaySpace_ScanReportDA;

class HMapMeshInfos3D : public HMapBaseInfos3D
{
protected:
	PlaySpace_ScanReportDA	m_ScanReport;
	BlindDetector			m_BlindDetector;
	S32						m_LastDeviceVolume;

	HU8DA					m_infos2D;			// test JY

	Bool					IsEmptyZone(S32 _x, S32 _y, S32 _z);
	PlaySpace_HVoxelZone	*ConstructEmptyZone(S32 _x, S32 _y, S32 _z);

	FINLINE_Z	Bool	IsReportFlat(PlaySpace_ScanReport &_Report, Vec3f *_pRefNormal,Float _CosFlat)
	{
		if (_Report.Multi_Error < _CosFlat)
			return FALSE;

		if (_Report.Ray.pFace)
		{
			// Have a Ray.
			if (_Report.Closest.pFace)
			{
				if ((_Report.Ray.Normal * _Report.Closest.Normal) < _CosFlat)
					return FALSE;
				else if (_pRefNormal)
				{
					if ((*_pRefNormal * _Report.Closest.Normal) < _CosFlat)
						return FALSE;
					if ((_Report.Ray.Normal * *_pRefNormal) < _CosFlat)
						return FALSE;
				}
			}

			// Is It Flat.
			if (!_Report.Ray.pFace->IsFlat)
				return FALSE;
		}
		if (_Report.Closest.pFace)
		{
			// Is It Flat.
			if (!_Report.Closest.pFace->IsFlat)
				return FALSE;
		}

		return TRUE;
	}

	FINLINE_Z	void	ScanFaceClosest(Vec3f &_Pos, Vec3f &_Dir, PlaySpace_ScanReport &_Report, Float _DistMax);
	FINLINE_Z	Bool	ScanZoneFaceClosest(Vec3f &_Pos, Vec3f &_Dir, QuadTree_ScanMesh::Cell *_pCell, S32 _y, PlaySpace_ScanReport &_Report);
	FINLINE_Z	void	ScanFaceRay(Vec3f &_Pos, Vec3f &_Dir, PlaySpace_ScanReport &_Report,Float _DistMax);
	FINLINE_Z	Bool	ScanZoneFaceRay(Vec3f &_Pos, Vec3f &_Dir, QuadTree_ScanMesh::Cell *_pCell, S32 _y, PlaySpace_ScanReport &_Report);

	FINLINE_Z	void	ProcessZone(const Vec3i &_Pos, S32 _hmin, S32 _hmax, DynArray_Z<Vec3i, 256, FALSE, FALSE>	&_ProcessList);
				S32		ProcessFlood(const Vec3i &_StartPos);

	FINLINE_Z	void	GetMinMax(Vec3i &_Pos, const Vec3f &_vDir, S32 _Axis, Float &_Min, Float &_Max);
	FINLINE_Z	void	SnapMinMax(Playspace_Mesh &_Mesh, SafeArray_Z<S32, 8, FALSE, FALSE> &_TabFace, S32 _NbFaces, Vec3f &_vPos, const Vec3f &_vDir, S32 _Axis, S32 _IsPositivNormal);

	void	PrepareScanReport(Playspace_Mesh &_Mesh,Bool _KeppMode);
	void	ClearConeReport(Vec3f &_Pos,Vec3f &_Dir,Float _HalfAngleDeg,Float _DistMax);

	void	CreateCubicEnvelop(Playspace_Mesh &_Mesh,Bool _DontAddVirtual);
	void	PushPerpendicular(Playspace_Mesh &_Mesh);
	void	PushFromFaces(Playspace_Mesh &_Mesh);

	void	PlanarVirtualZones(Playspace_Mesh	&_Mesh);
	void	VerifyLeak();
	void	VerifyMeshDatas();

public:
typedef enum {
	ScanModeComplete = 0x0,
	ScanModeIncremental = 0x1,
	ScanModeIncrementalReInit = 0x2,
} Scan_Mode;

	QuadTree_ScanMesh	*m_pMapMeshDatas;
	QuadTree_KeepScan	*m_pMapResultDatas;

	HMapMeshInfos3D();
	virtual ~HMapMeshInfos3D();
	void	Flush();
	virtual void	Init(Playspace_Area &_Area,Float _YGround,Float _YCeiling);
	virtual void	Move(Playspace_Area &_Area,Float _YGround,Float _YCeiling);

	Bool	DoScan(Playspace_Area &_Area, Float _YGround, Float _YCeiling,Scan_Mode _Mode,Playspace_SR_W *_pSurfaceSR,Playspace_Mesh *_pSRMesh,Playspace_Mesh &_ResultMesh);

	void	AddMesh(Playspace_Mesh *_pMesh,Bool _FastMode,Bool _KeepMode = FALSE);

	Bool	ProcessOnePass(Segment_Z &_ViewSeg,Bool _FastMode);
	Bool	ProcessFromDevice(Segment_Z &_ViewSeg,Playspace_SR_W *_pPlayspaceSR);

	Bool	ProcessBubbleAlgorithm(Segment_Z &_ViewSeg, Playspace_SR_W *_pPlayspaceSR, Bool onlySeen = TRUE);

	void	ComputeWallInfos(HU32DA& wallInfos);
	void	ProcessConcaveHull2D(HU8DA& tabInfos2D, HU16DA& flagBuffer, Bool onlySeen);

	void	CreateMesh(Playspace_Mesh &_Mesh,Bool _FastMode,Bool _KeepMode);

	void	RemoveBadQuads(Playspace_Mesh	&_Mesh);
};

#endif //__PLAYSPACE_SCAN_H__
