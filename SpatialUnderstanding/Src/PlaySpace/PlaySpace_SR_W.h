// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef __PLAYSPACE_SR_H__
#define __PLAYSPACE_SR_H__

#include <PlaySpace\PlaySpace_Mesh_W.h>
#include <Dll_Interface.h>

#define	SR_BLINDRAY_MAP_SIZE	128		// 8 cm precision on 10 m dist.
#define	SR_BLINDRAY_MAP_MASK	0x7F	// 8 cm precision on 10 m dist.
#define	SR_BLINDRAY_MAP_SHIFT	7
#define	SR_BLINDRAY_MAP_TO_CELL		12.5f	// 1 / 8 cm 
#define	SR_BLINDRAY_MAP_TO_WORLD	0.08f	// 8 cm 
#define	SR_BLINDRAY_MAP_VOXEL_DIAMETER (0.08f * 1.732f)

#define SR_BLINDRAY_ZBUFF_SIZE	128
#define SR_BLINDRAY_ZBUFF_MASK	127
#define SR_BLINDRAY_ZBUFF_SHIFT	7
#define SR_BLINDRAY_ZBUFF_FHSIZE 64.f
#define SR_BLINDRAY_ZBUFF_HMASK	63

#define	SR_CONE_TAB_SIZE		32
#define	SR_CONE_TAB_MSK			(SR_CONE_TAB_SIZE-1)

class PSR_BlindMap_Voxel		//GetPointInfos , GetPointNormalInfos , ApplyQuatAndMinMax , 
{
public:
	enum Flag {
		None = 0x0,
		IsBlock = 0x1,		// Have faces.
		IsSeen = 0x2,		// Have been seen.
		IsInCone = 0x4,		// Have been in cone.
		IsOut = 0x8,		// Unusable.
		IsPaint = 0x10,		// Paint by user.
		IsUnPaint = 0x20,	// UnPaint/Clear by user.
		IsWork = 0x80,		// System Flag.	=> Have to be TOP BIT
	};

	U8		Flags;
	U8		SeenCount;

	void Clear() {Flags=0; SeenCount=0;}
};

typedef HugeDynArray_Z<PSR_BlindMap_Voxel,256,FALSE,FALSE>		PSR_BlindMap_VoxelHUDA;

class Playspace_SR_ConeView
{
public:
	Vec3f	Pos;
	Vec3f	Dir;
	Float	Angle;
	Float	Dist;
	U32		Frame30FPS;
};

class Playspace_SR_BlindMap
{
public:
	Vec3f		m_fCenter;
	Vec3f		m_fOrigin;
	Vec3f		m_fOrigin_Half;

	// Work Datas for AddMesh !
	AutoStackArray_Z<Vec3f, 16384, FALSE, FALSE>	m_StackPoint;	// Set Auto Stack grow Array.
	AutoStackArray_Z<U32, 8192, FALSE, FALSE>		m_StackVertex;	// Set Auto Stack grow Array.

	PSR_BlindMap_VoxelHUDA	m_BlindMap;
	HU8DA		m_LastZBuffer;

	enum Mode {
		Mode_Blind,
		Mode_Paint,
		Mode_Clear
	};
	Playspace_SR_BlindMap();
	~Playspace_SR_BlindMap();

	Bool	IsInit() {return (m_BlindMap.GetSize() > 0);}

	void	Init(Vec3f _Center);
	void	Clear();
	void	Flush();

	S32		GetSeenZoneList(Float _x,Float _z,Float _yMin,Float _yMax,Float *_pTabDest,S32 _NbMax);

	void	AddMesh(Playspace_Mesh &_Mesh,PSR_BlindMap_Voxel::Flag _Flag = PSR_BlindMap_Voxel::IsBlock);
	void	AddMeshSlow(U32 *_pTabTriIdx, S32 _NbTriIdx, U32 _ModuloTri, Vec3f *_pTabVtx, S32 _NbVtx, PSR_BlindMap_Voxel::Flag _Flag = PSR_BlindMap_Voxel::IsBlock);
	void	AddMeshNew(U32 *_pTabTriIdx, S32 _NbTriIdx,U32 _ModuloTri, Vec3f *_pTabVtx, S32 _NbVtx, PSR_BlindMap_Voxel::Flag _Flag = PSR_BlindMap_Voxel::IsBlock); // Plus LENT !! En cours de modification car plus simple et devrait être plus rapide :(
	void	AddMesh(U32 *_pTabTriIdx, S32 _NbTriIdx,U32 _ModuloTri, Vec3f *_pTabVtx, S32 _NbVtx, PSR_BlindMap_Voxel::Flag _Flag = PSR_BlindMap_Voxel::IsBlock);
	void	AddMesh2(U32 *_pTabTriIdx, S32 _NbTriIdx, U32 _ModuloTri, Vec3f *_pTabVtx, S32 _NbVtx, PSR_BlindMap_Voxel::Flag _Flag = PSR_BlindMap_Voxel::IsBlock);
	void	AddMeshEnd();

	void	ConvertWorkFlag(PSR_BlindMap_Voxel::Flag _Flag = PSR_BlindMap_Voxel::IsBlock);
	void	CleatFlag(PSR_BlindMap_Voxel::Flag _Flag);
	void	ProcessConeView(Playspace_SR_BlindMap::Mode _Mode, const Vec3f &_Pos, const Vec3f &_Dir,const Vec3f &_LateralDir, Float _Angle, Float _MaxDist);
	void	ProcessConeViewNew(Playspace_SR_BlindMap::Mode _Mode,const Playspace_SR_ConeView &_ConeBlind,const Playspace_SR_ConeView &_ConeMode,const Vec3f &_LateralDir);

	void	ResetZBuffer(HU8DA	&_ZBuffer);
	void	AddMeshToZBuffer(HU8DA	&_ZBuffer,Playspace_Mesh &_Mesh,const Vec3f &_Pos, const Vec3f &_Dir, Float _Angle, Float _MaxDist);
	void	AddMeshToZBuffer(HU8DA	&_ZBuffer,U32 *_pTabTriIdx, S32 _NbTriIdx, U32 _ModuloTri, Vec3f *_pTabVtx, S32 _NbVtx,const Vec3f &_Pos, const Vec3f &_Dir, Float _Angle, Float _MaxDist);

	void	GetPointInfos(Vec3f *pTabPoints, S32 _NbPoints, PSR_BlindMap_VoxelHUDA &_TabVoxels);
	void	GetPointNormalInfos(Vec3f *pTabPoints, Playspace_Mesh::ToolPointNormal *pTabNormal,S32 _NbPoints,Float _fDeltaNormal, PSR_BlindMap_VoxelHUDA &_TabVoxels);
	void	RefreshMergePaintToMesh(Playspace_Mesh &_Mesh, Float _fUseDeltaNormal = -1.f);
	void	ApplyQuatAndMinMax(const Quat &_Transfo, const Vec3f &_Min, const Vec3f &_Max);
	Bool	ApplyMoveXZAndMinMax(Vec3f _NewCenter, const Vec3f &_Min, const Vec3f &_Max);
	void	LockMinMax(const Vec3f &_Min, const Vec3f &_Max);
	void	Draw();
};

typedef S32 Playspace_SR_SurfaceInfoID;	// To be define.

class Playspace_SR_BlockHdl
{
public:
	S32							m_BlocNum;
	Playspace_SR_SurfaceInfoID	m_SurfaceHdl;

	Playspace_SR_BlockHdl()
	{
		m_BlocNum = -1;
		m_SurfaceHdl = FRIG_SURFACE_HANDLE_NULL;
	}
	void Init()
	{
		m_BlocNum = -1;
		m_SurfaceHdl = FRIG_SURFACE_HANDLE_NULL;
	}
	FINLINE_Z	Bool	operator == (const Playspace_SR_BlockHdl &aHdl)	const	{ return (m_BlocNum == aHdl.m_BlocNum) && (m_SurfaceHdl == aHdl.m_SurfaceHdl); }
	FINLINE_Z	Bool	operator != (const Playspace_SR_BlockHdl &aHdl)	const	{ if (m_BlocNum < 0) return (m_SurfaceHdl != m_SurfaceHdl); else return (m_BlocNum != aHdl.m_BlocNum); }
	FINLINE_Z	Bool	IsValid() {return ((m_BlocNum >= 0) || (m_SurfaceHdl != FRIG_SURFACE_HANDLE_NULL));}
};

class Playspace_SR_Block
{
public:
	Playspace_SR_BlockHdl	m_BlockHdl;
	HugeDynArray_Z<Vec3f>	m_TabVtx;
	HU32DA					m_TabFaceIdx;

	Vec3f					m_Center;

	U32						m_FrameRefresh;
	U32						m_CRCRefresh;
	Bool					m_HaveBeenSeen;
	Bool					m_NeedBlindRefresh;
	U8						m_CounthForDelete;

	Playspace_SR_Block		*pNext;

	Playspace_SR_Block() {};
	~Playspace_SR_Block() {};
};

enum Playspace_SR_Refresh {
	REFRESH_SR_NONE,
	REFRESH_SR_DEVICE,
	REFRESH_SR_FILE,
	REFRESH_SR_SCENE
};

enum Playspace_SR_Status {
	STATUS_OK = 0,
	STATUS_DELAYED = 1,
	STATUS_FREEZE = 2
};

class Playspace_SR_SurfaceInfo
{
public:
	Playspace_SR_SurfaceInfoID	ID;	// This is a S32 be could be change for have the good format (Microsoft ONE => SEE typedef)
	S32		FrameUpdate;			// This is a Date or number that change if datas change.

	Vec3f	*pVertexPos;			// Vertex position (Block frame).
	S32		NbVertex;

	S32		NbIndex;
	S32		*pIndex;
	Bool	IndexIsClockWise;		// Face winding.

	Mat4x4	PosToWorldMtx;			// Transform Vertex to World (This is used as BLOCK PIVOT : MUST BE UNIQ FOR EACH SURFACE)
	
	Playspace_SR_SurfaceInfo() { Null(); }
	void	Null()
	{
		pVertexPos = NULL;
		pIndex = NULL;
		NbVertex = 0;
		NbIndex = 0;
		FrameUpdate = 0;
		IndexIsClockWise = FALSE;
		PosToWorldMtx.SetIdentity();
	}
};
class Playspace_SR_DeviceSR
{
public:
	Mat4x4						GlobalTransfo;
	Playspace_SR_SurfaceInfo	*TabSurfaces;
	S32							NbSurfaces;
};

class Playspace_SR_W
{
protected:
	SharedResource_Z		m_LockThreadRefresh;
	SharedResource_Z		m_LockThreadBlind;
	Playspace_SR_Refresh	m_TypeRefresh;
	U32						m_LastFrameUpdate;
	Playspace_SR_Block		*pFreeBlock;
	BnkDynArray_Z<Playspace_SR_Block, 16, TRUE, TRUE, _ALLOCDEFAULTALIGN, 256>	m_TabBlocks;

	void				ReleaseBlock(Playspace_SR_Block *_pBlock);
	Playspace_SR_Block	*GetNewBlock();
	Playspace_SR_Block	*GetBlock(Playspace_SR_BlockHdl &_BlockHdl, Bool _CreateIfNeeded = FALSE);
	Playspace_SR_Block	*GetBlockByCenterPos(Vec3f &_Pos,Float _DistMax2);
	void				FlushBlocks();
	void				FlushBlind();

	void				MarkBlindMapFromSR(Bool _FullRefresh,PSR_BlindMap_Voxel::Flag _Flag);

	// Filter Cone.
	U32			m_SpeedCheckTime30FPS;
	Vec3f		m_SpeedCheckPos;
	Vec3f		m_SpeedCheckDir;
	S32			m_IdFirstCone;
	S32			m_IdLastCone;
	Playspace_SR_ConeView	m_TabConeView[32];

	S32			m_CurCone;
	Vec3f		m_FilterConePos;
	Vec3f		m_FilterConeDir;
	Float		m_FilterConeAngle;
	Float		m_FilterConeMax;
	Float		m_FilterConeMax2;
	Float		m_FilterConeCos2;
	Float		m_FilterConeInvCos;
	Float		m_FilterConeTan;
	void		SetLagConeView(Float _LagTime);
	Bool		IsValidConeView() {return (m_CurCone >= 0);}
	void		InvalidateConeView() {m_CurCone = -1;}

	FINLINE_Z Bool		SRSphereVsCone(const Vec3f &_Center, Float R);

	// Valid Zone;
	Vec3f		m_Min;
	Vec3f		m_Max;

	static Float		m_SnapGranularity;
public:
	Playspace_SR_BlindMap	m_BlindMap;

	Playspace_SR_W();
	~Playspace_SR_W();

	Playspace_SR_Status	GetRefreshStatus();
	void	Flush(Bool _OnlyIfDevice);

	Playspace_SR_Refresh	GetTypeRefresh() {return m_TypeRefresh;}
	void	SetSnapValue(Float	_Granularity);
	static void	SnapPos(Vec3f &_Pos)
	{
		_Pos.x =  FLOORF(_Pos.x / m_SnapGranularity + 0.01f) * m_SnapGranularity;
		_Pos.y =  FLOORF(_Pos.y / m_SnapGranularity + 0.01f) * m_SnapGranularity;
		_Pos.z =  FLOORF(_Pos.z / m_SnapGranularity + 0.01f) * m_SnapGranularity;
	}

	void	SetValidZone(const Vec3f &_Min, const Vec3f &_Max);
	void	GetValidZone(Vec3f &_Min, Vec3f &_Max) {_Min = m_Min;_Max = m_Max;}
	void	AddConeView(const Vec3f &_Pos, const Vec3f &_Dir, Float _DegDemiAngle = 30.f, Float _MaxDist = 3.f);

	void	RefreshSRFromStaticDatas();
	void	RefreshSRFromDevice(Playspace_SR_DeviceSR *_pDeviceSR, Bool _UseFilter);

	void	RefreshBlindMapFromDevice(Bool _ForceTotal = FALSE,Bool _ClearFisrt = FALSE);

	Bool	LoadSRMeshRaw(const char *_FileName);
	Bool	SetSRMesh(U32 *_pTabTriIdx, S32 _NbTriIdx,Vec3f *_pTabVtx, S32 _NbVtx);
	Bool	SetSRMesh(Dll_Interface::MeshData* _TabMesh,S32 _NbMesh, const Quat &_MeshToWorld);

	void	RefreshZBuffer(HU8DA &_ZBuffer,const Vec3f &_Pos, const Vec3f &_Dir, Float _Angle, Float _MaxDist);
	void	RefreshZBufferFromConeView();
	void	RefreshBlindAndModeFromZBuffer(Playspace_SR_BlindMap::Mode _Mode,const Vec3f &_Pos, const Vec3f &_Dir,const Vec3f &_LateralDir, Float _HalfAngle = DegToRad(18.f), Float _MaxDist = 3.f);

	void	RefreshBlindFromConeView();
	void	RefreshPaint(const Vec3f &_Pos, const Vec3f &_Dir,const Vec3f &_LateralDir, Float _DegDemiAngle = 18.f, Float _MaxDist = 3.f);
	void	RefreshClear(const Vec3f &_Pos, const Vec3f &_Dir,const Vec3f &_LateralDir, Float _DegDemiAngle = 9.f, Float _MaxDist = 3.f);
	void	RefreshMergePaintToMesh(Playspace_Mesh &_Mesh, Float _fUseDeltaNormal = -1.f) { m_BlindMap.RefreshMergePaintToMesh(_Mesh,_fUseDeltaNormal); }

	void	ThreadSafe_ExtractSRToMesh(Playspace_Mesh &_Mesh,Bool _FilterAndMarkFaces);
	void	ThreadSafe_TotalRefreshBlindMapFromMesh(Playspace_Mesh &_Mesh);
	void	ThreadSafe_TotalRefreshBlindMapFromSR();
	void	SuppressUnpaintBorder(Playspace_Mesh &_Mesh);

	void	ApplyQuat(const Quat &_Transfo,Bool _FullReInsertMesh);
	void	MoveIfNeeded(Bool _FullReInsertMesh);

	U32		GetFrameUpdate() {return m_LastFrameUpdate;}

	S32								GetNextBlock(S32 _NumBlock = -1);
	FINLINE_Z	Playspace_SR_Block	*GetBlock(S32 _NumBlock) {return &m_TabBlocks[_NumBlock];}

	void	ExternalLockBlind(Bool _Lock) { if(_Lock) m_LockThreadBlind.Lock(); else m_LockThreadBlind.Unlock();}
	
};

#endif //__PLAYSPACE_SR_H__
