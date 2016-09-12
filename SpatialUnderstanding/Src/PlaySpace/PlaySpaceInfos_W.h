// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef __PLAYSPACEINFOS_W__
#define __PLAYSPACEINFOS_W__

#include <PlaySpace\PlaySpace_Tools_W.h>
#include <PlaySpace\PlaySpace_OldScan_W.h>
#include <PlaySpace\PlaySpace_ScanV3_W.h>
#include <PlaySpace\PlaySpace_Mesh_W.h>
#include <PlaySpace\PlaySpace_SR_W.h>
#include <Topology\TopologyAnalyzer_W.h>
#include <Topology\ShapeAnalyzer_W.h>

class UnderstandingMgr_W;

class PlaySpace_ZoneInfos
{
public:
	S32		TopologyId;
	Name_Z	ShapeName;
	Name_Z	SlotName;
	Bool	IsCouch;		// Provisoire bien entendu... Il faut faire propre.
};
typedef DynArray_Z<PlaySpace_ZoneInfos,256,TRUE,TRUE>		PlaySpace_ZoneInfosDA;
typedef	DynArray_Z<Vec3fDA,16,TRUE,TRUE>					Vec3fDADA;
typedef	DynArray_Z<PlaySpace_SurfaceInfos,64,FALSE,FALSE>	PInfoDA;

typedef void (*CallBack_CreateShapeDesc)(ShapeReco::ShapeAnalyzer_W	&_ShapeAnalyzer);
class   PlaySpaceMgr_W;

class  PlaySpaceInfos_ScanningStats
{
public:
	Bool				IsWorkingOnStats;
	PlaySpaceMesh_Stats	MeshStats;

	void Reset()
	{
		IsWorkingOnStats = FALSE;
		MeshStats.Reset();
	}

	void operator=(const PlaySpaceInfos_ScanningStats& _other)
	{
		IsWorkingOnStats = _other.IsWorkingOnStats;
		MeshStats = _other.MeshStats;
	}
};

class	PlaySpaceInfos_W
{
friend class PlaySpaceMgr_W;
friend class Playspace_SR_W;
friend class TopologyAnalyzer_W;
friend class UnderstandingMgr_W;

public:
typedef enum {
	PSI_State_NonInitialized,
	PSI_State_Initialized,
	PSI_State_SearchGroundCeil,
	PSI_State_Aligning,
	PSI_State_Snaping,
	PSI_State_WaitEndSnap,
	PSI_State_EndSnap,
	PSI_State_Scanning,
	PSI_State_Finalized,
	PSI_State_Error,
	PSI_State_Scanning_Align,
	PSI_State_Scanning_Finish,
	PSI_State_Loading,
} PSI_State;

typedef enum {
	PSI_Mode_NOP = 0x0,
	PSI_Mode_Paint = 0x1,
	PSI_Mode_Clear = 0x2,
} PSI_UpdateMode;

typedef enum {
	PSI_SCAN_OLD = 0x1,
	PSI_SCAN_NEW = 0x2,
	PSI_SCAN_MESH = 0x4,	// Use Ray if it's not set to TRUE
} PSI_Scan_Type;

protected:
	PSI_State		m_CurState;	// State is out of CRC.
	Float			m_TimeInState;
	PSI_UpdateMode	m_CurUpdateMode;

U8	CRC_ZoneStart;
	Float			m_OptimalZoneSize;

	Float			m_YEyesAccu;
	Float			m_YEyesAccuNb;
	Float			m_YEyesAccuTime;

public:
	Float			m_fConfidence;	//% : 100 is the best

	Vec3f			m_vOriginalPlayerPos;
	Vec3f			m_vCenter;
	Vec3f			m_vOriginalPlayerDir;
	Vec3f			m_AlignXAxis;

	Float			m_AlignSurface;
	S32				m_AlignTransfoId;
	Quat			m_AlignTransfo;

	Vec3f			m_vMinAvailable;
	Vec3f			m_vMaxAvailable;

	Vec3f			m_vNonAlignedZone[4];

	Bool			m_IsVirtualCeiling;
	Float			m_YCeiling;			// This is the clamped one (virtual...)
	Float			m_YCeilingMeasure;	// This is the sampled One.
	Float			m_YCeilingSurface;

	Float			m_YGround;
	Float			m_YGroundMeasure;	// This is the sampled One.
	Float			m_YGroundSurface;

	Float			m_YEyes;
	Float			m_YEyesConfidence;

	Float			m_SizeVoxel;
	S32				m_NbCellX;
	S32				m_NbCellY;
	S32				m_NbCellH;
U8	CRC_ZoneEnd;
	Playspace_SR_W	*m_pSurfaceSR;
	UnderstandingMgr_W	*m_pUnderstandingMgr;

	PlaySpace_SurfaceInfosV3	*m_pSurfaceInfosV3;
	S32				m_GroundZoneID;
	S32				m_NbZoneID;

	Bool			m_ForceRecomputeMode;
	Bool			m_ForceRescanMode;
	Bool			m_bHaveAlignToWall;

	// Scan
	Vec3f			m_vPreviousViewPosRef;
	Vec3f			m_vPreviousScanTestDir;

	// Align Datas.
	PlaySpaceAlign_W	m_pAlignMgr;

	// Protection Tool.
	U32		m_CurrentCRC;
	U32		ComputeCRC();
	Bool	VerifyCRC() {return (m_CurrentCRC==ComputeCRC());}

	static U32						m_CodeDatasChange;

	// TOOLS.
	Bool							m_TopologyActivated;
	TopologyAnalyzer_W				m_TopologyAnalyzer;

	Bool							m_ShapeActivated;
	ShapeReco::ShapeAnalyzer_W		m_ShapeAnalyzer;

	PlaySpace_ZoneInfosDA			m_TabZoneInfos;
	Bool							m_bRefreshZoneInfos;

protected:
	CallBack_CreateShapeDesc		m_pCallBack_CreateDecriptors;

	void	RefreshTabZoneInfos();

	// Internal Job...
	void	ComputeMinMax();
	void	SetState(PSI_State	_CurState);

	void	SetUpdateMode(PSI_UpdateMode _CurUpdateMode);

	void	RefreshEye(Float _dTime,Bool _DrawIt = FALSE);
	void	RefreshGroundAndCeiling(Bool _IsInOnePassMode,Bool _DrawIt = FALSE);
	Bool	RefreshZone(const Vec3f& _AlignXAxis, Bool _DrawIt);
	Bool	InitScan();

	void	SnapToVoxelGrid(Vec3f &_Pos);
	void	SnapComposantToVoxelGrid(Vec3f &_Pos,S32 _Composante);
	void	VerifySnapToVoxelGrid();

	// Specific Mesh Functions.
	Playspace_Mesh	m_TheMeshSR;
	Bool	FilterMeshFromSR();
	Bool	RefreshSurfaceSR(Playspace_SR_DeviceSR *_pDeviceSR,Bool _OnlyIfNew, Bool _UseFilter, Bool _RefreshBlind);
	Bool	RefreshMeshFromSurfaceSR();
	//	Bool	RefreshMeshFromSR(const Game_ZHdl& _hGame, Bool _OnlyIfNew);
	void	RefreshGroundAndCeilingMesh(Bool _IsInOnePassMode, Bool _DrawIt = FALSE);
	Bool	RefreshZoneMesh(const Vec3f& _AlignXAxis, Bool _DrawIt = FALSE);
	Bool	OnePassComputeZoneMesh();

	// New Scan
	void	ComputeScanningBrush(Float _dTime,Float &_DistMin,Float &_CosMin,Vec3f &_Dir);
	Bool	UpdateScanV3(Float _dTime,Bool _DrawIt = FALSE,Bool _OneFrameMode = FALSE);
	void	FinalizeScanV3Mesh(Playspace_Mesh *_pMesh,PlaySpace_SurfelBoard *_pSurfels,Playspace_Area *pArea = NULL);
	void	FinalizeScanV3();
	void	ComputeOneBorderV3(Vec3fDA &TabPoints,S32 x,S32 y,S32 _iLevel,S32 Dx,S32 Dy,S32 Ex,S32 Ey,S32 SizeX);

	// TaskSceduler paint.
	JobGroup_Z			m_PaintingJobs;
	volatile Bool		m_PaintingJobs_Init;
	volatile Bool		m_PaintingJobs_Started;
	volatile Bool		m_PaintingJobs_Finished;
	volatile S32		m_PaintingJobs_NumCount;

	Vec3f				m_PaintingJobs_Pos;
	Vec3f				m_PaintingJobs_Dir;
	PSI_UpdateMode		m_PaintingJobs_Mode;
	S32					m_PaintingJobs_BlindCount;

	// TaskScheduler scanning.
	JobGroup_Z			m_ScanningJobs;

	volatile Bool		m_ScanningJobs_Init;
	volatile Bool		m_ScanningJobs_Started;
	volatile Bool		m_ScanningJobs_Finished;
	volatile S32		m_ScanningJobs_NumCount;
	volatile Bool		m_ScanningJobs_Result;
	volatile PSI_State	m_ScanningJobs_WantedState;
	HMapMeshInfos3D::Scan_Mode			m_ScanningJobs_RefreshMode;
	PlaySpaceInfos_ScanningStats		m_ScanningJobsStats;

	PlaySpaceInfos_ScanningStats		m_ScanningJobsLastStats;

	Bool				m_ScanningRefineMode;
	U8					m_ScanningFrameCounter;
	U8					m_ScanningRequestStop;	// 0 : Nothing 1: Wait Finish 2: Force Stop
	U8					m_ScanningJobsRequestStop;
	Vec3f				m_ScanningJobs_MeshMin;
	Vec3f				m_ScanningJobs_MeshMax;
	Playspace_Area		m_ScanningJobs_Area;
	Playspace_Mesh		m_ScanningJobs_Mesh;
	PlaySpace_SurfelBoard m_ScanningJobs_Surfels;

	Bool				UnifiedScanningUpdate(Playspace_Mesh &_Mesh,PlaySpace_SurfelBoard &_Surfels, Float _dTime);
	static void			UnifiedScanningUpdateTask(const U16 taskIndex, const U16 tasksCount, void* userData);
	static void			UnifiedPaintingUpdateTask(const U16 taskIndex, const U16 tasksCount, void* userData);

	// Load.
//	Bool	LoadFromChunk_OLD(FileChunk_Z& file,const Game_ZHdl& _hGame);	Replace by Standard IO

	void	UpdateTools(Bool _PlaySpaceChanged,Bool _ForceOnLoad,Bool _CanUseJob);

public:
	// Basic.
	PlaySpaceInfos_W();
	~PlaySpaceInfos_W();


	Vec3f GetOriginalPlayerPos() const { return m_vOriginalPlayerPos; }
	Vec3f GetOriginalPlayerDir() const { return m_vOriginalPlayerDir; }
	void  ComputeRoomMinMax(Vec3f &_Min,Vec3f &_Max);

	void	Reset(Bool _DeactivateTools = FALSE);
	PlaySpaceInfos_W		&operator=(const PlaySpaceInfos_W &f);

	// Static Infos.
	static U8	g_TypeScan;
	static Bool	g_SetNewAlign;

	// PlaySpaceMgr Update Call !
	void	SetPlayspaceSR(Playspace_SR_W *_PlayspaceSR) {m_pSurfaceSR = _PlayspaceSR;}
	void	UpdatePlaySpace(UnderstandingMgr_W *_pMyMgr,Float _dTime);

	// Lààààà ! à modifier...
	Bool	IsInitialized() const {return (m_CurState != PSI_State_NonInitialized) && (m_CurState != PSI_State_Loading);}
	Bool	IsFinalized() const {return m_CurState == PSI_State_Finalized;}
	Bool	IsScanning() const {return m_CurState == PSI_State_Scanning;}
	Bool	IsValid(Float _MinConfidence = 0.f) const;

	Bool	IsPainting() const {return m_CurUpdateMode != PSI_Mode_NOP;}

	// SCANNING FLOW
	// 1- Init
	Bool	InitProcess(const Vec3f& _vPlayerPosition,const Vec3f& _vPlayerFrontDir,Float _fSearchDist,Float _OptimalSize);

	// 2- Unified Scan Process => V3 : Align + Scan.
	PSI_State	UpdateScanV3_Unified(Float _dTime, PSI_UpdateMode _Mode, Float _SurfaceValidation, S32 &_OutAlignTransfoId) { return UpdateScanV3_Unified(NULL, _dTime, _Mode, _SurfaceValidation, _OutAlignTransfoId); };
	PSI_State	UpdateScanV3_Unified(Playspace_SR_DeviceSR *_pDeviceSR,Float _dTime, PSI_UpdateMode _Mode, Float _SurfaceValidation, S32 &_OutAlignTransfoId);
	void		RequestFinishUnifiedScanning(Bool _Force = FALSE);
	void		GetUnifiedScanV3Stats(PlaySpaceInfos_ScanningStats &_Stats);
	void		ComputeMeshStats(Playspace_Mesh& _Mesh, PlaySpaceMesh_Stats& _outStats);
	PlaySpaceMesh_Stats& GetScanningJobsLastMeshStats() { return m_ScanningJobsLastStats.MeshStats; }

	Bool	HaveAlignToWall() {return m_bHaveAlignToWall;}
	FINLINE_Z void		ApplyAutoFitTransfo(Vec3f &_Point) {_Point = m_AlignTransfo * _Point;}
	FINLINE_Z void		ApplyAutoFitTransfo(Quat &_Rot) { _Rot = m_AlignTransfo * _Rot; }
	FINLINE_Z Quat		GetAutoFitTransfo() { return m_AlignTransfo; }

	// 3-  Finalize or restart.
	void	FinalizeScan();
	void	RefineScan();		// For restart after Finalized.

	// OnePAss...
	Bool	OnePassComputePlaySpace(const Vec3f& _vPlayerPosition,const Vec3f& _vPlayerFrontDir,Float _fSize);
	static	Float	DefaultSizeOnePassPlayspace;

	// Post-Process
	// Topology / Shape Tools (After Finalize).
	Bool	IsActivTopology() const {return m_TopologyActivated;}
	Bool	IsActivShape() const {return m_ShapeActivated;}

	void	ActivateTopology();
	void	ActivateShape();

	void	DeActivateTopology();
	void	DeActivateShape();

	void	SetShapeDescriptor(CallBack_CreateShapeDesc _pCallBack);

	// Tools.
	void	AlignSurfaceReco(const Vec3f &_XAxis, Bool _NoThreadAlignVersion = FALSE);
	Bool	GetBestCentroidPosition(Vec3f& result, S32 granularity = 4);

	// Collision Tools.
	Bool	IsInsideBBox(const Vec3f& _vPos, Float _Tolerance=0.01f);	// Tolerance Positif si objet épais (sort quand bord touche), négatif si on tolère que le centre sorte un peu.
	Bool	IsInside(const Vec3f& _vPos, Float _Tolerance=0.01f);	// Tolerance Positif si objet épais (sort quand bord touche), négatif si on tolère que le centre sorte un peu.
	Bool	IsPosFilled(const Vec3f& _vPos, Float _Tolerance=0.01f) { return !IsInside(_vPos,_Tolerance); }
	Bool	RayCastVoxel(const Vec3f &p1,const Vec3f &p2,Vec3f &pResult, S32 *_pZoneID=NULL, Bool _precise = FALSE, PlaySpace_Surfel **_ppSurfel = NULL);	// Collide with VOXEL => not faces.
	Bool	IsCollidingSphere(const Vec3f &p,Float r);
	Bool	SnapToEdge(Vec3f &_pos, const Vec3f &_Dir, Float DistFromEdge, Float DistMax, Bool _OkForWall, Bool _OkForEdge,Bool _OkForVirtual);

	struct RaycastY_Result
	{
		Vec3f	m_vInter;
		Vec3f	m_vNormal;
		Bool	m_bIsBasin;
		Bool	m_bIsCouch;
		S16		m_ZoneId;

		RaycastY_Result()
		{
			m_vInter = VEC3F_NULL;
			m_vNormal = VEC3F_NULL;
			m_bIsBasin = FALSE;
			m_bIsCouch = FALSE;
			m_ZoneId = -1;
		}
	};
	Bool	RayCastY(const Vec3f& p, Float fUpLength,RaycastY_Result* _pResult = NULL) const;
	FINLINE_Z Bool ComputeCellIndice(const Vec3f& _vPoint, S32& _outX, S32& _outY) const
	{
		_outX = FLOORINT((_vPoint.x - m_vMinAvailable.x) / m_SizeVoxel);
		_outY = FLOORINT((_vPoint.z - m_vMinAvailable.z) / m_SizeVoxel);

		return (_outX >= 0 && _outX < m_NbCellX && _outY >= 0 && _outY < m_NbCellY);
	}

	FINLINE_Z Vec3f ComputeCellPos(S32 _x, S32 _y) const
	{
#ifdef _GAMEDEBUG
		EXCEPTIONC_Z(_x >= 0 && _x < m_NbCellX, "%s (%s:%d) bad x indice", __FUNCTION__, __FILE__, __LINE__);
		EXCEPTIONC_Z(_y >= 0 && _y < m_NbCellY, "%s (%s:%d) bad y indice", __FUNCTION__, __FILE__, __LINE__);
#endif

		Vec3f pos;
		pos.x = m_vMinAvailable.x + _x * m_SizeVoxel;
		pos.y = m_vMinAvailable.y;
		pos.z = m_vMinAvailable.z + _y * m_SizeVoxel;
		return pos;
	}

	// Path Finding Functions...
	struct NeightborPos
	{
		Vec3i	m_vCellPos;
		Vec3f	m_vWorldPos;
		Vec3f	m_vWorldNormal;
		Float	m_fHDelta;

		NeightborPos()
		{
			m_vCellPos = VEC3I_NULL;
			m_vWorldPos = VEC3F_NULL;
			m_vWorldNormal = VEC3F_NULL;
			m_fHDelta = 0.f;
		};

		Vec3f	GetRealPos() const	{ return m_vWorldPos + m_vWorldNormal * m_fHDelta; }
	};	

	Bool	GetNeighborPos(const Vec3f& _vPos, const Vec3f& _vNormal, PlaySpaceInfos_W::NeightborPos& _outPos);
	Bool	GetNearestNeighborPos(const Vec3f& _vPos, PlaySpaceInfos_W::NeightborPos& _outPos);
	void	GetNeighborsFromNeightborPos(const Vec3f& _inPos, const Vec3f& _inNormal, DynArray_Z<NeightborPos>& _outNeighbor);

	Bool	GetBorder(Vec3fDA &_TabPoints,Bool _Ground,Float _DistMargin);	// Ceil if false.

	// Debug Commande.
	void	ForceAlign();
	void	ForceReScan(Bool _IsSet);
	Bool	IsForceReScan() {return m_ForceRescanMode;}
	void	MovePlaySpace(Vec3f &_Delta);

	void	ForceReCompute(Bool _IsSet) {m_ForceRecomputeMode = _IsSet;}

	U32		GetCodeChange() { return PlaySpaceInfos_W::m_CodeDatasChange; }

	// Mesh Specific Functions.
	Playspace_Mesh	&GetMeshSR() { return m_TheMeshSR; }

	PlaySpace_SurfaceInfosV3 *GetSurfaceInfosV3() { return m_pSurfaceInfosV3; }
};

#endif //__PLAYSPACEINFOS_W__
