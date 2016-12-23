// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include <pch.h>
#include <PlaySpace\PlaySpaceInfos_W.h>
#include <GenericQSort_Z.h>
#include <PlaySpace\PlaySpaceInfos_Cst_W.h>
#include <UnderstandingMgr_W.h>

#include <Bresenham2D_Z.h>

//#define DRAW_TIME_SCAN	1

//#define DISABLE_INPUTMESH_FLUSH

U8			PlaySpaceInfos_W::g_TypeScan = PlaySpaceInfos_W::PSI_SCAN_OLD;
Bool		PlaySpaceInfos_W::g_SetNewAlign = TRUE;
Float		PlaySpaceInfos_W::DefaultSizeOnePassPlayspace = 5.f;

U32			PlaySpaceInfos_W::m_CodeDatasChange = 0;
//static Vec3f	StaticMoveDelta = VEC3F_NULL;
PlaySpaceInfos_W::PlaySpaceInfos_W() : m_ShapeAnalyzer(m_TopologyAnalyzer)
{
	m_fConfidence = -1.f;
	m_vCenter = VEC3F_NULL;
	m_vMinAvailable = VEC3F_NULL;
	m_vMaxAvailable = VEC3F_NULL;
	m_pSurfaceInfosV3 = NULL;
	m_NbCellX = m_NbCellY = 0;
	m_NbCellH = 0;
	m_YCeilingMeasure = -1e6f;
	m_IsVirtualCeiling = FALSE;

	m_YGround  = 0.f;
	m_YGroundMeasure = m_YGround;
	m_YGroundSurface = -1.f;

	m_YCeiling = 0.f;
	m_YCeilingMeasure = m_YCeiling;
	m_YCeilingSurface = -1.f;

	m_ForceRecomputeMode = FALSE;
	m_ForceRescanMode = FALSE;
	m_vPreviousViewPosRef = VEC3F_NULL;
	m_vPreviousScanTestDir = VEC3F_NULL;
	m_vNonAlignedZone[0] = VEC3F_NULL;
	m_vNonAlignedZone[1] = VEC3F_NULL;
	m_vNonAlignedZone[2] = VEC3F_NULL;
	m_vNonAlignedZone[3] = VEC3F_NULL;
	m_OptimalZoneSize = 0.f;
	m_SizeVoxel = DEFAULT_SIZE_VOXEL;
	SetState(PSI_State_NonInitialized);
	SetUpdateMode(PSI_Mode_NOP);
	m_pSurfaceSR = NULL;
	m_pUnderstandingMgr = NULL;
	m_CurrentCRC = 0;
	CRC_ZoneStart = 0;
	CRC_ZoneEnd = 0;
	m_TopologyActivated = FALSE;
	m_ShapeActivated = FALSE;
	m_pCallBack_CreateDecriptors = NULL;

	m_ScanningJobs_Init = FALSE;
	m_ScanningJobs_Started = FALSE;
	m_ScanningJobs_Finished = FALSE;
	m_ScanningJobs_NumCount = 0;

	m_PaintingJobs_Init = FALSE;
	m_PaintingJobs_Started = FALSE;
	m_PaintingJobs_Finished = FALSE;
	m_PaintingJobs_NumCount = 0;

	m_bHaveAlignToWall = FALSE;
}

/**************************************************************************/

PlaySpaceInfos_W::~PlaySpaceInfos_W()
{
	Reset();
	m_pSurfaceSR = NULL;

	m_ScanningJobs_Init = FALSE;
	m_ScanningJobs_Started = FALSE;
	m_ScanningJobs_Finished = FALSE;
	m_ScanningJobs_NumCount = 0;

	m_PaintingJobs_Init = FALSE;
	m_PaintingJobs_Started = FALSE;
	m_PaintingJobs_Finished = FALSE;
	m_PaintingJobs_NumCount = 0;
}

/**************************************************************************/

void	PlaySpaceInfos_W::Reset(Bool _DeactivateTools)
{
	if (IsScanning())
	{
		// Protect if scanning is active...
		RequestFinishUnifiedScanning(TRUE);

		// Finalize / Stop / Wait process.
		if (g_TypeScan & PSI_SCAN_NEW)
			FinalizeScanV3();

		// Stop Task.
		if (m_ScanningJobs_Init)
		{
			m_ScanningJobs.Shut();
			m_ScanningJobs_Init= FALSE;
		}
		m_ScanningJobs_Started = FALSE;
		m_ScanningJobs_Finished = FALSE;

		if (m_PaintingJobs_Init)
		{
			m_PaintingJobs.Shut();
			m_PaintingJobs_Init= FALSE;
		}
		m_PaintingJobs_Started = FALSE;
		m_PaintingJobs_Finished = FALSE;
	}

	// ... and reset.
	SetState(PSI_State_NonInitialized);
	m_TimeInState = 0.f;

	SetUpdateMode(PSI_Mode_NOP);

	m_fConfidence = -1.f;
	m_vCenter = VEC3F_NULL;
	m_vMinAvailable = VEC3F_NULL;
	m_vMaxAvailable = VEC3F_NULL;

	m_NbCellX = m_NbCellY = 0;
	m_NbCellH = 0;

	if (m_pSurfaceInfosV3)
		Delete_Z m_pSurfaceInfosV3;
	m_pSurfaceInfosV3 = NULL;

//	m_ForceRecomputeMode = FALSE;
	m_ForceRescanMode = FALSE;
	m_vPreviousViewPosRef = VEC3F_NULL;
	m_vPreviousScanTestDir = VEC3F_NULL;
	m_vOriginalPlayerDir = VEC3F_FRONT;

	m_YGround  = 0.f;
	m_YGroundMeasure = m_YGround;
	m_YGroundSurface = -1.f;

	m_YCeiling = 0.f;
	m_YCeilingMeasure = m_YCeiling;
	m_YCeilingSurface = -1.f;
	
	m_YEyes = 0.f;
	m_YEyesConfidence = -1.f;

	m_YEyesAccu = 0.f;
	m_YEyesAccuTime = 0.f;
	m_YEyesAccuNb = 0.f;

	m_OptimalZoneSize = 0.f;

	m_AlignTransfoId = 0;
	m_AlignTransfo = QUAT_NULL;

	m_TopologyAnalyzer.Reset();
	m_ShapeAnalyzer.Reset();
	m_TabZoneInfos.Flush();

	m_bRefreshZoneInfos = FALSE;

	if (_DeactivateTools)
	{
		m_TopologyActivated = FALSE;
		m_ShapeActivated = FALSE;
	}

	m_GroundZoneID = -1;
	m_NbZoneID = 0;

	m_ScanningRefineMode = FALSE;

	m_ScanningJobs.Shut();
	m_ScanningJobs_Init = FALSE;
	m_ScanningJobs_Started = FALSE;
	m_ScanningJobs_Finished = FALSE;
	m_ScanningJobs_NumCount = 0;

	m_PaintingJobs.Shut();
	m_PaintingJobs_Init = FALSE;
	m_PaintingJobs_Started = FALSE;
	m_PaintingJobs_Finished = FALSE;
	m_PaintingJobs_NumCount = 0;

	m_ScanningJobsLastStats.Reset();
	m_ScanningRequestStop = 0;
}


/**************************************************************************/

//static PlaySpaceInfos_W::PSI_UpdateMode TheMode = PlaySpaceInfos_W::PSI_Mode_Paint;
//Playspace_Mesh	TestSR;
//static S32 Tutututu = 0;
PlaySpaceInfos_W::PSI_State	PlaySpaceInfos_W::UpdateScanV3_Unified(Playspace_SR_DeviceSR *_pDeviceSR, Float _dTime,PSI_UpdateMode _Mode, Float _SurfaceValidation, S32 &_OutAlignTransfoId)
{
//	_Mode = TheMode;
	_OutAlignTransfoId = -1;
	static Float UpdateScanV3_Unified_Time = 0.f;

	EXCEPTIONC_Z((g_TypeScan & PSI_SCAN_NEW) != 0, "Cannot Use UpdateScanV3_Unified in V1");
	EXCEPTIONC_Z((g_TypeScan & PSI_SCAN_MESH) != 0, "Cannot Use UpdateScanV3_Unified in Non Mesh Scan");

/*{
		// FINISH MODE.
		// Force Valid Zone on Valid Datas.
		S32		NbFaces = TestSR.m_TabQuad.GetSize();
		Vec3f	MeshMin = m_vCenter;
		Vec3f	MeshMax = m_vCenter;

		for (S32 i = 0; i<NbFaces ; i++)
		{
			Playspace_Mesh::Face &CurFace = TestSR.m_TabQuad[i];
			if (CurFace.IsPaintMode != 2)	// Keep Only paint faces.
				continue;
			Vec3f &Pos0 = TestSR.m_TabPoints[CurFace.TabPoints[0]];
			MeshMin = Min(MeshMin, Pos0);
			MeshMax = Max(MeshMax, Pos0);
			Vec3f &Pos1 = TestSR.m_TabPoints[CurFace.TabPoints[1]];
			MeshMin = Min(MeshMin, Pos1);
			MeshMax = Max(MeshMax, Pos1);
			Vec3f &Pos2 = TestSR.m_TabPoints[CurFace.TabPoints[2]];
			MeshMin = Min(MeshMin, Pos2);
			MeshMax = Max(MeshMax, Pos2);
if (Tutututu)
{
	DRAW_DEBUG_SPHERE3D((Pos0+Pos1+Pos2) * 0.3333f, COLOR_GREEN, 0.03f);
}
		}

		SnapToVoxelGrid(MeshMin);
		SnapToVoxelGrid(MeshMax);

		Vec3f	p1(MeshMin.x, m_YGround + 0.1f, MeshMin.z);
		Vec3f	p2(MeshMax.x, m_YGround + 0.1f, MeshMin.z);
		Vec3f	p3(MeshMax.x, m_YGround + 0.1f, MeshMax.z);
		Vec3f	p4(MeshMin.x, m_YGround + 0.1f, MeshMax.z);


		DRAW_DEBUG_LINE3D(p1, p2, COLOR_GREEN, 0.05f);
		DRAW_DEBUG_LINE3D(p2, p3, COLOR_GREEN, 0.05f);
		DRAW_DEBUG_LINE3D(p3, p4, COLOR_GREEN, 0.05f);
		DRAW_DEBUG_LINE3D(p4, p1, COLOR_GREEN, 0.05f);
}*/

	m_TimeInState += _dTime;
	SetUpdateMode(_Mode);

	// Init Sccaning Job.
	if (!m_ScanningJobs_Init)
	{
		Bool TaskSchedulerIsOK = TaskScheduler_Z::TheUnicJobScheduler.InitIfNeeded();
		EXCEPTIONC_Z(TaskSchedulerIsOK, "Problem with TaskScheduler_Z");
		OUTPUT_Z("AsynchronousUpdateScanV3 INIT");
		m_ScanningJobs_Init = TRUE;

		U16	excludedProcessorAffinities[] = { 0, 3, 5 };//3, 5};
		m_ScanningJobs.Init(excludedProcessorAffinities, _countof(excludedProcessorAffinities), "PlayspaceScan");
		m_ScanningJobs_Started = FALSE;
		m_ScanningJobs_Finished = FALSE;

		m_fConfidence = 0.f;
		m_ScanningRequestStop = 0;
		m_ScanningJobsStats.Reset();
		m_ScanningFrameCounter = 0;

		m_vPreviousViewPosRef = m_vOriginalPlayerPos;
		m_vPreviousScanTestDir = m_vOriginalPlayerPos + m_vOriginalPlayerDir * 1.5f;

		// Init Scanning.
		if (m_CurState == PSI_State_Initialized)
		{
			// Only if new scan...
			EXCEPTIONC_Z(m_pSurfaceInfosV3 == NULL,"UnifiedScanningUpdate : Possible crash detected...");
			SetState(PSI_State_Scanning);	// Direct Go To Scanning Step.

			// Set Min Max to MAX.
			Float	HalfZone = m_OptimalZoneSize * 0.5f;
			m_vMinAvailable = m_vCenter;
			m_vMinAvailable.x -= HalfZone;
			m_vMinAvailable.z -= HalfZone;
			m_vMinAvailable.y -= 3.f;

			m_vMaxAvailable = m_vCenter;
			m_vMaxAvailable.x += HalfZone;
			m_vMaxAvailable.z += HalfZone;
			m_vMaxAvailable.y += 3.f;

			m_YGround  = m_vMinAvailable.y;
			m_YGroundMeasure = m_YGround;
			m_YGroundSurface = -1.f;

			m_ScanningJobs_MeshMin = m_vCenter - VEC3F_ONE;
			m_ScanningJobs_MeshMax = m_vCenter + VEC3F_ONE;

			m_YCeiling = m_vMaxAvailable.y;
			m_YCeilingMeasure = m_YCeiling;
			m_YCeilingSurface = -1.f;
			InitScan();
		}

		// Set Valid Zone.
		m_pSurfaceSR->SetValidZone(m_vMinAvailable,m_vMaxAvailable);
	}

	// Init Painting Job.

	if (!m_PaintingJobs_Init)
	{
		m_PaintingJobs_Init = TRUE;
		U16	excludedProcessorAffinities[] = { 0, 3, 5 };//3, 5};
		m_PaintingJobs.Init(excludedProcessorAffinities, _countof(excludedProcessorAffinities), "PlayspacePaint");
		m_PaintingJobs_Started = FALSE;
		m_PaintingJobs_Finished = FALSE;
		m_PaintingJobs_BlindCount = 0;
	}

	// Update Eye (OUTSIDE THREAD!!!!)
	RefreshEye(_dTime);

	// Add Cone View.
	Segment_Z	SegView;
	Util_L::GetViewSegment(&SegView);
	m_pSurfaceSR->AddConeView(SegView.Org, SegView.Dir);

	// Refresh Blind and paint...
	if (!m_PaintingJobs_Started)
	{
		// LaunchTask Scan?
		if (!m_ScanningJobs_Started)
		{
			if (!m_ScanningRequestStop && (_Mode == PSI_UpdateMode::PSI_Mode_NOP))
				return m_CurState;
			OUTPUT_Z("AsynchronousUpdateScanV3 LAUNCH");
			UpdateScanV3_Unified_Time = GetAbsoluteTime();

			// Refresh SR from Device before scan pass.
			if (!m_ScanningRequestStop && (_Mode == PSI_UpdateMode::PSI_Mode_Paint))
			{
				if (!RefreshSurfaceSR(_pDeviceSR,TRUE, TRUE, FALSE))
					return m_CurState;
			}
			if (!m_ScanningRequestStop && (m_pSurfaceSR->GetRefreshStatus() != Playspace_SR_Status::STATUS_OK))
				return m_CurState;

			// Avoid Crash ... :(
			if (m_ScanningRequestStop)
				m_pSurfaceInfosV3->m_Surfels.Empty();

			// And Process IT.
			m_ScanningJobsRequestStop = m_ScanningRequestStop;

			m_ScanningJobs_Started = TRUE;
			m_ScanningJobs_Finished = FALSE;
			m_ScanningJobs_NumCount = 0;
			m_ScanningJobs_WantedState = m_CurState;
			m_ScanningJobs_Area.Min = m_vMinAvailable;
			m_ScanningJobs_Area.Max = m_vMaxAvailable;
			m_ScanningJobs_Area.Center = m_vCenter;
			m_ScanningJobs_Area.NbCellX = m_NbCellX;
			m_ScanningJobs_Area.NbCellY = m_NbCellY;
			m_ScanningJobs_Area.NbCellH = m_NbCellH;
			m_ScanningJobs_Area.SizeVoxel = m_SizeVoxel;

			if (!m_ScanningJobsRequestStop)
			{
				// Freeze Stats if resquest finsih
				m_ScanningJobsStats.Reset();
				m_ScanningJobsLastStats.IsWorkingOnStats = TRUE;
			}

			m_ScanningJobs.Run(1, UnifiedScanningUpdateTask, this);
		}
		else if (m_ScanningJobs_Finished)
		{
			if (m_ScanningJobs.PollForCompletion())
			{
	/*static S32 jojo = 0;
	jojo++;
	if (jojo > 20)
	{
			{
	jojo = 0;
	UnifiedScanningUpdateTask(0,0,this);*/
				UpdateScanV3_Unified_Time = GetAbsoluteTime() - UpdateScanV3_Unified_Time;
				OUTPUT_Z("AsynchronousUpdateScanV3 FINISHED => %.3fs", UpdateScanV3_Unified_Time);
				// Get Result.
				if (m_ScanningJobs_Result)
				{
					// Report new Min Max Center.
					m_vMinAvailable = m_ScanningJobs_Area.Min;
					m_vMaxAvailable = m_ScanningJobs_Area.Max;
					m_NbCellX = m_ScanningJobs_Area.NbCellX;
					m_NbCellY = m_ScanningJobs_Area.NbCellY;
					m_NbCellH = m_ScanningJobs_Area.NbCellH;
					m_vCenter = m_ScanningJobs_Area.Center;

					VerifySnapToVoxelGrid();

					// Swap Mesh && Surfels
					m_pSurfaceInfosV3->m_Mesh.Swap(m_ScanningJobs_Mesh);
					m_pSurfaceInfosV3->m_MeshCycleID++;
					m_pSurfaceInfosV3->m_Surfels.Swap(m_ScanningJobs_Surfels);

					// Surface Update.
					m_ScanningJobsLastStats = m_ScanningJobsStats;

					if (m_ScanningJobsLastStats.MeshStats.HorizSurface >= _SurfaceValidation)
						m_fConfidence = 1.f;
				}
				m_ScanningJobsLastStats.IsWorkingOnStats = FALSE;

	//			TestSR = m_TheMeshSR;
				m_ScanningJobs_Mesh.Flush(TRUE);
				m_ScanningJobs_Surfels.Flush(TRUE);

				// Finish.
				m_ScanningJobs_Started = FALSE;
				m_ScanningJobs_Finished = FALSE;

				// Align ?
				if (m_ScanningJobs_WantedState == PSI_State_Scanning_Align)
				{
					// Finalize Alignement.
					AlignSurfaceReco( m_AlignXAxis, FALSE);
					m_pSurfaceInfosV3->m_Mesh.ApplyQuat(m_AlignTransfo);
					m_pSurfaceInfosV3->m_MeshCycleID++;
					_OutAlignTransfoId = m_AlignTransfoId;
					m_ScanningJobs_WantedState = m_CurState;
					m_ScanningJobs_RefreshMode = HMapMeshInfos3D::ScanModeIncrementalReInit;

					// Force total reinsert Blind Mesh.
					m_PaintingJobs_BlindCount = 0;
					return PSI_State_Scanning_Align;
				}
				else if (!m_ScanningRefineMode)
				{
					// Move ?
					m_pSurfaceSR->MoveIfNeeded(FALSE);
					// Force total reinsert Blind Mesh.
					m_PaintingJobs_BlindCount = 0;
				}
				// Finish.
				if (    (m_ScanningRequestStop == 2)
					||  (m_ScanningJobs_WantedState == PSI_State_Scanning_Finish)
					)
				{
					m_ScanningJobs_WantedState = m_CurState;
					return PSI_State_Scanning_Finish;
				}
			}
		}
		else
		{
			if (!m_pSurfaceInfosV3)
				return m_CurState;
			if (m_ScanningRequestStop)
				return m_CurState;
			if (_Mode == PSI_UpdateMode::PSI_Mode_NOP) 
				return m_CurState;
			if (m_CurState != PSI_State_Scanning)
				return m_CurState;
			if (m_pSurfaceSR->GetRefreshStatus() != Playspace_SR_Status::STATUS_OK)
				return m_CurState;

			// Refresh SR
			RefreshSurfaceSR(_pDeviceSR,TRUE, TRUE, FALSE);

			// Refresh Scan Pos / Dir.
			Segment_Z	SegView;
			Util_L::GetViewSegment(&SegView);

			Vec3f	RefPos = SegView.Org + SegView.Dir * 1.5f;
			Vec3f	vTestDir = VEC3F_NULL;

			vTestDir = RefPos - m_vPreviousViewPosRef;
			vTestDir -= (vTestDir * SegView.Dir) * SegView.Dir;
			if( !vTestDir.ANormalize() )
			{
				vTestDir = m_vPreviousScanTestDir;
			}
			else 
			{
				if( m_vPreviousScanTestDir != VEC3F_NULL )
				{
					Quat q(m_vPreviousScanTestDir,vTestDir);
					q.Maximize(4.f * Pi * _dTime);
					vTestDir = q * m_vPreviousScanTestDir;
					vTestDir -= (vTestDir * SegView.Dir) * SegView.Dir;
				}

				m_vPreviousScanTestDir = vTestDir;
				m_vPreviousViewPosRef = RefPos;
			}

			m_PaintingJobs_Pos = SegView.Org;
			m_PaintingJobs_Dir = SegView.Dir;

			// Run Task.
			m_PaintingJobs_Mode = _Mode;
			m_PaintingJobs_Started = TRUE;
			m_PaintingJobs_Finished = FALSE;
			m_PaintingJobs_NumCount = 0;
			m_PaintingJobs_BlindCount--;
			m_PaintingJobs.Run(1, UnifiedPaintingUpdateTask, this);
		}
	}
	else if (m_PaintingJobs_Finished && m_PaintingJobs.PollForCompletion())
	{
		// Paint Mesh...
		if (!m_pSurfaceInfosV3->m_Mesh.IsEmpty())
			m_pSurfaceSR->RefreshMergePaintToMesh(m_pSurfaceInfosV3->m_Mesh, 0.1f);

		// Finish.
		m_PaintingJobs_Started = FALSE;
		m_PaintingJobs_Finished = FALSE;

		// BlindCount Reset ?
		if (m_PaintingJobs_BlindCount < 0)
			m_PaintingJobs_BlindCount = 10;
	}

	return m_CurState;
}

/**************************************************************************/
//static Bool Nothing = FALSE;
//static Bool PaintNew = TRUE;
void	PlaySpaceInfos_W::UnifiedPaintingUpdateTask(const U16 taskIndex, const U16 tasksCount, void* userData)
{
	Float t0 = GetAbsoluteTime();
	PlaySpaceInfos_W	*pPlayspace = (PlaySpaceInfos_W*)userData;
	S32 CurJobNum = Thread_Z::SafeIncrement(&pPlayspace->m_PaintingJobs_NumCount);
	if (CurJobNum != 1)
		return;

	// Refresh Blind Blocs.
	Bool bFullRefresh = (pPlayspace->m_PaintingJobs_BlindCount < 0);
	pPlayspace->m_pSurfaceSR->RefreshBlindMapFromDevice(bFullRefresh,bFullRefresh);

	Float t1 = GetAbsoluteTime();
	// Refresh Seen.
/*	if (Nothing)
	{
		pPlayspace->m_PaintingJobs_Finished = TRUE;
		return;
	}*/
	Float t2;
#if 1
	{
		// NEW PAINT
		pPlayspace->m_pSurfaceSR->RefreshZBufferFromConeView();
 t2 = GetAbsoluteTime();
		if (pPlayspace->m_PaintingJobs_Mode == PSI_UpdateMode::PSI_Mode_Paint)
			pPlayspace->m_pSurfaceSR->RefreshBlindAndModeFromZBuffer(Playspace_SR_BlindMap::Mode_Paint,pPlayspace->m_PaintingJobs_Pos,pPlayspace->m_PaintingJobs_Dir,pPlayspace->m_vPreviousScanTestDir,DegToRad(18.f));
		else if (pPlayspace->m_PaintingJobs_Mode == PSI_UpdateMode::PSI_Mode_Clear)
			pPlayspace->m_pSurfaceSR->RefreshBlindAndModeFromZBuffer(Playspace_SR_BlindMap::Mode_Clear,pPlayspace->m_PaintingJobs_Pos,pPlayspace->m_PaintingJobs_Dir,VEC3F_NULL,DegToRad(9.f));
		else
			pPlayspace->m_pSurfaceSR->RefreshBlindAndModeFromZBuffer(Playspace_SR_BlindMap::Mode_Blind,VEC3F_NULL,VEC3F_NULL,VEC3F_NULL);
	}
#else
	{
		// OLD PAINT
		pPlayspace->m_pSurfaceSR->RefreshBlindFromConeView();
 t2 = GetAbsoluteTime();
		// Refresh Paint.
		if (pPlayspace->m_PaintingJobs_Mode == PSI_UpdateMode::PSI_Mode_Paint)
			pPlayspace->m_pSurfaceSR->RefreshPaint(pPlayspace->m_PaintingJobs_Pos,pPlayspace->m_PaintingJobs_Dir,pPlayspace->m_vPreviousScanTestDir);
		else if (pPlayspace->m_PaintingJobs_Mode == PSI_UpdateMode::PSI_Mode_Clear)
			pPlayspace->m_pSurfaceSR->RefreshClear(pPlayspace->m_PaintingJobs_Pos,pPlayspace->m_PaintingJobs_Dir,pPlayspace->m_vPreviousScanTestDir);
	}
#endif
 	Float t3 = GetAbsoluteTime();
//	OUTPUT_Z("AsynchronousUpdateScanV3 PAINT JOB %d %d %.3fs => %.3fs %.3fs %.3fs", bFullRefresh,PaintNew, t3-t0,t1-t0,t2-t1,t3-t2);

	pPlayspace->m_PaintingJobs_Finished = TRUE;
}

/**************************************************************************/

void	PlaySpaceInfos_W::UnifiedScanningUpdateTask(const U16 taskIndex, const U16 tasksCount, void* userData)
{
	PlaySpaceInfos_W	*pPlayspace = (PlaySpaceInfos_W*)userData;
	S32 CurJobNum = Thread_Z::SafeIncrement(&pPlayspace->m_ScanningJobs_NumCount);
	if (CurJobNum != 1)
		return;
	pPlayspace->m_ScanningJobs_Result = pPlayspace->UnifiedScanningUpdate(pPlayspace->m_ScanningJobs_Mesh,pPlayspace->m_ScanningJobs_Surfels, 1 / 60.f);
	pPlayspace->m_ScanningJobs_Finished = TRUE;
}

/**************************************************************************/
#define ABORT_USU_IF_NEEDED	if (m_ScanningJobsRequestStop != m_ScanningRequestStop)	return FALSE;	// ABORT !


Bool	PlaySpaceInfos_W::UnifiedScanningUpdate(Playspace_Mesh &_Mesh,PlaySpace_SurfelBoard &_Surfels, Float _dTime)
{
	// Is in Mode STOP SCAN : Store at the start of the scan frame.
	Bool HaveRequestedFinish = m_ScanningJobsRequestStop != 0;
	Bool FastMode = !HaveRequestedFinish;

	Float t0 = GetAbsoluteTime();

	// RefreshSR.
	if (!RefreshMeshFromSurfaceSR())
		return FALSE;

	Float t0b = GetAbsoluteTime();
	m_TheMeshSR.RemoveIsolatedFaces(30);	// Have to be done now because of Normal futur computing.

	Float t0c = GetAbsoluteTime();
	ABORT_USU_IF_NEEDED;

	// Update Valid Zone ?
	if (HaveRequestedFinish)
	{
		// FINISH MODE.
		if (m_ScanningRefineMode)
		{
			m_ScanningJobs_RefreshMode = HMapMeshInfos3D::ScanModeComplete;
			m_pSurfaceSR->ThreadSafe_TotalRefreshBlindMapFromSR();	// Because not done in the Paint Task (pause).
		}
		else
		{
			// Force Valid Zone on Valid Datas.
			S32		NbFaces = m_TheMeshSR.m_TabQuad.GetSize();
			Vec3f	MeshMin = m_ScanningJobs_Area.Center;
			Vec3f	MeshMax = m_ScanningJobs_Area.Center;

			for (S32 i = 0; i<NbFaces ; i++)
			{
				Playspace_Mesh::Face &CurFace = m_TheMeshSR.m_TabQuad[i];
				if (CurFace.IsPaintMode != 2)	// Keep Only paint faces.
					continue;
				Vec3f &Pos0 = m_TheMeshSR.m_TabPoints[CurFace.TabPoints[0]];
				MeshMin = Min(MeshMin, Pos0);
				MeshMax = Max(MeshMax, Pos0);
				Vec3f &Pos1 = m_TheMeshSR.m_TabPoints[CurFace.TabPoints[1]];
				MeshMin = Min(MeshMin, Pos1);
				MeshMax = Max(MeshMax, Pos1);
				Vec3f &Pos2 = m_TheMeshSR.m_TabPoints[CurFace.TabPoints[2]];
				MeshMin = Min(MeshMin, Pos2);
				MeshMax = Max(MeshMax, Pos2);
			}

			m_ScanningJobs_Area.Min = Max(MeshMin,m_ScanningJobs_Area.Min);
			m_ScanningJobs_Area.Min.y = m_YGround - MARGIN_GROUND_CEILING;
			SnapToVoxelGrid(m_ScanningJobs_Area.Min);

			m_ScanningJobs_Area.Max = Min(MeshMax,m_ScanningJobs_Area.Max);
			m_ScanningJobs_Area.Max.y = m_YCeiling + MARGIN_GROUND_CEILING;
			SnapToVoxelGrid(m_ScanningJobs_Area.Max);

			// GetFiltered height.
			m_ScanningJobs_Area.NbCellX = ((m_ScanningJobs_Area.Max.x - m_ScanningJobs_Area.Min.x) / m_ScanningJobs_Area.SizeVoxel + 0.5f) + 1;
			m_ScanningJobs_Area.NbCellY = ((m_ScanningJobs_Area.Max.z - m_ScanningJobs_Area.Min.z) / m_ScanningJobs_Area.SizeVoxel + 0.5f) + 1;
			m_ScanningJobs_Area.NbCellH = ((m_ScanningJobs_Area.Max.y - m_ScanningJobs_Area.Min.y) / m_ScanningJobs_Area.SizeVoxel + 0.5f) + 1;

			// Recompute Max (Have to be in last Cell).
			m_ScanningJobs_Area.Max.x = m_ScanningJobs_Area.Min.x + (m_ScanningJobs_Area.NbCellX * m_ScanningJobs_Area.SizeVoxel) - 0.001f;
			m_ScanningJobs_Area.Max.y = m_ScanningJobs_Area.Min.y + (m_ScanningJobs_Area.NbCellH * m_ScanningJobs_Area.SizeVoxel) - 0.001f;
			m_ScanningJobs_Area.Max.z = m_ScanningJobs_Area.Min.z + (m_ScanningJobs_Area.NbCellY * m_ScanningJobs_Area.SizeVoxel) - 0.001f;

			// Recompute Center.
			m_ScanningJobs_Area.Center = (MeshMin + MeshMax) * 0.5f;
		
			m_ScanningJobs_Surfels.Init(m_ScanningJobs_Area);
			m_pSurfaceInfosV3->ReInitScan(m_ScanningJobs_Area,m_YGround,m_YCeiling);

			// Set Valid Zone.
			m_pSurfaceSR->SetValidZone(m_ScanningJobs_Area.Min,m_ScanningJobs_Area.Max);
			m_ScanningJobs_RefreshMode = HMapMeshInfos3D::ScanModeComplete;

			// Force Refresh.
			RefreshMeshFromSurfaceSR();
			m_TheMeshSR.RemoveIsolatedFaces(30);					// Have to be done now because of Normal futur computing.
			m_pSurfaceSR->ThreadSafe_TotalRefreshBlindMapFromSR();	// Because not done in the Paint Task (pause).
		}
	}
	else if (!m_ScanningRefineMode)
	{
		// Compute Valide Zone.
		S32		NbFaces = m_TheMeshSR.m_TabQuad.GetSize();
		Vec3f	MeshMin = m_ScanningJobs_Area.Center;
		Vec3f	MeshMax = m_ScanningJobs_Area.Center;

		for (S32 i = 0; i<NbFaces ; i++)
		{
			Playspace_Mesh::Face &CurFace = m_TheMeshSR.m_TabQuad[i];
			if (CurFace.IsPaintMode != 2)	// Keep Only paint faces.
				continue;
			Vec3f &Pos0 = m_TheMeshSR.m_TabPoints[CurFace.TabPoints[0]];
			MeshMin = Min(MeshMin, Pos0);
			MeshMax = Max(MeshMax, Pos0);
			Vec3f &Pos1 = m_TheMeshSR.m_TabPoints[CurFace.TabPoints[1]];
			MeshMin = Min(MeshMin, Pos1);
			MeshMax = Max(MeshMax, Pos1);
			Vec3f &Pos2 = m_TheMeshSR.m_TabPoints[CurFace.TabPoints[2]];
			MeshMin = Min(MeshMin, Pos2);
			MeshMax = Max(MeshMax, Pos2);
		}

		Vec3f MeshSize = MeshMax-MeshMin;
		if (MeshSize.x > m_OptimalZoneSize)
		{
			if ((MeshMin.x + 0.01f) < m_ScanningJobs_MeshMin.x)
				MeshMin.x = m_ScanningJobs_MeshMin.x;
			else if (MeshMin.x > (m_vOriginalPlayerPos.x - 1.f))
				MeshMin.x = m_vOriginalPlayerPos.x - 1.f;

			if ((MeshMax.x - 0.01f) > m_ScanningJobs_MeshMax.x)
				MeshMax.x = m_ScanningJobs_MeshMax.x;
			else if (MeshMax.x < (m_vOriginalPlayerPos.x + 1.f))
				MeshMax.x = m_vOriginalPlayerPos.x + 1.f;
		}
		if (MeshSize.z > m_OptimalZoneSize)
		{
			if ((MeshMin.z + 0.01f) < m_ScanningJobs_MeshMin.z)
				MeshMin.z = m_ScanningJobs_MeshMin.z;
			else if (MeshMin.z > (m_vOriginalPlayerPos.z - 1.f))
				MeshMin.z = m_vOriginalPlayerPos.z - 1.f;

			if ((MeshMax.z - 0.01f) > m_ScanningJobs_MeshMax.z)
				MeshMax.z = m_ScanningJobs_MeshMax.z;
			else if (MeshMax.z < (m_vOriginalPlayerPos.z + 1.f))
				MeshMax.z = m_vOriginalPlayerPos.z + 1.f;
		}

		if (   ((m_ScanningJobs_MeshMin-MeshMin).GetNorm() > 0.01f)
			|| ((m_ScanningJobs_MeshMax-MeshMax).GetNorm() > 0.01f)
			)
		{
			// MOVE !
			m_ScanningJobs_MeshMin = MeshMin;
			m_ScanningJobs_MeshMax = MeshMax;

			Vec3f Center = (MeshMin+MeshMax) * 0.5f;
			Float	HalfZone = m_OptimalZoneSize * 0.5f;

			m_ScanningJobs_Area.Min.x = Center.x - HalfZone;
			m_ScanningJobs_Area.Min.z = Center.z - HalfZone;
			SnapToVoxelGrid(m_ScanningJobs_Area.Min);

			m_ScanningJobs_Area.Max.x = Center.x + HalfZone;
			m_ScanningJobs_Area.Max.z = Center.z + HalfZone;
			SnapToVoxelGrid(m_ScanningJobs_Area.Max);

			// GetFiltered height.
			m_ScanningJobs_Area.NbCellX = ((m_ScanningJobs_Area.Max.x - m_ScanningJobs_Area.Min.x) / m_ScanningJobs_Area.SizeVoxel +0.5f) + 1;
			m_ScanningJobs_Area.NbCellY = ((m_ScanningJobs_Area.Max.z - m_ScanningJobs_Area.Min.z) / m_ScanningJobs_Area.SizeVoxel +0.5f) + 1;

			// Recompute Max (Have to be in last Cell).
			m_ScanningJobs_Area.Max.x = m_ScanningJobs_Area.Min.x + (m_ScanningJobs_Area.NbCellX * m_ScanningJobs_Area.SizeVoxel) - 0.001f;
			m_ScanningJobs_Area.Max.z = m_ScanningJobs_Area.Min.z + (m_ScanningJobs_Area.NbCellY * m_ScanningJobs_Area.SizeVoxel) - 0.001f;

			m_ScanningJobs_Area.Center = (m_ScanningJobs_Area.Max+m_ScanningJobs_Area.Min) * 0.5f;

			Vec3f ZoneMin = m_ScanningJobs_Area.Min;
			Vec3f ZoneMax = m_ScanningJobs_Area.Max;
			ZoneMin.y = m_vOriginalPlayerPos.y - 3.f;
			ZoneMax.y = m_vOriginalPlayerPos.y + 4.f;

			m_pSurfaceSR->SetValidZone(ZoneMin, ZoneMax);
		}
	}
	ABORT_USU_IF_NEEDED;

/*	if (StaticMoveDelta.GetNorm2() > 0.01f)
	{
		SnapToVoxelGrid(StaticMoveDelta);
		m_ScanningJobs_Area.Center += StaticMoveDelta;
		m_ScanningJobs_Area.Min +=StaticMoveDelta;
		m_ScanningJobs_Area.Max +=StaticMoveDelta;
		StaticMoveDelta = VEC3F_NULL;
	}*/

	// Update Ceiling and Eye
	Float t1 = GetAbsoluteTime();

	if (!HaveRequestedFinish)
		RefreshGroundAndCeilingMesh( FALSE);
	// Do The Process
	Float t2 = GetAbsoluteTime();
	m_TimeInState += _dTime;

	// If Ground and Ceiling OK...
	if (!HaveRequestedFinish && !m_ScanningRefineMode)
	{
		Bool HaveToAlign = FALSE;
		if (((m_YCeilingSurface >= 0.8f) || m_IsVirtualCeiling)
			&& (m_YGroundSurface >= 0.8f)
			)
		{
			// Never Refresh Zone ON FINAL V3 System.
			// RefreshZone(_hGame, m_AlignXAxis, FALSE);
			m_ScanningJobs_Area.Min.y = m_YGround - MARGIN_GROUND_CEILING;
			SnapComposantToVoxelGrid(m_ScanningJobs_Area.Min,1);
			m_ScanningJobs_Area.Max.y = m_YCeiling + MARGIN_GROUND_CEILING;
			SnapComposantToVoxelGrid(m_ScanningJobs_Area.Max,1);

			// New Align.
			Vec3f	AxisAlign;
			Float	Surface = m_pAlignMgr.GetAlignAxis_Mesh(m_TheMeshSR, m_YGround, m_YCeiling, AxisAlign, FALSE);
			if ((Surface > 0.2f) && (Surface > (m_AlignSurface * 0.9f)))
			{
				Float AlignDiff = Abs(m_AlignXAxis * AxisAlign);
				Vec3f AxisAlign90 = Quat(M_PI_2, VEC3F_UP) * AxisAlign;
				Float AlignDiff90 = Abs(m_AlignXAxis * AxisAlign90);

				if (	(!m_bHaveAlignToWall)
					||	((AlignDiff < 0.9998f) && (AlignDiff90 < 0.9998f))
					)
				{
					OUTPUT_Z("ALIGN NEW AXIS %.3f surf => Surface", Surface);
					HaveToAlign = TRUE;
					m_bHaveAlignToWall = TRUE;
					m_AlignXAxis = AxisAlign;
					m_AlignSurface = Surface;
					m_ScanningJobs_WantedState = PSI_State_Scanning_Align;
				}
			}
		}
	}
	ABORT_USU_IF_NEEDED;

	// And Scan now...
	Float t3 = GetAbsoluteTime();
	m_TheMeshSR.PlanarFilter_AccurateNew();

	// RefreshCenter.
	m_ScanningJobs_Area.Center = (m_ScanningJobs_Area.Min + m_ScanningJobs_Area.Max) * 0.5f;
	m_ScanningJobs_Area.Center.y = m_YGround;

	m_ScanningJobs_Area.Min.y = m_YGround - MARGIN_GROUND_CEILING;
	SnapComposantToVoxelGrid(m_ScanningJobs_Area.Min,1);

	m_ScanningJobs_Area.Max.y = m_YCeiling + MARGIN_GROUND_CEILING;
	SnapComposantToVoxelGrid(m_ScanningJobs_Area.Max,1);

	ABORT_USU_IF_NEEDED;

	// Do Scan.
	Float t4 = GetAbsoluteTime();
	m_pSurfaceInfosV3->m_IsInRayMode = FALSE;
	if (!m_pSurfaceInfosV3->m_MapMesh3D.DoScan(m_ScanningJobs_Area,m_YGround,m_YCeiling,m_ScanningJobs_RefreshMode,m_pSurfaceSR,&m_TheMeshSR,_Mesh))
	{
		Float t5 = GetAbsoluteTime();
#ifdef DRAW_TIME_SCAN
		MESSAGE_Z("Async Scan V3 : Refresh Mesh %.3f %.3f %.3f",t0b - t0,t0c - t0b,t1 - t0c);
		MESSAGE_Z("Async Scan V3 : Ceil/Grd/Eye %.3f", t2 - t1);
		MESSAGE_Z("Async Scan V3 : Align %.3f", t3 - t2);
		MESSAGE_Z("Async Scan V3 : Filter %.3f", t4 - t3);
		MESSAGE_Z("Async Scan V3 : DoScan %.3f", t5 - t4);
		MESSAGE_Z("Async Scan V3 : Aborted...");
#endif
		return FALSE;
	}
	ABORT_USU_IF_NEEDED;

	if (m_ScanningJobs_RefreshMode == HMapMeshInfos3D::ScanModeIncrementalReInit)
		m_ScanningJobs_RefreshMode = HMapMeshInfos3D::ScanModeIncremental;

	Float t5 = GetAbsoluteTime();
	if (!HaveRequestedFinish)
	{
		_Surfels.Init(m_ScanningJobs_Area);
		_Surfels.CreateSurfelsFromMesh_fast(_Mesh);
	}
	ABORT_USU_IF_NEEDED;

	Float t6 = GetAbsoluteTime();
	m_pSurfaceSR->RefreshMergePaintToMesh(_Mesh,0.1f);
	if (!HaveRequestedFinish)
	{
		m_pSurfaceSR->SuppressUnpaintBorder(_Mesh);
	}
	ABORT_USU_IF_NEEDED;

	Float t7 = GetAbsoluteTime();
	if (!HaveRequestedFinish)
	{
		_Mesh.ComputeStats(DegToRad(20.f), m_YGround, m_YCeiling, m_YGround + 1.f, m_ScanningJobs_Area.Min, m_ScanningJobs_Area.Max, m_ScanningJobsStats.MeshStats);
		MESSAGE_Z("UnifiedScanningUpdate %.3f %.3f", m_ScanningJobsStats.MeshStats.HorizSurface, m_ScanningJobsStats.MeshStats.TotalSurface);
	}
	else
	{
		// All face are painted with minimal quality.
		S32		NbFaces = _Mesh.m_TabQuad.GetSize();
		for (S32 i = 0; i<NbFaces ; i++)
		{
			Playspace_Mesh::Face &CurFace = _Mesh.m_TabQuad[i];
			if (!CurFace.IsPaintMode || !CurFace.IsSeenQuality)
			{
				CurFace.IsPaintMode = 2;
				CurFace.IsSeenQuality = 1;
				continue;
			}
		}
	}

	Float t8 = GetAbsoluteTime();

#ifdef DRAW_TIME_SCAN
MESSAGE_Z("Async Scan V3 : Refresh Mesh %.3f %.3f %.3f",t0b - t0,t0c - t0b,t1 - t0c);
MESSAGE_Z("Async Scan V3 : Ceil/Grd/Eye %.3f", t2 - t1);
MESSAGE_Z("Async Scan V3 : Align %.3f", t3 - t2);
MESSAGE_Z("Async Scan V3 : Filter %.3f", t4 - t3);
MESSAGE_Z("Async Scan V3 : DoScan %.3f", t5 - t4);
MESSAGE_Z("Async Scan V3 : Do Surfel %.3f", t6 - t5);
MESSAGE_Z("Async Scan V3 : Refresh Paint %.3f", t7 - t6);
MESSAGE_Z("Async Scan V3 : Stats %.3f", t8 - t7);
#endif

	// End of Scan Process : End request ?
	if (HaveRequestedFinish)
	{
		FinalizeScanV3Mesh(&_Mesh,&_Surfels,&m_ScanningJobs_Area);
		m_ScanningJobs_WantedState = PSI_State_Scanning_Finish;
	}
		
	return TRUE;
}


/**************************************************************************/

void	PlaySpaceInfos_W::RequestFinishUnifiedScanning(Bool _Force)
{
	if (_Force)
		m_ScanningRequestStop = 2;
	else
		m_ScanningRequestStop = 1;
}

/**************************************************************************/

void	PlaySpaceInfos_W::GetUnifiedScanV3Stats(PlaySpaceInfos_ScanningStats &_Stats)
{
	_Stats = m_ScanningJobsLastStats;
}

/**************************************************************************/

void	PlaySpaceInfos_W::ComputeMeshStats(Playspace_Mesh& _Mesh, PlaySpaceMesh_Stats& _outStats)
{
	_Mesh.ComputeStats(DegToRad(20.f), m_YGround, m_YCeiling, m_YGround + 1.f, m_vMinAvailable, m_vMaxAvailable, _outStats);
}

/**************************************************************************/

void	PlaySpaceInfos_W::UpdatePlaySpace(UnderstandingMgr_W *_pMyMgr,Float _dTime)
{
	m_pUnderstandingMgr = _pMyMgr;
	UpdateTools(FALSE,FALSE,FALSE);
}

/**************************************************************************/

void	PlaySpaceInfos_W::SetState(PSI_State	_CurState)
{
	if (m_CurState != _CurState)
	{
		m_TimeInState = 0.f;
		m_CurState = _CurState;
	}
}

/**************************************************************************/

void	PlaySpaceInfos_W::SetUpdateMode(PSI_UpdateMode _CurUpdateMode)
{
	m_CurUpdateMode = _CurUpdateMode;
}

/**************************************************************************/

PlaySpaceInfos_W &PlaySpaceInfos_W::operator=(const PlaySpaceInfos_W &f)

{
	EXCEPTIONC_Z(0,"PAS LE DROIT !!!!  mettre en place un clone ou utilisé uniquement le pointeur");
	return *this;
}

/**************************************************************************/

Bool	PlaySpaceInfos_W::IsValid(Float _MinConfidence) const
{
	if (m_CurState == PSI_State_NonInitialized)
		return FALSE;
	if (m_CurState == PSI_State_Loading)
		return FALSE;
	Float SizeX = m_vMaxAvailable.x - m_vMinAvailable.x;
	Float SizeZ = m_vMaxAvailable.z - m_vMinAvailable.z;
	return ((m_fConfidence >= _MinConfidence) && (SizeX>1.f) && (SizeZ>1.f));
}

/**************************************************************************/

void  PlaySpaceInfos_W::ComputeRoomMinMax(Vec3f &_Min,Vec3f &_Max)
{
	_Min = m_vMinAvailable;
	_Max = m_vMaxAvailable;

	_Min.y = m_YGround;
	_Max.y = m_YCeiling;

	if ((g_TypeScan & PSI_SCAN_NEW) && m_pSurfaceInfosV3 && !m_pSurfaceInfosV3->m_Surfels.IsEmpty())
	{
		// Surfels Check.
		_Min.x = m_vMaxAvailable.x;
		_Min.z = m_vMaxAvailable.z;

		_Max.x = m_vMinAvailable.x;
		_Max.z = m_vMinAvailable.z;

		Float HalfSurfel = m_SizeVoxel * 0.5f;
		S32 NbCells = m_pSurfaceInfosV3->m_Surfels.m_CellBoard.GetSize();
		for (S32 i=0 ; i<NbCells ; i++)
		{
			PlaySpace_CellInfos *pCell = &m_pSurfaceInfosV3->m_Surfels.m_CellBoard[i];
			PlaySpace_Surfel *pSurfel = pCell->pFirst;
			while(pSurfel)
			{
				if (	(pSurfel->iDir == PlaySpace_Surfel::SURFDIR_UP)
					&&	((pSurfel->Flags & (PlaySpace_Surfel::SURFFLAG_BADSURFEL|PlaySpace_Surfel::SURFFLAG_NOGAMEPLAY|PlaySpace_Surfel::SURFFLAG_VIRTUAL)) == 0)
					)
				{
					_Min.x = Min(_Min.x,pCell->fCornerX);
					_Min.z = Min(_Min.z,pCell->fCornerZ);

					_Max.x = Max(_Max.x,pCell->fCornerX);
					_Max.z = Max(_Max.z,pCell->fCornerZ);
					break;
				}
				pSurfel = pSurfel->pNext;
			}
		}

		_Max.x +=  m_SizeVoxel;
		_Max.z +=  m_SizeVoxel;

		if (_Min.x > _Max.x)
		{
			_Min.x = m_vMinAvailable.x;
			_Max.x = m_vMaxAvailable.x;
		}

		if (_Min.z > _Max.z)
		{
			_Min.z = m_vMinAvailable.z;
			_Max.z = m_vMaxAvailable.z;
		}
	}
}

/**************************************************************************/

void	PlaySpaceInfos_W::ComputeMinMax()
{
	m_vMinAvailable = m_vNonAlignedZone[0];
	m_vMaxAvailable = m_vNonAlignedZone[0];
	for (S32 i=1 ; i<4 ; i++)
	{
		m_vMinAvailable.x = Min(m_vMinAvailable.x , m_vNonAlignedZone[i].x);
		m_vMinAvailable.z = Min(m_vMinAvailable.z , m_vNonAlignedZone[i].z);

		m_vMaxAvailable.x = Max(m_vMaxAvailable.x , m_vNonAlignedZone[i].x);
		m_vMaxAvailable.z = Max(m_vMaxAvailable.z , m_vNonAlignedZone[i].z);
	}

	if (g_TypeScan & PSI_SCAN_NEW)
	{
		m_vMinAvailable.y = m_YGround - MARGIN_GROUND_CEILING;
		m_vMaxAvailable.y = m_YCeiling + MARGIN_GROUND_CEILING;
	}
	else
	{
		// Avoid Blob pb :(
		m_vMinAvailable.y = m_YGround;
		m_vMaxAvailable.y = m_YCeiling;
	}

	m_vCenter = (m_vMinAvailable + m_vMaxAvailable) * 0.5f;
	m_vCenter.y = m_YGround;
}

/**************************************************************************/

Bool	PlaySpaceInfos_W::GetBestCentroidPosition(Vec3f& result, S32 granularity)
{
	if (!m_pSurfaceInfosV3 || granularity <= 0 || (m_NbCellX < granularity) || (m_NbCellY < granularity))
		return FALSE;

	// Init
	HS32DA tab2D;
	S32 sizeX = m_NbCellX / granularity;
	S32 sizeY = m_NbCellY / granularity;
	Float sizeVox = m_SizeVoxel * granularity;
	S32 cx = sizeX - 1;
	S32 cz = sizeY - 1;
	Float invSizeVoxel = 1.0f / sizeVox;

	S32 size = sizeX * sizeY;		// m_NbCellX * m_NbCellY;
	tab2D.SetSize(size);
	S32 *pTab2D = tab2D.GetArrayPtr();
	memset(pTab2D, -1, tab2D.GetSize() * sizeof(S32));
Float t0 = GetAbsoluteTime();
	// Add Seen Face Blocs.
	S32 nb = 0;
	Vec3f middlePos = VEC3F_NULL;
	S32* pCurCell = pTab2D;
	for (S32 z = 0; z < sizeY; z++)
	{
		S32 z2 = z * granularity;
		for (S32 x = 0; x < sizeX; x++)
		{
			S32 x2 = x * granularity;
			Bool empty = FALSE;
			for (S32 i = 0; i < granularity; i++)
			{
				for (S32 j = 0; j < granularity; j++)
				{
					if (((x2 + i) < m_NbCellX) && ((z2 + j) < m_NbCellY))
					{
						PlaySpace_CellInfos	*pCell = m_pSurfaceInfosV3->m_Surfels.GetCell(x2 + i, z2 + j);
						if (!pCell || !pCell->pFirst)
							empty = TRUE;
					}
				}
			}
			if (!empty)
			{
				*pCurCell = 0;		// init with 0
				Vec3f curPos = Vec3f((float(x) + 0.5f) * sizeVox, 1.0f, (float(z) + 0.5f) * sizeVox);
				curPos += m_vMinAvailable;
				middlePos += curPos;
				nb++;
			}
			pCurCell++;
		}
	}
	if (nb)
		middlePos /= Float(nb);
Float t1 = GetAbsoluteTime();
	////////////////////////////////////////////////////////// Compute distance to border
	pCurCell = pTab2D;
	for (S32 z = 0; z < sizeY; z++)
	{
		for (S32 x = 0; x < sizeX; x++)
		{
			if ((*pCurCell == 0) && ((x == 0) || (x == cx) || (z == 0) || (z == cz) || (pCurCell[-1] == -1) || (pCurCell[1] == -1) || (pCurCell[-sizeX] == -1) || (pCurCell[sizeX] == -1)))
				*pCurCell = 0x10000;

			pCurCell++;
		}
	}
	S32 curVal = 0x10000;
	Bool somethingToDo = TRUE;
	while (somethingToDo)
	{
		somethingToDo = FALSE;
		pCurCell = pTab2D;
		for (S32 z = 0; z < sizeY; z++)
		{
			for (S32 x = 0; x < sizeX; x++)
			{
				if ((*pCurCell == 0) && ((pCurCell[-1] == curVal) || (pCurCell[1] == curVal) || (pCurCell[-sizeX] == curVal) || (pCurCell[sizeX] == curVal)))
				{
					*pCurCell = curVal + 0x10000;
					somethingToDo = TRUE;
				}
				pCurCell++;
			}
		}
		curVal += 0x10000;
	}
Float t2 = GetAbsoluteTime();
	////////////////////////////////////////////////////////// Compute corners visibility
	S32 nbCorners = 0;
	S32 max = 0;
	pCurCell = pTab2D;
	for (S32 z = 0; z < sizeY; z++)
	{
		U8 zmask = 0;
		if (z == 0)
			zmask |= 4;
		else if (z == cz)
			zmask |= 8;

		for (S32 x = 0; x < sizeX; x++)
		{
			if (*pCurCell >= 0)
			{
				U8 xmask = zmask;
				if (x == 0)
					xmask |= 1;
				else if (x == cx)
					xmask |= 2;

				U8 mask = xmask;
				if (!(mask & 1) && (pCurCell[-1] < 0))
					mask |= 1;
				else if (!(mask & 2) && (pCurCell[1] < 0))
					mask |= 2;

				if (!(mask & 4) && (pCurCell[-sizeX] < 0))
					mask |= 4;
				else if (!(mask & 8) && (pCurCell[sizeX] < 0))
					mask |= 8;

				if ((mask != 5) && (mask != 9) && (mask != 6) && (mask != 10))
				{
					pCurCell++;
					continue;
				}
				nbCorners++;
				S32 x1 = ((xmask & 1) ? cx : 0);
				S32 x2 = ((xmask & 2) ? 0 : cx);
				S32 tx1_0 = (((xmask == 1) || (xmask == 9)) ? cx : 0);
				S32 tx1_1 = (((xmask == 1) || (xmask == 5)) ? cx : 0);
				S32 tx2_0 = (((xmask == 2) || (xmask == 10)) ? cx : 0);
				S32 tx2_1 = (((xmask == 2) || (xmask == 6)) ? cx : 0);
				S32 incr = (((xmask == 0) || (xmask == 4) || (xmask == 8)) ? cx : 1);
				for (S32 dz = 0; dz < sizeY; dz++)
				{
					S32 xBeg = x1, xEnd = x2, xIncr = incr;
					if (dz == 0)
					{
						xBeg ^= tx1_0;
						xEnd ^= tx2_0;
						if ((xmask == 0) || (xmask == 8))
							xIncr = 1;
					}
					else if (dz == cz)
					{
						xBeg ^= tx1_1;
						xEnd ^= tx2_1;
						if ((xmask == 0) || (xmask == 4))
							xIncr = 1;
					}
					for (S32 dx = xBeg; dx <= xEnd; dx += xIncr)
					{
						Bresenham2DInt_Z Bres;
						Bres.InitSead(x, z, dx, dz);
						S32	X, Y;
						while (Bres.GetNextPointSead(X, Y))
						{
							if ((X >= 0) && (X <= cx) && (Y >= 0) && (Y <= cz))
							{
								S32& ptr = pTab2D[(Y * sizeX) + X];
								if (ptr < 0)
									break;

								if (!(ptr & 0x40000000))		// if bit 30 not set
								{
									ptr += 0x40000001;
									if ((ptr & 0xFFFF) > max)
										max = ptr & 0xFFFF;
								}
							}
						}
					}
				}
				S32* pCurCell2 = pTab2D;
				for (S32 i = 0; i < size; i++)
				{
					if (*pCurCell2 > 0)
						*pCurCell2 &= 0x3FFFFFFF;		// reset bit 30 if bit 31 not set

					pCurCell2++;
				}
			}
			pCurCell++;
		}
	}
	S32 max1 = max, max2 = max >> 1;
	if (max > 3)
		max1 = max - 1;
	if (max2 < 2)
		max2 = 2;
Float t3 = GetAbsoluteTime();
	//////////////////////////////////////////////////////////
	nb = 0;
	Vec3f centerPos = VEC3F_NULL;
	pCurCell = pTab2D;
	for (S32 z = 0; z < sizeY; z++)
	{
		for (S32 x = 0; x < sizeX; x++)
		{
			if ((*pCurCell & 0xFFFF) <= max1)
			{
				Vec3f curPos = Vec3f((float(x) + 0.5f) * sizeVox, 1.0f, (float(z) + 0.5f) * sizeVox);
				curPos += m_vMinAvailable;
				centerPos += curPos;
				nb++;
				//DRAW_DEBUG_SPHERE3D(pos, COLOR_GREEN * 0.99f, 0.05f, .displayDuration(2.f));
			}
			pCurCell++;
		}
	}
	if (nb)
		centerPos /= Float(nb);

	Vec3f bestPos = (middlePos + centerPos) * 0.5f;
	S32 centerX = FLOORINT(((bestPos.x - m_vMinAvailable.x) * invSizeVoxel) + 0.5f);
	S32 centerY = FLOORINT(((bestPos.z - m_vMinAvailable.z) * invSizeVoxel) + 0.5f);
	S32* ptr = pTab2D + ((centerY * sizeX) + centerX);
	if ((centerX >= 0) && (centerX <= cx) && (centerY >= 0) && (centerY <= cz) && (*ptr >= 0) && ((*ptr & 0xFFFF) >= max2) && ((*ptr >> 16) > 10))
	{
		//DRAW_DEBUG_SPHERE3D(bestPos, COLOR_BLUE, 0.1f, .displayDuration(5.f));
		result = bestPos;
		return TRUE;
	}
	Bool ok = FALSE;
	Vec3f pos;
	Float minDist = 1e8f;
	pCurCell = pTab2D;
	for (S32 z = 0; z < sizeY; z++)
	{
		for (S32 x = 0; x < sizeX; x++)
		{
			if ((*pCurCell >= 0) && ((*pCurCell & 0xFFFF) >= max2) && ((*pCurCell >> 16) > 10))
			{
				Vec3f curPos = Vec3f((float(x) + 0.5f) * sizeVox, 1.0f, (float(z) + 0.5f) * sizeVox);
				curPos += m_vMinAvailable;
				Float dist = (curPos - bestPos).GetNorm2();
				if (dist < minDist)
				{
					pos = curPos;
					minDist = dist;
					ok = TRUE;
				}
			}
			pCurCell++;
		}
	}
	if (!ok)
	{
		pCurCell = pTab2D;
		for (S32 z = 0; z < sizeY; z++)
		{
			for (S32 x = 0; x < sizeX; x++)
			{
				if ((*pCurCell >= 0) && ((*pCurCell & 0xFFFF) >= max2) && ((*pCurCell >> 16) > 5))
				{
					Vec3f curPos = Vec3f((float(x) + 0.5f) * sizeVox, 1.0f, (float(z) + 0.5f) * sizeVox);
					curPos += m_vMinAvailable;
					Float dist = (curPos - bestPos).GetNorm2();
					if (dist < minDist)
					{
						pos = curPos;
						minDist = dist;
						ok = TRUE;
					}
				}
				pCurCell++;
			}
		}
	}
	if (ok)
	{
		result = pos;
		//DRAW_DEBUG_SPHERE3D(pos, COLOR_RED, 0.1f, .displayDuration(5.f));
	}
Float t4 = GetAbsoluteTime();
//OUTPUT_Z("CENTROID !!! %.3f %.3f %.3f %.3f",t4-t3,t3-t2,t2-t1,t1-t0);
	return ok;
}

/**************************************************************************/

void	PlaySpaceInfos_W::RefreshEye(Float _dTime,Bool _DrawIt)
{
	// Eye is camera Pos...
	Segment_Z	SegView;
	Util_L::GetViewSegment(&SegView);

	Vec3f	PosEyes = SegView.Org;

	if ((m_YEyesAccuNb <= 0.5f) || (Abs(PosEyes.y - (m_YEyesAccu/m_YEyesAccuNb)) >= 0.015f))
	{
		// Init or Reset
		m_YEyesAccu = PosEyes.y;
		m_YEyesAccuTime = _dTime;
		m_YEyesAccuNb = 1.f;
	}
	else
	{
		// Accumulator.
		m_YEyesAccu += PosEyes.y;
		m_YEyesAccuTime += _dTime;
		m_YEyesAccuNb += 1.f;

		Float CurY = m_YEyesAccu / m_YEyesAccuNb;

		if ((m_YEyesConfidence < 30.f) && (m_YEyesAccuNb > m_YEyesConfidence))
		{
			// If nothing really good => set a value...
			m_YEyes = CurY;
			m_YEyesConfidence = m_YEyesAccuNb;
		}
		else if ((m_YEyesAccuTime > 1.f) && (m_YEyesAccuNb > 30) && (CurY > m_YEyes))
		{
			// If Upper and good => set a value.
			m_YEyes = CurY;
			m_YEyesConfidence = m_YEyesAccuNb;
		}
	}
	if (_DrawIt && (m_YEyesConfidence > 1.f))
	{
		PosEyes.y = m_YEyes;

		Vec3f Dir = SegView.Dir;
		Dir.CHNormalize();

		PosEyes += Dir * 1.5f;
		DRAW_DEBUG_SPHERE3D(PosEyes,COLOR_YELLOW,0.15f);
	}
}

/**************************************************************************/

void	PlaySpaceInfos_W::RefreshGroundAndCeiling(Bool _IsInOnePassMode, Bool _DrawIt)
{
	RefreshGroundAndCeilingMesh( _IsInOnePassMode, _DrawIt);
}

/**************************************************************************/

Bool	PlaySpaceInfos_W::InitProcess(const Vec3f& _vPlayerPosition,const Vec3f& _vPlayerFrontDir,Float _fSearchDist,Float _OptimalSize)
{
	// Init Some infos.

	m_SizeVoxel = DEFAULT_SIZE_VOXEL;
//	if(m_pSurfaceReco && !(g_TypeScan & PSI_SCAN_MESH))
//		m_SizeVoxel = m_pSurfaceReco->GetParams().VoxelOctreeInitParams.fVoxelSize;

	// New PlaySpace.
	Reset();
	SetState(PSI_State_Initialized);
	m_vOriginalPlayerPos = _vPlayerPosition;
	m_vOriginalPlayerDir = _vPlayerFrontDir;
	m_vOriginalPlayerDir.CHNormalize();

	m_vCenter = _vPlayerPosition;//vCenter;
	m_vMinAvailable = m_vOriginalPlayerPos;//vCenter;
	m_vMaxAvailable = m_vOriginalPlayerPos;//vCenter;

	m_OptimalZoneSize = _OptimalSize;

	// Align Manager Reinit.
	m_pAlignMgr.Flush();
	if ((g_TypeScan & PSI_SCAN_NEW) && (g_TypeScan & PSI_SCAN_MESH))
		m_pAlignMgr.SetNbCheckNormal(8);	//Because new flow... en attendant mieux.
	m_AlignXAxis = VEC3F_LEFT;
	m_AlignSurface = 0.f;
	m_AlignTransfoId = 0;
	m_AlignTransfo = QUAT_NULL;
	m_bHaveAlignToWall = FALSE;

	// Alignement by default is set on Playspace center.
	m_AlignXAxis = Quat(VEC3F_FRONT, m_vOriginalPlayerDir) * VEC3F_LEFT;

	// If On PC => Flush the MeshSR.
	m_TheMeshSR.Flush(TRUE);

	// Flush SR Manager.
	m_pSurfaceSR->Flush(TRUE);

	// Flush Mesh
	m_ScanningJobs_Mesh.Flush(FALSE);
	m_ScanningJobs_Surfels.Flush(FALSE);
	if (m_pSurfaceInfosV3)
	{
		Delete_Z m_pSurfaceInfosV3;
		m_pSurfaceInfosV3 = NULL;
	}

	m_pSurfaceSR->SetSnapValue(m_SizeVoxel);

	// Flush Stats.
	m_ScanningJobsLastStats.Reset();
	m_ScanningJobs_RefreshMode = HMapMeshInfos3D::ScanModeIncremental;

	PlaySpaceInfos_W::g_SetNewAlign = TRUE;

	// Modify Center if V3
	if (g_TypeScan & PSI_SCAN_NEW)
		m_vCenter += m_vOriginalPlayerDir * (_fSearchDist * 0.5f * 0.7f);

	return TRUE;
}

/**************************************************************************/

Bool	PlaySpaceInfos_W::RefreshZone(const Vec3f& _AlignXAxis, Bool _DrawIt)
{
	return RefreshZoneMesh(_AlignXAxis, _DrawIt);
}
	
/**************************************************************************/

void	PlaySpaceInfos_W::AlignSurfaceReco(const Vec3f &_XAxis, Bool _NoThreadAlignVersion)
{
	Float	CosMaxAngle = Cos(DegToRad(1.f));
	Vec3f	StabilizedNormal = _XAxis;

	// rotate 90° until we find an axis close enough to the one the player expects
	// (ie. alongside the wall he's looking at)
	Vec3f vPlayerLeft = VEC3F_UP ^ m_vOriginalPlayerDir;
	vPlayerLeft.CHNormalize();
	Float fBestDot = StabilizedNormal * vPlayerLeft;
	Vec3f vRotated = _XAxis;
	for ( S32 i = 0 ; i < 3 ; i++ )
	{
		vRotated = Vec3f(-vRotated.z, vRotated.y, vRotated.x);
		Float fDot = vRotated * vPlayerLeft;
		if ( fDot > fBestDot )
		{
			fBestDot = fDot;
			StabilizedNormal = vRotated;
		}
	}

	Float CosAngle = Clamp(StabilizedNormal * VEC3F_LEFT,0.f,1.f);
	MESSAGE_Z("ALIGN WORLD => %f %f %f",RadToDeg(ACos(CosAngle)),Abs(StabilizedNormal.x ),Abs(StabilizedNormal.z ));

	//gData.InputMgr->FRigSetWorldSpaceXAxis(StabilizedNormal);
//	if(m_pSurfaceReco)
//		m_pSurfaceReco->SetWorldSpaceXAxis(StabilizedNormal);

	m_AlignTransfoId++;
	m_AlignTransfo = Quat(StabilizedNormal,VEC3F_LEFT);

	// HERE YOU HAVE TO Turn All Objects of your scene.
	ApplyAlignToAllScene(m_AlignTransfo);
#if 0
	Node_Z *pNodeSR = NULL;

	if (m_pSurfaceReco)
	{
		pNodeSR = _hGame->GetGameWorld()->GetNodeByName(Name_Z("VOXELOCTREE"));
//		EXCEPTIONC_Z(pNodeSR,"Unknow Node VOXELOCTREE!!!");
	}

	Node_Z *pNode = _hGame->GetGameWorld()->GetRoot(); 
	if (pNode)
	{
		pNode = pNode->GetHeadSon();
		while (pNode)
		{
			if (pNode!=pNodeSR)
			{
				Quat Rot = pNode->GetRotInWorld();
				Vec3f Trans = pNode->GetWorldTranslation();
				Rot = m_AlignTransfo * Rot;
				Trans = m_AlignTransfo * Trans;
				pNode->SetFromWorldTransRot(Trans,Rot);
				pNode->Update();
			}

			// Next.
			pNode = pNode->GetNext();
		}
	}

	// Cam.
	Node_Z	*pCamNode=gData.MainRdr->GetViewport(0).GetCamera();
	if (pCamNode)
	{
		Camera_Z	*pCamera=(Camera_Z *)pCamNode->GetObject();
		if (pCamera)
		{
			Vec3f TargetPos = pCamera->GetTarget();
			TargetPos = m_AlignTransfo * TargetPos;
			pCamera->SetTarget(TargetPos);
	
			pCamNode->Changed();
			pCamNode->Update();

//				pCamera->GetDir() = _Transfo * pCamera->GetDir();
		}
	}
#endif
	// Apply Transfo to Playspace.
	m_vCenter = m_AlignTransfo * m_vCenter;
	m_vOriginalPlayerPos = m_AlignTransfo * m_vOriginalPlayerPos;

	m_vOriginalPlayerDir = m_AlignTransfo * m_vOriginalPlayerDir;
	m_AlignXAxis = VEC3F_LEFT;

	if (_NoThreadAlignVersion)
	{
		m_vNonAlignedZone[0] = m_AlignTransfo * m_vNonAlignedZone[0];
		m_vNonAlignedZone[1] = m_AlignTransfo * m_vNonAlignedZone[1];
		m_vNonAlignedZone[2] = m_AlignTransfo * m_vNonAlignedZone[2];
		m_vNonAlignedZone[3] = m_AlignTransfo * m_vNonAlignedZone[3];

		ComputeMinMax();
	}
	else
	{
		Float DeltaX = m_vMaxAvailable.x - m_vMinAvailable.x;
		Float DeltaZ = m_vMaxAvailable.z - m_vMinAvailable.z;

		m_vMinAvailable.x = m_vCenter.x - (DeltaX*0.5f);
		m_vMinAvailable.z = m_vCenter.z - (DeltaZ*0.5f);
		SnapToVoxelGrid(m_vMinAvailable);

		// Recompute Max (Have to be in last Cell).
		m_vMaxAvailable.x = m_vMinAvailable.x + (m_NbCellX * m_SizeVoxel) - 0.001f;
		m_vMaxAvailable.y = m_vMinAvailable.y + (m_NbCellH * m_SizeVoxel) - 0.001f;
		m_vMaxAvailable.z = m_vMinAvailable.z + (m_NbCellY * m_SizeVoxel) - 0.001f;
	}

	// Apply Transfo to Local SR MEsh.
	m_pSurfaceSR->ApplyQuat(m_AlignTransfo,_NoThreadAlignVersion);
}

/**************************************************************************/

void PlaySpaceInfos_W::ForceAlign()
{
	if (m_CurState == PSI_State_Aligning)
	{
		Segment_Z	SegView;
		Util_L::GetViewSegment(&SegView);
		Vec3f	NewXAxis = SegView.Dir;
		NewXAxis.CHNormalize();
		m_AlignXAxis.x  = NewXAxis.z;
		m_AlignXAxis.z -= NewXAxis.x;
		m_AlignXAxis.y  = 0;

		SetState(PSI_State_Snaping);
	}
	else if (m_CurState == PSI_State_Scanning)
	{
		Segment_Z	SegView;
		Util_L::GetViewSegment(&SegView);
		Vec3f	NewXAxis = SegView.Dir;
		NewXAxis.CHNormalize();
		m_AlignXAxis.x = NewXAxis.z;
		m_AlignXAxis.z = -NewXAxis.x;
		m_AlignXAxis.y = 0;

		RefreshZone(m_AlignXAxis,FALSE);
		AlignSurfaceReco(m_AlignXAxis);
		InitScan();
	}
}

/**************************************************************************/

void PlaySpaceInfos_W::ForceReScan(Bool _IsSet)
{
	m_ForceRescanMode = _IsSet;
}

/**************************************************************************/

void PlaySpaceInfos_W::MovePlaySpace(Vec3f &_Delta)
{
//	StaticMoveDelta = _Delta;
//	return;
	m_vCenter += _Delta;
	m_vMinAvailable += _Delta;
	m_vMaxAvailable += _Delta;

	m_vNonAlignedZone[0] += _Delta;
	m_vNonAlignedZone[1] += _Delta;
	m_vNonAlignedZone[2] += _Delta;
	m_vNonAlignedZone[3] += _Delta;

	m_YCeiling += _Delta.y;
	m_YGround += _Delta.y;

	if (m_pSurfaceInfosV3)
	{
		PlaySpace_CellInfos *pCell = m_pSurfaceInfosV3->m_Surfels.m_CellBoard.GetArrayPtr();
		for (S32 i=0 ; i<m_NbCellX*m_NbCellY ; i++)
		{
			PlaySpace_Surfel *pSurfel = pCell->pFirst;
			while (pSurfel)
			{
				pSurfel->Point += _Delta;
				pSurfel = pSurfel->pNext;
			}

			// Next.
			pCell++;
		}
	}

	m_CurrentCRC = ComputeCRC();
}

/**************************************************************************/

void PlaySpaceInfos_W::SnapToVoxelGrid(Vec3f &_Pos)
{
	_Pos.x =  FLOORF(_Pos.x / m_SizeVoxel + 0.01f) * m_SizeVoxel;
	_Pos.y =  FLOORF(_Pos.y / m_SizeVoxel + 0.01f) * m_SizeVoxel;
	_Pos.z =  FLOORF(_Pos.z / m_SizeVoxel + 0.01f) * m_SizeVoxel;
}

/**************************************************************************/

void PlaySpaceInfos_W::SnapComposantToVoxelGrid(Vec3f &_Pos,S32 _Composante)
{
	Vec3f SnapPos = _Pos;
	SnapToVoxelGrid(SnapPos);
	_Pos[_Composante] = SnapPos[_Composante];
}

/**************************************************************************/

void	PlaySpaceInfos_W::VerifySnapToVoxelGrid()
{
	S32 x = FLOORINT(m_vMinAvailable.x / m_SizeVoxel + 0.05f);
	S32 y = FLOORINT(m_vMinAvailable.y / m_SizeVoxel + 0.05f);
	S32 z = FLOORINT(m_vMinAvailable.z / m_SizeVoxel + 0.05f);

	Vec3f	RealPos(x * DEFAULT_SIZE_VOXEL,y * DEFAULT_SIZE_VOXEL,z * DEFAULT_SIZE_VOXEL);
	EXCEPTIONC_Z(Abs(RealPos.x-m_vMinAvailable.x) < 0.05f,"LAAAAAAAAAAAAAAAA");
	EXCEPTIONC_Z(Abs(RealPos.z-m_vMinAvailable.z) < 0.05f,"LAAAAAAAAAAAAAAAA");
}

/**************************************************************************/

Bool	PlaySpaceInfos_W::InitScan()
{
	// Init Some infos.
	m_TimeInState = 0.f;
	m_SizeVoxel = DEFAULT_SIZE_VOXEL;
//	if (m_pSurfaceReco && !(g_TypeScan & PSI_SCAN_MESH))
//		m_SizeVoxel = m_pSurfaceReco->GetParams().VoxelOctreeInitParams.fVoxelSize;

	// Snap to grid.
	SnapToVoxelGrid(m_vMinAvailable);
	SnapToVoxelGrid(m_vMaxAvailable);

	// GetFiltered height.
	m_NbCellX = ((m_vMaxAvailable.x - m_vMinAvailable.x) / m_SizeVoxel + 0.5f);
	m_NbCellY = ((m_vMaxAvailable.z - m_vMinAvailable.z) / m_SizeVoxel + 0.5f);
	m_NbCellH = ((m_vMaxAvailable.y - m_vMinAvailable.y) / m_SizeVoxel + 0.5f);

	// Recompute Max (Have to be in last Cell).
	m_vMaxAvailable.x = m_vMinAvailable.x + (m_NbCellX * m_SizeVoxel) - 0.001f;
	m_vMaxAvailable.y = m_vMinAvailable.y + (m_NbCellH * m_SizeVoxel) - 0.001f;
	m_vMaxAvailable.z = m_vMinAvailable.z + (m_NbCellY * m_SizeVoxel) - 0.001f;

	// V3 Init.
	if (g_TypeScan & PSI_SCAN_NEW)
	{
		// TRUE FIRST INIT.
		if (m_pSurfaceInfosV3)
		{
			Delete_Z m_pSurfaceInfosV3;
			m_pSurfaceInfosV3 = NULL;
		}

		PlaySpace_SurfaceInfosV3	*pNewScanV3 = New_Z PlaySpace_SurfaceInfosV3();

		pNewScanV3->m_Surfels.Init(m_vMinAvailable,m_SizeVoxel,m_NbCellX,m_NbCellY,m_NbCellH);
		pNewScanV3->ReInitScan(m_vMinAvailable,m_vMaxAvailable,m_YGround,m_YCeiling,m_SizeVoxel,m_NbCellX,m_NbCellH,m_NbCellY);

		m_pSurfaceInfosV3 = pNewScanV3;
	}

	return TRUE;
}

/**************************************************************************/

void	PlaySpaceInfos_W::RefineScan()
{
	m_ScanningRefineMode = TRUE;
	m_ScanningRequestStop = 0;

	m_ScanningJobs_Mesh.Flush(FALSE);
	m_ScanningJobs_Surfels.Flush(FALSE);

	// Reset Painting on Mesh.
	if ((g_TypeScan & PSI_SCAN_NEW) && (g_TypeScan & PSI_SCAN_MESH))
	{
		// Suppress paint.
		S32		NbFaces = m_pSurfaceInfosV3->m_Mesh.m_TabQuad.GetSize();
		for (S32 i = 0; i<NbFaces ; i++)
		{
			Playspace_Mesh::Face &CurFace = m_pSurfaceInfosV3->m_Mesh.m_TabQuad[i];
			CurFace.IsPaintMode = 0;
			CurFace.IsSeenQuality = 0;
		}
		// ReAdd It.
		m_pSurfaceSR->RefreshMergePaintToMesh(m_pSurfaceInfosV3->m_Mesh, 0.1f);		

		// Compute Stats.
		m_ScanningJobsStats.Reset();
		ComputeMeshStats(m_pSurfaceInfosV3->m_Mesh, m_ScanningJobsStats.MeshStats);
		m_ScanningJobsLastStats = m_ScanningJobsStats;
		m_ScanningJobs_RefreshMode = HMapMeshInfos3D::ScanModeIncrementalReInit;
	}

	SetState(PSI_State_Scanning);
}

/**************************************************************************/

void	PlaySpaceInfos_W::ComputeScanningBrush(Float _dTime,Float &_DistMin,Float &_CosMin,Vec3f &_Dir)
{
	// View...
	Vec3f	EyePos = VEC3F_NULL;
	Vec3f	DirEyeFront = VEC3F_FRONT;
	Vec3f	DirEyeLeft = VEC3F_LEFT;
	Vec3f	DirEyeUp = VEC3F_UP;
	Util_L::GetAllViewData(EyePos,DirEyeFront,DirEyeLeft,DirEyeUp);
	Vec3f	RefPos = EyePos + DirEyeFront * 2.f;

	_CosMin = 2.f;
	_DistMin = 0.5f;
	_Dir = VEC3F_NULL;

	if (m_TimeInState > 1.f)
	{
		_Dir = RefPos - m_vPreviousViewPosRef;
		_Dir -= (_Dir * DirEyeFront) * DirEyeFront;
		if( !_Dir.CNormalize() )
		{
			_Dir = m_vPreviousScanTestDir;
		}
		else 
		{
			if( m_vPreviousScanTestDir != VEC3F_NULL )
			{
				Quat q(m_vPreviousScanTestDir,_Dir);
				q.Maximize(Pi * _dTime);
				_Dir = q * m_vPreviousScanTestDir;
			}

			m_vPreviousScanTestDir = _Dir;
			m_vPreviousViewPosRef = RefPos;
		}

		_DistMin = 2.5f*2.5f;
		Float val = m_TimeInState * 10.f;
		if (val > 30.f)
			val = 30.f;

		_CosMin = Cos(DegToRad(val));
	}
	else
	{
		m_vPreviousViewPosRef = RefPos;
	}
}

/**************************************************************************/
static Bool SaveSegV = TRUE;
static Segment_Z	SaveSegView;

Bool	PlaySpaceInfos_W::UpdateScanV3(Float _dTime, Bool _DrawIt, Bool _OneFrameMode)
{
	RefreshEye(_dTime);

	if (m_CurState != PSI_State_Loading)
	{
		EXCEPTIONC_Z(m_CurState == PSI_State_Scanning,"BAD STATE FOR SCANNING");
		if (m_CurState != PSI_State_Scanning)
			return FALSE;
	}

	m_TimeInState += _dTime;

	// Verify Snap.
	VerifySnapToVoxelGrid();

	// Compute Brush
	Float	CosMin = 2.f;
	Float	DistMin = 0.5f;
	Vec3f	vTestDir = VEC3F_NULL;
	ComputeScanningBrush(_dTime,DistMin,CosMin,vTestDir);

/*	// Prepare Start View.
	Float	CosMin = 2.f;
	Float	DistMin = 0.5f;

	if (m_TimeInState > 1.f)
	{
		DistMin = 2.5f*2.5f;
		Float val = m_TimeInState * 10.f;
		if (val > 30.f)
			val = 30.f;

		CosMin = Cos(DegToRad(val));
	}*/

	// Start Point : Eye !
	Segment_Z	SegView;
	Util_L::GetViewSegment(&SegView);

/*	if (SaveSegV)
		SaveSegView = SegView;
	else
		SegView = SaveSegView;
	SaveSegV = FALSE;*/

	Vec3f	EyePos = SegView.Org;
	Vec3f	EyeDir = SegView.Dir;

	// Dist Max scan.
	Float		DistMax = 2.5f;
	if (_OneFrameMode)
		DistMax = 5.f;

	// Init RayCast Process.
	m_pSurfaceInfosV3->m_IsInRayMode = FALSE;
Float t0 = GetAbsoluteTime();
	m_pSurfaceInfosV3->m_Surfels.Init(m_vMinAvailable,m_SizeVoxel,m_NbCellX,m_NbCellY,m_NbCellH);
	m_pSurfaceInfosV3->ReInitScan(m_vMinAvailable, m_vMaxAvailable, m_YGround, m_YCeiling, m_SizeVoxel, m_NbCellX, m_NbCellH, m_NbCellY);
Float t1 = GetAbsoluteTime();
	m_pSurfaceInfosV3->m_MapMesh3D.AddMesh(&m_TheMeshSR,FALSE);
Float t2 = GetAbsoluteTime();
	//if (!m_pSurfaceInfosV3->m_MapMesh3D.ProcessOnePass(SegView,FALSE))
	//		return FALSE;
	if (!m_pSurfaceInfosV3->m_MapMesh3D.ProcessBubbleAlgorithm(SegView, m_pSurfaceSR, FALSE))
		return FALSE;
	Float t3 = GetAbsoluteTime();
#ifdef DRAW_TIME_SCAN
	MESSAGE_Z("Time Scan V3 : %.3f %.3f %.3f", t1 - t0, t2 - t1, t3 - t2);
#endif

	return TRUE;
}

/**************************************************************************/

void		PlaySpaceInfos_W::FinalizeScan()
{
	//if (g_TypeScan & PSI_SCAN_NEW)
	FinalizeScanV3();

	// Stop Task.
	if (m_ScanningJobs_Init)
	{
		m_ScanningJobs.Shut();
		m_ScanningJobs_Init= FALSE;
	}
	m_ScanningJobs_Started = FALSE;
	m_ScanningJobs_Finished = FALSE;

	if (m_PaintingJobs_Init)
	{
		m_PaintingJobs.Shut();
		m_PaintingJobs_Init= FALSE;
	}
	m_PaintingJobs_Started = FALSE;
	m_PaintingJobs_Finished = FALSE;

	// Compute CRC.
	m_CurrentCRC = ComputeCRC();

	// Refresh Tools.
	UpdateTools(TRUE,TRUE,FALSE);

	// Refresh Playfield.
	Segment_Z SegView(m_vOriginalPlayerPos,m_vOriginalPlayerDir,5.f);

	// CONEX -> this need to be in the game part 
	//Vec3f vDelta = (m_vMaxAvailable - m_vMinAvailable);
	//BASE_TELEMETRY(EndScanningSession, vDelta.x, vDelta.z, vDelta.y);

	PlaySpaceInfos_W::m_CodeDatasChange++;
//	gData.Cons->InterpCommand("AddGameMemoryInfos SCAN:FinalizeScan");

	// Finalize.
	SetState(PSI_State_Finalized);
}

/**************************************************************************/

void	PlaySpaceInfos_W::FinalizeScanV3Mesh(Playspace_Mesh *_pMesh,PlaySpace_SurfelBoard *_pSurfels,Playspace_Area *pArea)
{
	// Create Simplified Mesh.
	if (_pMesh->IsEmpty())
		return;

#ifdef DRAW_TIME_SCAN
		Float t1 = GetAbsoluteTime();
#endif

	// Filter Mesh	
	_pMesh->RemoveDeadFaces(0.01f);
	_pMesh->RemoveBadFaces();
	_pMesh->ComputeExternalFaces(m_YGround,m_YCeiling);

//	m_pSurfaceInfosV3->m_Mesh_Test = *_pMesh;
//	_pMesh->CheckBadWinding();

	// Simplified Mesh.
	m_pSurfaceInfosV3->m_SimplifiedMesh = *_pMesh;
	m_pSurfaceInfosV3->m_SimplifiedMesh.Triangularize();

#ifdef DRAW_TIME_SCAN
		Float t2 = GetAbsoluteTime();
#endif
	m_pSurfaceInfosV3->m_SimplifiedMesh.SimplifyTri2(Cos(DegToRad(8.f)), FALSE, FALSE);

#ifdef DRAW_TIME_SCAN
		Float t3 = GetAbsoluteTime();
		MESSAGE_Z("Time Scan V3 - Part2 : %.3f %.3f", t2 - t1, t3 - t2);
#endif

	// Create Surfel and Sort.
	if (pArea)
		_pSurfels->Init(*pArea);
	else
		_pSurfels->Init(m_vMinAvailable,m_SizeVoxel,m_NbCellX,m_NbCellY,m_NbCellH);
	_pSurfels->CreateSurfelsFromMesh(*_pMesh);

	// Rec-compute Conexity Infos.
	_pSurfels->ComputeConexity();

	// Get Ground and Ceiling.
	Float	H;
	S32		Nb;
	m_GroundZoneID = _pSurfels->SearchHorizontalLimit(H,Nb,FALSE);
	if (m_GroundZoneID >= 0)
	{
		m_YGround = H;
		m_YGroundSurface = Nb * 0.08f*0.08f;
	}
	if (!m_IsVirtualCeiling && _pSurfels->SearchHorizontalLimit(H,Nb,TRUE) >= 0)
	{
		m_YCeiling = H;
		m_YCeilingSurface = Nb * 0.08f*0.08f;
	}
	m_NbZoneID = _pSurfels->GetNbZoneId();
//	m_vMinAvailable.y = m_YGround;		// Sinon, ça pète les structures !!!!
//	m_vMaxAvailable.y = m_YCeiling;
//	m_vCenter.y = m_YGround;

	// Filter.
	Float fEyeY = 1000.f;
	if (m_YEyesConfidence > 1.f)
		fEyeY = m_YEyes;

	_pSurfels->FilterSurfel(fEyeY,m_YGround,m_YCeiling);	
	_pSurfels->BasinFilter(m_GroundZoneID);
}

/**************************************************************************/

void	PlaySpaceInfos_W::FinalizeScanV3()
{
#ifdef DRAW_TIME_SCAN
	Float t0 = GetAbsoluteTime();
#endif
	// Wait end Thead.
	if (m_ScanningJobs_Init)
	{
//		EXCEPTIONC_Z(!m_ScanningJobs_Started, "FinalizeScanV3 : WHY PROCESS IS NOT STOP => Scanning Finish !");
		if (m_ScanningJobs_Started)
		{
			// Old scan V3 => Have to wait for the end of process.
			while (!m_ScanningJobs_Finished) { ; }
			OUTPUT_Z("AsynchronousUpdateScanV3 FINISHED");
			while (!m_ScanningJobs.PollForCompletion()) { ; }

			// Get Result.
			if (m_ScanningJobs_Result)
			{
				m_pSurfaceInfosV3->m_Mesh.Swap(m_ScanningJobs_Mesh);
				m_pSurfaceInfosV3->m_MeshCycleID++;
				m_pSurfaceInfosV3->m_Surfels.Swap(m_ScanningJobs_Surfels);
			}
			m_ScanningJobs_Mesh.Flush(FALSE);
			m_ScanningJobs_Surfels.Flush(FALSE);

			// Finish.
			m_ScanningJobs_Started = FALSE;
			m_ScanningJobs_Finished = FALSE;
		}
	}
	if (m_PaintingJobs_Init)
	{
//		EXCEPTIONC_Z(!m_PaintingJobs_Started, "FinalizeScanV3 : WHY PROCESS IS NOT STOP => Scanning Finish !");
		if (m_PaintingJobs_Started)
		{
			// Old scan V3 => Have to wait for the end of process.
			while (!m_PaintingJobs_Finished) { ; }
			while (!m_PaintingJobs.PollForCompletion()) { ; }
			m_PaintingJobs_Started = FALSE;
			m_PaintingJobs_Finished = FALSE;
		}
	}

	// Flush Some Datas on Device only.
#ifndef DISABLE_INPUTMESH_FLUSH
	m_TheMeshSR.Flush(FALSE);
#endif
	m_ScanningJobs_Mesh.Flush(FALSE);
	m_ScanningJobs_Surfels.Flush(FALSE);
	m_pSurfaceInfosV3->m_Mesh.SetEditMode(FALSE,TRUE,FALSE);
	m_pSurfaceInfosV3->m_SimplifiedMesh.SetEditMode(FALSE);
	m_pSurfaceInfosV3->m_MapMesh3D.Flush();
	m_pAlignMgr.Flush();
}

/**************************************************************************/

class GroundAndCeilingInfos
{
public:
	Float	SumY;
	Float	Surface;
	S32		Nb;
};

void	PlaySpaceInfos_W::RefreshGroundAndCeilingMesh( Bool _IsInOnePassMode, Bool _DrawIt)
{
	// Prepare.
	Float t0 = GetAbsoluteTime();
	m_TheMeshSR.ComputePointsLinks();
	Float t1 = GetAbsoluteTime();
	m_TheMeshSR.ComputeFacesToolNormal();
	m_TheMeshSR.ComputePointsToolNormal();
	Float t2 = GetAbsoluteTime();
#ifdef DRAW_TIME_SCAN
	MESSAGE_Z("Time Scan RefreshGroundAndCeilingMesh : %.3f %.3f", t1 - t0, t2 - t1);
#endif

	// Prepare table.
#define OnePassCZMesh_NBVAL		64
#define OnePassCZMesh_MIDDLE	32
	GroundAndCeilingInfos		TabHoriz[OnePassCZMesh_NBVAL];

	Float	TabPrec = 0.1f;
	Float	iTabPrec = 1.f / TabPrec;

	for (S32 i = 0; i<OnePassCZMesh_NBVAL; i++)
	{
		TabHoriz[i].Nb = 0;
		TabHoriz[i].SumY = 0.f;
		TabHoriz[i].Surface = 0.f;
	}

	// Get Cam Pos.
	Segment_Z	SegView;
	Util_L::GetViewSegment(&SegView);

	// 5 Mètre de recherche en gros...
	Float	MiddleSearch = SegView.Org.y;
	Float	MinYSearch = MiddleSearch - 2.5f;
	Float	MaxYSearch = MiddleSearch + 1.7f;

	S32 NbPoint = m_TheMeshSR.m_TabPoints.GetSize();
	Vec3f *TabPoint = m_TheMeshSR.m_TabPoints.GetArrayPtr();
	Float ErrorMin = Cos(DegToRad(10.f));

	for (S32 i = 0; i<NbPoint; i++)
	{
		// Compute normale.
		Playspace_Mesh::ToolPointNormal &NormInfos = m_TheMeshSR.m_TabPointsToolNormal[i];
		if ((NormInfos.Error < ErrorMin) || (m_TheMeshSR.m_TabPointsLinks[i].GetNbFaces() < 3))
			continue;

		Vec3f vNormal = NormInfos.Normal;
		Vec3f vPos = TabPoint[i];

		// Ground Ceiling
		if (vPos.y > MiddleSearch)
		{
			// Ceiling.
			if ((vNormal.y < -0.9f) && (vPos.y < MaxYSearch))
			{
				// Good One..
				S32		Delta = (S32)((vPos.y - MiddleSearch) * iTabPrec) + OnePassCZMesh_MIDDLE;
				if ((Delta >= (OnePassCZMesh_MIDDLE+2)) && (Delta < OnePassCZMesh_NBVAL))
				{
					TabHoriz[Delta].SumY += vPos.y;
					TabHoriz[Delta].Surface += NormInfos.Surface;
					TabHoriz[Delta].Nb++;
					if (_DrawIt)
					{
						DRAW_DEBUG_SPHERE3D(vPos, COLOR_GREEN, 0.05f);
					}
				}
			}

		}
		else
		{
			// Ground.
			if ((vNormal.y > 0.9f) && (vPos.y > MinYSearch))
			{
				// Good One..
				S32		Delta = OnePassCZMesh_MIDDLE - (S32)((MiddleSearch - vPos.y) * iTabPrec);
				if ((Delta >= 0) && (Delta < (OnePassCZMesh_MIDDLE-2)))
				{
					TabHoriz[Delta].SumY += vPos.y;
					TabHoriz[Delta].Surface += NormInfos.Surface;
					TabHoriz[Delta].Nb++;
					if (_DrawIt)
					{
						DRAW_DEBUG_SPHERE3D(vPos, COLOR_RED, 0.05f);
					}
				}
			}
		}
	}

	// Search Ground.
	Float	BetterY = 0.f;
	Float	BetterSurf = 0.f;
	S32		BetterNb = 0;
	Float	MinSurf = 10.f * 0.08f * 0.08f;
	for (S32 i = 1; i<OnePassCZMesh_MIDDLE - 1; i++)
	{
		Float	CurSurface = TabHoriz[i].Surface;
		if (CurSurface < MinSurf)
			continue;

		S32	  nb = TabHoriz[i].Nb;
		Float valY = TabHoriz[i].SumY / (Float)nb;
		Float Limit = (Float)(i)* TabPrec + MinYSearch;

		if ((valY - Limit) < 0.5f)
		{
			nb += TabHoriz[i - 1].Nb;
			CurSurface += TabHoriz[i - 1].Surface;
			valY = (TabHoriz[i].SumY + TabHoriz[i - 1].SumY) / (Float)nb;
		}
		else
		{
			nb += TabHoriz[i + 1].Nb;
			CurSurface += TabHoriz[i + 1].Surface;
			valY = (TabHoriz[i].SumY + TabHoriz[i + 1].SumY) / (Float)nb;
		}

		if (CurSurface > BetterSurf)
		{
			BetterNb = nb;
			BetterY = valY;
			BetterSurf = CurSurface;
		}
	}

	Float	OKSurf = 1.f;	// 1 square metter.
	if ((BetterSurf > OKSurf) && (BetterSurf >= (m_YGroundSurface * 0.8f)))
	{
		m_YGround = BetterY;
		m_YGroundMeasure = BetterY;
		m_YGroundSurface = BetterSurf;

		if (_DrawIt)
		{
			Vec3f dir = SegView.Dir;
			dir.CHNormalize();
			Vec3f pos = SegView.Org + dir * 1.2f;
			pos.y = m_YGround;
			DRAW_DEBUG_SPHERE3D(pos, COLOR_GREEN *0.9f, 0.5f);
		}
	}

	// Search Ceiling.
	BetterY = 0.f;
	BetterSurf = 0.f;
	BetterNb = 0;
	for (S32 i = OnePassCZMesh_MIDDLE + 1; i<OnePassCZMesh_NBVAL - 1; i++)
	{
		Float	CurSurface = TabHoriz[i].Surface;
		if (CurSurface < MinSurf)
			continue;

		S32	  nb = TabHoriz[i].Nb;
		Float valY = TabHoriz[i].SumY / (Float)nb;
		Float Limit = (Float)(i)* TabPrec + MinYSearch;

		if ((valY - Limit) < 0.5f)
		{
			nb += TabHoriz[i - 1].Nb;
			CurSurface += TabHoriz[i - 1].Surface;
			valY = (TabHoriz[i].SumY + TabHoriz[i - 1].SumY) / (Float)nb;
		}
		else
		{
			nb += TabHoriz[i + 1].Nb;
			CurSurface += TabHoriz[i + 1].Surface;
			valY = (TabHoriz[i].SumY + TabHoriz[i + 1].SumY) / (Float)nb;
		}

		if (CurSurface > BetterSurf)
		{
			BetterNb = nb;
			BetterY = valY;
			BetterSurf = CurSurface;
		}
	}


	if ((BetterSurf > OKSurf) && (BetterSurf >= (m_YCeilingSurface * 0.8f)))
	{
		m_YCeilingMeasure = BetterY;
		m_YCeilingSurface = BetterSurf;
		if (_DrawIt)
		{
			Vec3f dir = SegView.Dir;
			dir.CHNormalize();
			Vec3f pos = SegView.Org + dir * 1.2f;
			pos.y = m_YCeiling;
			DRAW_DEBUG_SPHERE3D(pos, COLOR_RED *0.9f, 0.4f);
		}
	}

	// Compute Clamped Ceiling.
	m_YCeiling = m_YCeilingMeasure;
	m_IsVirtualCeiling = FALSE;

	Float MaxCeiling = m_YEyes + WALL_MAX_HEIGHT;
	if (m_YEyesConfidence < 1.f)
	{
		// Get Camera pos...
		MaxCeiling = SegView.Org.y + WALL_MAX_HEIGHT;
	}

	if (	(m_YCeilingMeasure >  MaxCeiling)
		||	(m_YCeilingSurface < 1.f)
		)
	{
		// Pas de plafond... :(
		m_YCeiling = MaxCeiling;
		m_YCeilingMeasure = MaxCeiling;
		m_IsVirtualCeiling = TRUE;

		if (_DrawIt)
		{
			Vec3f dir = SegView.Dir;
			dir.CHNormalize();
			Vec3f pos = SegView.Org + dir * 1.2f;
			pos.y = m_YCeiling;
			DRAW_DEBUG_SPHERE3D(pos, COLOR_WHITE *0.9f, 0.4f);
		}
	}
}

/**************************************************************************/

#define		NB_LIMIT_MESH_ACC	64

class OneMeshLimit
{
public:
	Float	SumDist;
	Float	Surface;
	S32		Nb;
};

class LimitMeshAcc
{
public:
	OneMeshLimit	TabLimit[NB_LIMIT_MESH_ACC];

	LimitMeshAcc()
	{
		memset(TabLimit, 0, sizeof(OneMeshLimit)*NB_LIMIT_MESH_ACC);
	}
	void	AddLimit(Float _Dist,Float _Surface)
	{
		S32	num = _Dist * 10.f;
		if (num < 0)
			return;
		if (num >= NB_LIMIT_MESH_ACC)
			return;
		TabLimit[num].Nb++;
		TabLimit[num].SumDist += _Dist;
		TabLimit[num].Surface += _Surface;
	}
	Float	SearchLimit(Float _SurfMin = 0.2f)
	{
		S32 betterNum = -1;
		Float betterSurf = _SurfMin;
		for (S32 i = 0; i<NB_LIMIT_MESH_ACC; i++)
		{
			if (TabLimit[i].Surface > betterSurf)
			{
				betterNum = i;
				betterSurf = TabLimit[i].Surface;
			}
		}

		if (betterNum < 0)
			return 6.f;
		return TabLimit[betterNum].SumDist / (Float)(TabLimit[betterNum].Nb);
	}
};

Bool	PlaySpaceInfos_W::RefreshZoneMesh(const Vec3f& _AlignXAxis, Bool _DrawIt)
{
	// Prepare.
	m_TheMeshSR.ComputePointsLinks();
	m_TheMeshSR.ComputeFacesToolNormal();
	m_TheMeshSR.ComputePointsToolNormal();

	// Cherche le sol et le plafond...
	if (((m_YCeilingSurface <= 0.f) && !m_IsVirtualCeiling)
		|| (m_YGroundSurface <= 0.f)
		)
		return FALSE;

	Float	CeilingHeight = m_YCeiling;
	Float	FloorHeight = m_YGround;

	if (g_TypeScan & PSI_SCAN_NEW)
	{
		m_vMinAvailable.y = FloorHeight - MARGIN_GROUND_CEILING;
		m_vMaxAvailable.y = CeilingHeight + MARGIN_GROUND_CEILING;
	}
	else
	{
		// Avoid Blob pb :(
		m_vMinAvailable.y = FloorHeight;
		m_vMaxAvailable.y = CeilingHeight;
	}

	// Search Walls...
	Float YGround = m_vMinAvailable.y;
	Float YCeiling = m_vMaxAvailable.y;

	if ((YCeiling - YGround) > 3.f)
		YCeiling = YGround + 3.f;
	else if ((YCeiling - YGround) < 1.f)
		YCeiling = YGround + 2.f;

	// Compute FrontDir.
	Vec3f	FrontAxis = _AlignXAxis;
	FrontAxis.CHNormalize();
	{
		Vec3f	RefDir = m_vOriginalPlayerDir;
		Vec3f	CurDir = FrontAxis;
		Float	MaxDot = CurDir * RefDir;
		for (S32 i = 0; i<3; i++)
		{
			Vec3f work(-CurDir.z, 0.f, CurDir.x);
			CurDir = work;

			Float CurDot = (CurDir*RefDir);
			if (CurDot > MaxDot)
			{
				MaxDot = CurDot;
				FrontAxis = CurDir;
			}
		}
	}

	Vec3f	LeftAxis = Quat(M_PI_2, VEC3F_UP) * FrontAxis;

	// Angle Table.
	LimitMeshAcc	AccFront;
//	LimitMeshAcc	AccBack;
	LimitMeshAcc	AccLeft;
	LimitMeshAcc	AccRight;

	// Now Get Some infos.
	Segment_Z	SegView;
	Util_L::GetViewSegment(&SegView);
	Vec3f	EyePos = SegView.Org;

//	Vec3f	FlatSegViewDir = SegView.Dir;
//	FlatSegViewDir.CHNormalize();

	// Search Vertical surfaces.
	Float MinCos = Cos(DegToRad(6.f));

	S32 NbPoint = m_TheMeshSR.m_TabPoints.GetSize();
	for (S32 i = 0; i<NbPoint; i++)
	{
		// Compute normale.
		Playspace_Mesh::ToolPointNormal &NormInfos = m_TheMeshSR.m_TabPointsToolNormal[i];
		if (NormInfos.Error < MinCos)
			continue;

		Vec3f vNormal = NormInfos.Normal;
		Vec3f vPos = m_TheMeshSR.m_TabPoints[i];

		// Ground Ceiling
		if ((vNormal.y > 0.707f) || (vNormal.y < -0.707f))	// Not horizontal.
			continue;
		if (vPos.y < YGround)
			continue;
		if (vPos.y > YCeiling)
			continue;

		// Good Angle ?
		Vec3f Dir = vPos - EyePos;
		Dir.y = 0;
		Float PosFront = Dir * FrontAxis;
		Float PosLeft = Dir * LeftAxis;

		Float DotFront = FrontAxis * vNormal;
		Float DotLeft = LeftAxis * vNormal;

		// Front Axis.
		Bool CouldDraw = FALSE;
		if (PosFront > 0.f)
		{
			// Front.
			if ((DotFront < 0.f) && (-DotFront > Abs(DotLeft)))
			{
				// A good one for Front.
				CouldDraw = TRUE;
				AccFront.AddLimit(PosFront, NormInfos.Surface);
			}
		}
/*		else if ((vNormal.z > 0.f) && (vNormal.z > Abs(vNormal.x)))
		{
			// A good one for Back.
			CouldDraw = TRUE;
			AccBack.AddLimit(-PosFront, NormInfos.Surface);
		}*/
		
		// Left Axis
		if (PosLeft > 0.f)
		{
			// Front.
			if ((DotLeft < 0.f) && (-DotLeft > Abs(DotFront)))
			{
				// A good one for Left.
				CouldDraw = TRUE;
				AccLeft.AddLimit(PosLeft, NormInfos.Surface);
			}
		}
		else if ((DotLeft > 0.f) && (DotLeft > Abs(DotFront)))
		{
			// A good one for Right.
			CouldDraw = TRUE;
			AccRight.AddLimit(-PosLeft, NormInfos.Surface);
		}

		// Got IT !

		if (_DrawIt && CouldDraw)
		{
			Color TheCoolCol = COLOR_GREEN;
			Float ColorV = Atan2(vNormal.z, vNormal.x);
			ColorV += M_PI;	// Rotation de 180 pour garder continuité MAIS voir que du positif.
			TheCoolCol.r = ColorV * 6.f / M_2_PI;
			TheCoolCol.g = 1.f;	// S
			TheCoolCol.b = 1.f; // V
			TheCoolCol = TheCoolCol.FromHSVToRGB();
			TheCoolCol.a = 1.f;
			DRAW_DEBUG_LINE3D(vPos, vPos + vNormal*0.2f, TheCoolCol, 0.01f);
		}
	}

	// Expand Playspace...
	Float FrontDist = AccFront.SearchLimit();
	Float LeftDist = AccLeft.SearchLimit();
	Float RightDist = AccRight.SearchLimit();

	// Front Snap + Margin
	Vec3f	FrontMax = EyePos + FrontAxis * FrontDist;
	Vec3f	Delta = FrontMax - m_vOriginalPlayerPos;
	Float	DistFront = FrontAxis * Delta;
	FrontMax = m_vOriginalPlayerPos + FrontAxis * DistFront;

	Float MarginV3 = m_SizeVoxel * 2.f;

	FrontMax += FrontAxis*MarginV3;

	// Compute Front Min.
	Vec3f	PlayerToFrontMax = FrontMax - m_vOriginalPlayerPos;
	Float	DistToWall = FrontAxis * PlayerToFrontMax;
	Vec3f	FrontMin;
	if (DistToWall < 2.5f)
		FrontMin = FrontMax - FrontAxis * 2.5f;
	else if (DistToWall > m_OptimalZoneSize)
		FrontMin = FrontMax - FrontAxis * m_OptimalZoneSize;
	else
		FrontMin = FrontMax - FrontAxis * DistToWall;

	Vec3f	CurCenter = (FrontMin + FrontMax) * 0.5f;
	//MESSAGE_Z("DEUBG CurCenter %f %f %f",CurCenter.x,CurCenter.y,CurCenter.z);

	FrontMax.y = m_YGround;
	FrontMin.y = m_YGround;
	CurCenter.y = m_YGround;

	if (_DrawIt)
	{
		DRAW_DEBUG_LINE3D(FrontMin, FrontMax, COLOR_GREEN, 0.05f);
	}

	// Get Left and Right Limits.
	Vec3f	RightLimit = EyePos - LeftAxis * RightDist;
	Vec3f	LeftLimit = EyePos + LeftAxis * LeftDist;
	Float DistR = (CurCenter - RightLimit) * LeftAxis;
	Float DistL = (LeftLimit - CurCenter) * LeftAxis;
	DistR += MarginV3;
	DistL += MarginV3;
	RightLimit = CurCenter - LeftAxis * DistR;
	LeftLimit = CurCenter + LeftAxis * DistL;

	if (m_OptimalZoneSize < (DistR + DistL))
	{
		// Don't use all space.
		if ((DistR < m_OptimalZoneSize * 0.5f)
			|| ((DistR < DistL) && (DistR < (m_OptimalZoneSize * 0.7f)))
			)
		{
			// Snap Right.
			LeftLimit = RightLimit + LeftAxis * m_OptimalZoneSize;
			CurCenter = (RightLimit + LeftLimit) * 0.5f;
		}
		else if ((DistL < m_OptimalZoneSize * 0.5f)
			|| ((DistL < DistR) && (DistL < (m_OptimalZoneSize * 0.7f)))
			)
		{
			// Snap Left.
			RightLimit = LeftLimit - LeftAxis * m_OptimalZoneSize;
			CurCenter = (RightLimit + LeftLimit) * 0.5f;
		}
		else
		{
			// Use max space.
			RightLimit = CurCenter - LeftAxis * m_OptimalZoneSize * 0.5f;
			LeftLimit = CurCenter + LeftAxis * m_OptimalZoneSize * 0.5f;
		}
	}

	// PlaySpace.
	/*
	Float SizeF = (FrontMax - FrontMin).HGetNorm();
	Float SizeLR = (RightLimit - LeftLimit).HGetNorm();*/

	Vec3f	DeltaF = (FrontMax - FrontMin) * 0.5f;
	m_vNonAlignedZone[0] = RightLimit - DeltaF;
	m_vNonAlignedZone[1] = RightLimit + DeltaF;
	m_vNonAlignedZone[2] = LeftLimit - DeltaF;
	m_vNonAlignedZone[3] = LeftLimit + DeltaF;

	ComputeMinMax();

	if (_DrawIt)
	{
		DRAW_DEBUG_LINE3D(CurCenter, RightLimit, COLOR_RED, 0.05f);
		DRAW_DEBUG_LINE3D(CurCenter, LeftLimit, COLOR_BLUE, 0.05f);

		DRAW_DEBUG_LINE3D(m_vNonAlignedZone[0], m_vNonAlignedZone[1], COLOR_YELLOW, 0.07f);
		DRAW_DEBUG_LINE3D(m_vNonAlignedZone[2], m_vNonAlignedZone[3], COLOR_YELLOW, 0.07f);
		DRAW_DEBUG_LINE3D(m_vNonAlignedZone[0], m_vNonAlignedZone[2], COLOR_YELLOW, 0.07f);
		DRAW_DEBUG_LINE3D(m_vNonAlignedZone[1], m_vNonAlignedZone[3], COLOR_YELLOW, 0.07f);
	}

	return TRUE;
}

/**************************************************************************/


Bool	PlaySpaceInfos_W::OnePassComputeZoneMesh()
{
	// Refresh G et C.
	RefreshGroundAndCeilingMesh( TRUE);

	// Alignement by default is set on Playspace center.
	m_AlignXAxis = Quat(VEC3F_FRONT, m_vOriginalPlayerDir) * VEC3F_LEFT;

	// Define Zone => Like Scan.
	if ((m_YCeilingSurface > 0.f) || (m_IsVirtualCeiling))
	{
		m_pAlignMgr.SearchStabilizedAlignAxis_Mesh(m_TheMeshSR, m_YGround, m_YCeiling, m_AlignXAxis, TRUE, FALSE);
	}

	// GetZone.
	if (!RefreshZoneMesh(m_AlignXAxis, FALSE))
		return FALSE;

	AlignSurfaceReco(m_AlignXAxis);		// A virer à terme...
	m_TheMeshSR.ApplyQuat(m_AlignTransfo);

	// Define Zone => static.

	// Cherche le sol et le plafond...
	if (g_TypeScan & PSI_SCAN_NEW)
	{
		m_vMinAvailable.y = m_YGround - MARGIN_GROUND_CEILING;
		m_vMaxAvailable.y = m_YCeiling + MARGIN_GROUND_CEILING;
	}
	else
	{
		// Avoid Blob pb :(
		m_vMinAvailable.y = m_YGround;
		m_vMaxAvailable.y = m_YCeiling;
	}

	Vec3f DirRight = m_vOriginalPlayerDir^VEC3F_UP;

	/*	Vec3f	RightLimit= _vPlayCenter + DirRight * _fSize * 0.5f;
	Vec3f	LeftLimit = _vPlayCenter - DirRight * _fSize * 0.5f;

	Vec3f	DeltaF = _vPlayFrontDir * _fSize * 0.5f;
	m_vNonAlignedZone[0] = RightLimit - DeltaF;
	m_vNonAlignedZone[1] = RightLimit + DeltaF;
	m_vNonAlignedZone[2] = LeftLimit - DeltaF;
	m_vNonAlignedZone[3] = LeftLimit + DeltaF;*/

	ComputeMinMax();
	return TRUE;
}

/**************************************************************************/

Bool	PlaySpaceInfos_W::OnePassComputePlaySpace(const Vec3f& _vPlayerPosition,const Vec3f& _vPlayerFrontDir,Float _fSize)
{
	if (!InitProcess(_vPlayerPosition,_vPlayerFrontDir,_fSize*1.5f,_fSize))
		return FALSE;

	// Refresh Eye.
	Segment_Z	SegView;
	Util_L::GetViewSegment(&SegView);
	Vec3f	PosEyes = SegView.Org;
	m_YEyes = PosEyes.y;
	m_YEyesConfidence = 100.f;

	Float t0 = GetAbsoluteTime();

	Bool MeshIsOk = RefreshSurfaceSR(NULL,FALSE, FALSE, TRUE);
	EXCEPTIONC_Z(MeshIsOk,"NO SR Mesh");
	if (!RefreshMeshFromSurfaceSR())
		return FALSE;

	if (!OnePassComputeZoneMesh())
	{
#ifndef DISABLE_INPUTMESH_FLUSH
		m_TheMeshSR.Flush(FALSE);
#endif
		return FALSE;
	}

	// Redefine Zone.
	m_pSurfaceSR->SetValidZone(m_vMinAvailable,m_vMaxAvailable);
	m_pSurfaceSR->MoveIfNeeded(TRUE);
	if (!RefreshMeshFromSurfaceSR())
		return FALSE;

	Float t1 = GetAbsoluteTime();

	// Scan !
	if (!InitScan())
		return FALSE;
	m_CurState = PSI_State_Scanning;
	// Filter for Scan.
	if (g_TypeScan & PSI_SCAN_MESH)
		FilterMeshFromSR();
	Float t2 = GetAbsoluteTime();
	// Scan !
	if (!UpdateScanV3(0.16f,FALSE,TRUE))
		return FALSE;
	m_fConfidence = 1.f;

	// Create Mesh.
	m_pSurfaceInfosV3->CreateMesh(m_pSurfaceInfosV3->m_Mesh);

	Float t3 = GetAbsoluteTime();
#ifdef DRAW_TIME_SCAN
	MESSAGE_Z("Total Time Scan V3 : %.3f %.3f %.3f", t1 - t0, t2 - t1, t3 - t2);
#endif

	// Set Infos to GOOD for solving
	PlaySpace_CellInfos *pCell = m_pSurfaceInfosV3->m_Surfels.m_CellBoard.GetArrayPtr();
	for (S32 y=0 ; y<m_NbCellY ; y++)
	{
		for (S32 x=0 ; x<m_NbCellX ; x++)
		{
			PlaySpace_Surfel *pSurfel = pCell->pFirst;
			while (pSurfel)
			{
				pSurfel->Quality = 255;
				pSurfel = pSurfel->pNext;
			}
			// Next.
			pCell++;
		}
	}

	// All face are painted with minimal quality.
	S32		NbFaces = m_pSurfaceInfosV3->m_Mesh.m_TabQuad.GetSize();
	for (S32 i = 0; i<NbFaces ; i++)
	{
		Playspace_Mesh::Face &CurFace = m_pSurfaceInfosV3->m_Mesh.m_TabQuad[i];
		CurFace.IsPaintMode = 2;
		CurFace.IsSeenQuality = 3;
	}

	// Finalize Scan.
	FinalizeScanV3Mesh(&m_pSurfaceInfosV3->m_Mesh,&m_pSurfaceInfosV3->m_Surfels);

	// Compute Stats.
	m_ScanningJobsStats.Reset();
	ComputeMeshStats(m_pSurfaceInfosV3->m_Mesh, m_ScanningJobsStats.MeshStats);
	m_ScanningJobsLastStats = m_ScanningJobsStats;
	m_ScanningJobs_RefreshMode = HMapMeshInfos3D::ScanModeIncrementalReInit;

	// Finalize.
	FinalizeScan();
	return TRUE;
}

Bool	PlaySpaceInfos_W::IsInsideBBox(const Vec3f& _vPos, Float _Tolerance)
{
	if (!m_pSurfaceInfosV3)
		return TRUE;

	if (_vPos.x > m_vMaxAvailable.x - _Tolerance)
		return FALSE;
	if (_vPos.x < m_vMinAvailable.x + _Tolerance)
		return FALSE;

	if (_vPos.y > m_vMaxAvailable.y - _Tolerance)
		return FALSE;
	if (_vPos.y < m_vMinAvailable.y + _Tolerance)
		return FALSE;

	if (_vPos.z > m_vMaxAvailable.z - _Tolerance)
		return FALSE;
	if (_vPos.z < m_vMinAvailable.z + _Tolerance)
		return FALSE;

	return TRUE;

}

Bool	PlaySpaceInfos_W::IsInside(const Vec3f &_vPos, Float _Tolerance)
{
	if (!m_pSurfaceInfosV3 || m_pSurfaceInfosV3->m_Surfels.IsEmpty())
		return TRUE;

	Vec3f vMin = m_vMinAvailable;
	Vec3f vMax = m_vMaxAvailable;

	if (_vPos.x > vMax.x - _Tolerance)
		return FALSE;
	if (_vPos.x < vMin.x + _Tolerance)
		return FALSE;

	if (_vPos.z > vMax.z - _Tolerance)
		return FALSE;
	if (_vPos.z < vMin.z + _Tolerance)
		return FALSE;

	S32 PosX = (_vPos.x - m_vMinAvailable.x) / m_SizeVoxel;
	S32 PosY = (_vPos.z - m_vMinAvailable.z) / m_SizeVoxel;

	PosX = Max(PosX,(S32)0);
	PosX = Min(PosX,m_NbCellX-1);

	PosY = Max(PosY,(S32)0);
	PosY = Min(PosY,m_NbCellY-1);

	// A compléter...
	// Faire un détecteur de INSIDE WALL...
	PlaySpace_CellInfos	*pCell = &(m_pSurfaceInfosV3->m_Surfels.m_CellBoard[PosY * m_NbCellX + PosX]);
	PlaySpace_Surfel	*pSurfel = pCell->pFirst;
	if (!pSurfel)
		return FALSE;

/*		while (pSurfel)
	{
		// Next 
		pSurfel = pSurfel->pFirst
	}*/
/*		if (pCell->Level[0].Flags & PlaySpace_SurfaceInfos::PSI_WALL)
		return FALSE;
	if (_vPos.y < pCell->Level[0].Height)
		return FALSE;
	if (_vPos.y > pCell->Level[1].Height)
		return FALSE;*/

	return TRUE;
}

/**************************************************************************/

Bool	PlaySpaceInfos_W::GetNeighborPos(const Vec3f& _vPos, const Vec3f& _vNormal, PlaySpaceInfos_W::NeightborPos& _outPos)
{
	Float halfVoxel = m_SizeVoxel * .5f;
	Vec3f outsidePos = _vPos + _vNormal * halfVoxel;
	if( IsPosFilled(outsidePos,0.f) )
		return FALSE;

	Vec3f insidePos = _vPos - _vNormal * halfVoxel;
	if( !IsPosFilled(insidePos,0.f) )
		return FALSE;

	static Float Tolerance = 0.01f;

	if (_vPos.x > m_vMaxAvailable.x - halfVoxel + Tolerance)
		return FALSE;
	if (_vPos.x < m_vMinAvailable.x - halfVoxel - Tolerance)
		return FALSE;

	if (_vPos.z > m_vMaxAvailable.z - halfVoxel + Tolerance)
		return FALSE;
	if (_vPos.z < m_vMinAvailable.z - halfVoxel - Tolerance)
		return FALSE;

	Vec3f originGrid = m_vMinAvailable ;
	originGrid.y = m_YGround + halfVoxel; // ajout demi case en hauteur pour le m_vMinAvailable

	S32 PosX = ROUNDINT((insidePos.x - originGrid.x) / m_SizeVoxel);
	S32 PosY = ROUNDINT((insidePos.y - originGrid.y) / m_SizeVoxel);
	S32 PosZ = ROUNDINT((insidePos.z - originGrid.z) / m_SizeVoxel);

	_outPos.m_vCellPos.x = PosX;
	_outPos.m_vCellPos.y = PosY;
	_outPos.m_vCellPos.z = PosZ;

	Vec3f absNormal;
	absNormal.x = POS_Z(_vNormal.x);
	absNormal.y = POS_Z(_vNormal.y);
	absNormal.z = POS_Z(_vNormal.z);
	if( absNormal.x > absNormal.y && absNormal.x > absNormal.z )
	{
		if( _vNormal.x >= 0.f )
			_outPos.m_vWorldNormal = VEC3F_LEFT;
		else
			_outPos.m_vWorldNormal = VEC3F_RIGHT;
	}
	else if( absNormal.y > absNormal.z )
	{
		if( _vNormal.y >= 0.f )
			_outPos.m_vWorldNormal = VEC3F_UP;
		else
			_outPos.m_vWorldNormal = VEC3F_DOWN;
	}
	else
	{
		if( _vNormal.z >= 0.f )
			_outPos.m_vWorldNormal = VEC3F_FRONT;
		else
			_outPos.m_vWorldNormal = VEC3F_BACK;
	}

	Vec3f centerPos = originGrid + Vec3f(PosX,PosY,PosZ) * m_SizeVoxel;
	_outPos.m_vWorldPos = centerPos + _outPos.m_vWorldNormal * halfVoxel;

	//_outPos.m_fHDelta = TODO;

	return TRUE;
}

/**************************************************************************/

Bool	PlaySpaceInfos_W::GetNearestNeighborPos(const Vec3f& _vPos, PlaySpaceInfos_W::NeightborPos& _outPos)
{
	Float	minDist2 = 1e7f;
	Bool	bFound = FALSE;
	static Vec3f allNormal[6] = 
	{
		VEC3F_LEFT,
		VEC3F_RIGHT,
		VEC3F_FRONT,
		VEC3F_BACK,
		VEC3F_DOWN,
		VEC3F_UP
	};

	for(S32 i=0;i<_countof(allNormal);i++)
	{
		NeightborPos newPos;
		if( GetNeighborPos(_vPos,allNormal[i],newPos) )
		{
			Float dist2 = (newPos.m_vWorldPos - _vPos).GetNorm2();
			if( dist2 < minDist2 )
			{
				bFound = TRUE;
				minDist2 = dist2;
				_outPos = newPos;
			}
		}
	}

	return bFound;
}

/**************************************************************************/

void	PlaySpaceInfos_W::GetNeighborsFromNeightborPos(const Vec3f& _inPos, const Vec3f& _inNormal, DynArray_Z<NeightborPos>& _outNeighbor)
{
	// orientation up to normal
	Quat rot = Quat(VEC3F_UP,_inNormal);

	// in UP coord
	static Vec3f saDelta[][2] = 
	{
		// same surface
		Vec3f(-1.f,0.01f,-1.f),	VEC3F_UP,
		Vec3f(-1.f,0.01f,0.f),	VEC3F_UP,
		Vec3f(-1.f,0.01f,1.f),	VEC3F_UP,
		Vec3f(0.f,0.01f,-1.f),	VEC3F_UP,
		Vec3f(0.f,0.01f,1.f),	VEC3F_UP,
		Vec3f(1.f,0.01f,-1.f),	VEC3F_UP,
		Vec3f(1.f,0.01f,0.f),	VEC3F_UP,
		Vec3f(1.f,0.01f,1.f),	VEC3F_UP,

		// change surface, move to wall
		Vec3f(-.5f,.5f,0.f),	VEC3F_LEFT,
		Vec3f(.5f,.5f,0.f),		VEC3F_RIGHT,
		Vec3f(0.f,.5f,-.5f),	VEC3F_FRONT,
		Vec3f(0.f,.5f,.5f),		VEC3F_BACK,

		// change surface, go down
		Vec3f(-.5f,-.5f,0.f),	VEC3F_RIGHT,
		Vec3f(.5f,-.5f,0.f),	VEC3F_LEFT,
		Vec3f(0.f,-.5f,-.5f),	VEC3F_BACK,
		Vec3f(0.f,-.5f,.5f),	VEC3F_FRONT,
	};

	// same surface
	for(S32 i=0;i<_countof(saDelta);i++)
	{
		NeightborPos newPos;
		Vec3f pos = _inPos + rot * saDelta[i][0] * m_SizeVoxel;
		Vec3f normal = rot * saDelta[i][1];
		if( GetNeighborPos(pos,normal,newPos) )
			_outNeighbor.Add(newPos);
	}
}

/**************************************************************************/

Bool	PlaySpaceInfos_W::RayCastVoxel(const Vec3f &p1, const Vec3f &p2, Vec3f &pResult, S32 *_pZoneID, Bool _precise, PlaySpace_Surfel **_ppSurfel)
{
	/*Bool _DrawIt = FALSE;
	if (_pZoneID == (S32 *)1)
	{
		_DrawIt = TRUE;
		_pZoneID = 0;
	}*/

//		return FALSE;
	// NEW SCAN RAY CAST.
	if (!m_pSurfaceInfosV3 || m_pSurfaceInfosV3->m_Surfels.IsEmpty())
		return FALSE;

	Float demiVoxel2 = m_SizeVoxel * m_SizeVoxel * 0.25f;
	Float invSizeVoxel = 1.0f / m_SizeVoxel;
	Vec2f Start,End;
	Start.x = (p1.x - m_vMinAvailable.x) * invSizeVoxel;
	Start.y = (p1.z - m_vMinAvailable.z) * invSizeVoxel;

	End.x = (p2.x - m_vMinAvailable.x) * invSizeVoxel;
	End.y = (p2.z - m_vMinAvailable.z) * invSizeVoxel;

	/*if (_DrawIt)
	{
		Vec3f pS1 = p1;
		pS1.y = m_YGround + 0.01f;
		Vec3f pS2 = p2;
		pS2.y = m_YGround + 0.01f;
		DRAW_DEBUG_LINE3D(pS1,pS2, COLOR_GREEN, 0.01f);
	}*/
	Bresenham2DInt_Z	MyBres;
	MyBres.InitSead(Start.x,Start.y,End.x,End.y);
	S32	X,Y;

	Bool	IsOut = FALSE;

	//Box_Z		CollideBox;
	//Segment_Z	CollideSeg(p1,p2);

	Vec3f View = p2 - p1;
	Bool x_ok = FALSE, z_ok = FALSE, y_ok = FALSE;
	Float inv_x, inv_z, inv_y;

	if (View.x * View.x > 1e-8f)
	{
		x_ok = TRUE;
		inv_x = 1.0f / View.x;
	}
	if (View.z * View.z > 1e-8f)
	{
		z_ok = TRUE;
		inv_z = 1.0f / View.z;
	}
	if (View.y * View.y > 1e-8f)
	{
		y_ok = TRUE;
		inv_y = 1.0f / View.y;
	}
	if (!x_ok && !y_ok && !z_ok)
		return FALSE;

	Vec3f				BoxSize(m_SizeVoxel, m_SizeVoxel, m_SizeVoxel);
	//CollisionReport_Z	CollideReport;
	Float distThres = m_SizeVoxel * m_SizeVoxel;
	Vec3f CenterBias(m_SizeVoxel * 0.5f, m_SizeVoxel * 0.5f, m_SizeVoxel * 0.5f);

	while (MyBres.GetNextPointSead(X,Y))
	{
		// Get Cell.
		if ((X < 0) || (Y < 0) || (X >= m_NbCellX) || (Y >= m_NbCellY))
		{
			IsOut = TRUE;
			if (_pZoneID)
				*_pZoneID = -1;
			continue;
		}
		//Vec3f Pos((Float)X*m_SizeVoxel + m_vMinAvailable.x + m_SizeVoxel*0.5f, 0.f, (Float)Y*m_SizeVoxel + m_vMinAvailable.z + m_SizeVoxel*0.5f);
		Vec3f Pos((Float)X*m_SizeVoxel + m_vMinAvailable.x, 0.f, (Float)Y*m_SizeVoxel + m_vMinAvailable.z);
		Float ymin = 1e8f, ymax = -1e8f;
		Bool intersect[6] = {FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};
		Float y[4], t[6];
		if (x_ok)
		{
			t[0] = (Pos.x - p1.x) * inv_x;
			float z = p1.z + (t[0] * View.z);
			if (/*(t[0] > 0.0f) && (t[0] < 1.0f) &&*/ (z >= Pos.z) && (z <= (Pos.z + m_SizeVoxel)))
			{
				intersect[0] = TRUE;
				y[0] = p1.y + (t[0] * View.y);
			}
			t[1] = (Pos.x + m_SizeVoxel - p1.x) * inv_x;
			z = p1.z + (t[1] * View.z);
			if (/*(t[1] > 0.0f) && (t[1] < 1.0f) &&*/ (z >= Pos.z) && (z <= (Pos.z + m_SizeVoxel)))
			{
				intersect[1] = TRUE;
				y[1] = p1.y + (t[1] * View.y);
			}
		}
		if (z_ok)
		{
			t[2] = (Pos.z - p1.z) * inv_z;
			float x = p1.x + (t[2] * View.x);
			if (/*(t[2] > 0.0f) && (t[2] < 1.0f) &&*/ (x >= Pos.x) && (x <= (Pos.x + m_SizeVoxel)))
			{
				intersect[2] = TRUE;
				y[2] = p1.y + (t[2] * View.y);
			}
			t[3] = (Pos.z + m_SizeVoxel - p1.z) * inv_z;
			x = p1.x + (t[3] * View.x);
			if (/*(t[3] > 0.0f) && (t[3] < 1.0f) &&*/ (x >= Pos.x) && (x <= (Pos.x + m_SizeVoxel)))
			{
				intersect[3] = TRUE;
				y[3] = p1.y + (t[3] * View.y);
			}
		}
		for (S32 i = 0; i < 4; i++)
		{
			if (intersect[i])
			{
				if (y[i] < ymin)
					ymin = y[i];

				if (y[i] > ymax)
					ymax = y[i];
			}
		}
		if ((p1.x >= Pos.x) && (p1.x <= (Pos.x + m_SizeVoxel)) && (p1.z >= Pos.z) && (p1.z <= (Pos.z + m_SizeVoxel)))
		{
			if (p1.y > ymax)
				ymax = p1.y;

			if (p1.y < ymin)
				ymin = p1.y;
		}
		if ((p2.x >= Pos.x) && (p2.x <= (Pos.x + m_SizeVoxel)) && (p2.z >= Pos.z) && (p2.z <= (Pos.z + m_SizeVoxel)))
		{
			if (p2.y > ymax)
				ymax = p2.y;

			if (p2.y < ymin)
				ymin = p2.y;
		}
		ymin -= 0.0001f;
		ymax += 0.0001f;
		PlaySpace_CellInfos	*pCell = &(m_pSurfaceInfosV3->m_Surfels.m_CellBoard[Y * m_NbCellX + X]);
		PlaySpace_Surfel	*pSurfel = pCell->pFirst;
		Bool found = FALSE;
		Bool foundPrec = FALSE;
		Bool foundSphere = FALSE;
		PlaySpace_Surfel* lastSurfel = NULL;
		PlaySpace_Surfel* selectedSurfel = NULL;
		PlaySpace_Surfel* selectedSurfelPrec = NULL;
		Float selectedT;
		Float selectedTPrec;
		S16 hmin = S16((ymin - m_vMinAvailable.y) * invSizeVoxel);
		S16 hmax = S16((ymax - m_vMinAvailable.y) * invSizeVoxel);

		while (pSurfel)
		{
			S16 posy = pSurfel->y;
			if ((posy < hmin) || (posy > hmax))
			{
				pSurfel = pSurfel->pNext;		// Next
				continue;
			}

			Pos.y = (pSurfel->y * m_SizeVoxel) + m_vMinAvailable.y; //+ m_SizeVoxel*0.5f;
			Vec3f voxelCenter = Pos + CenterBias;

			//CollideBox.Set(QUAT_NULL, Pos + Vec3f(0.5f * m_SizeVoxel, 0.0f, 0.5f * m_SizeVoxel), BoxSize);
			//if (_DrawIt)
			//{
			//	DRAW_DEBUG_BOX3D_DIRECT_FAST(CollideBox, COLOR_RED);
			//}

			intersect[5] = intersect[4] = FALSE;
			if (y_ok)
			{
				t[4] = (Pos.y - p1.y) * inv_y;
				Float x = p1.x + (t[4] * View.x);
				Float z = p1.z + (t[4] * View.z);
				intersect[4] = (/*(t[4] > 0.0f) && (t[4] < 1.0f) &&*/ (x >= Pos.x) && (x <= (Pos.x + m_SizeVoxel)) && (z >= Pos.z) && (z <= (Pos.z + m_SizeVoxel)));
				t[5] = (Pos.y + m_SizeVoxel - p1.y) * inv_y;
				x = p1.x + (t[5] * View.x);
				z = p1.z + (t[5] * View.z);
				intersect[5] = (/*(t[5] > 0.0f) && (t[5] < 1.0f) &&*/ (x >= Pos.x) && (x <= (Pos.x + m_SizeVoxel)) && (z >= Pos.z) && (z <= (Pos.z + m_SizeVoxel)));
			}
			Bool collision = FALSE;
			Float tmin = 1e8f;
			Float tmax = -1e8f;
			for (S32 i = 0; i < 4; i++)
			{
				if (intersect[i] && (y[i] >= Pos.y) && (y[i] <= (Pos.y + m_SizeVoxel)))
				{
					collision = TRUE;
					if (t[i] < tmin)
						tmin = t[i];
					if (t[i] > tmax)
						tmax = t[i];
				}
			}
			for (S32 i = 4; i < 6; i++)
			{
				if (intersect[i])
				{
					collision = TRUE;
					if ((t[i] < tmin))
						tmin = t[i];
					if (t[i] > tmax)
						tmax = t[i];
				}
			}
			Vec3f entryPoint = p1 + (tmin * View);
			Vec3f exitPoint = p1 + (tmax * View);
			//DRAW_DEBUG_SPHERE3D(entryPoint, COLOR_GREEN, 0.02f);
			//DRAW_DEBUG_SPHERE3D(exitPoint, COLOR_BLUE, 0.02f);

			Bool foundSurfel = FALSE;
			Float minDistIn = 1e8f;
			Float minDistOut = 1e8f;
			Float selectedTMin = 1e8f;
			Float selectedTMax = 1e8f;
			while (pSurfel && (pSurfel->y == posy))
			{
				if (!(pSurfel->Flags & PlaySpace_Surfel::SURFFLAG_VIRTUAL))
				{
					Float dot = pSurfel->Normal * View;
					if (dot < 0.0f)
					{
						if (collision)
						{
							if (_precise)
							{
								foundSurfel = TRUE;
								lastSurfel = pSurfel;
								Float d1 = (entryPoint - pSurfel->Point) * pSurfel->Normal;
								Float d2 = (exitPoint - pSurfel->Point) * pSurfel->Normal;
								if (d1 < minDistIn)
								{
									minDistIn = d1;
									selectedTMin = tmin;
								}
								if (d2 < minDistOut)
								{
									minDistOut = d2;
									selectedTMax = tmax;
								}

								Float coef = ((pSurfel->Point - p1) * pSurfel->Normal) / dot;
								if ((coef >= 0.0f) && (coef <= 1.0f))
								{
									Vec3f coll = p1 + (coef * View);
									Float dist2 = (coll - voxelCenter).GetNorm2();
									if (dist2 < distThres)
									{
										if (!foundSphere)
										{
											selectedT = coef;
											selectedSurfel = pSurfel;
											foundSphere = TRUE;
										}
										else if (coef < selectedT)
										{
											selectedT = coef;
											selectedSurfel = pSurfel;
										}
									}
								}
							}
							else if ((tmin <= 1.0f) && (tmin >= 0.0f))
							{
								if (!found)
								{
									found = TRUE;
									selectedSurfel = pSurfel;
									selectedT = tmin;
								}
								else
								{
									if (tmin < selectedT)
									{
										selectedSurfel = pSurfel;
										selectedT = tmin;
									}
								}
							}
						}
					}
				}
				pSurfel = pSurfel->pNext;		// Next
			}
			if (_precise && foundSurfel)
			{
				if ((minDistIn < 0.0f) || ((minDistIn > 0.0f) && (minDistOut < 0.0f)))
				{
					Float tMin;
					if (minDistIn < 0.0f)
						tMin = selectedTMin;
					else
					{
						Float alpha = minDistIn / (minDistIn - minDistOut);
						tMin = selectedTMin + (alpha * (selectedTMax - selectedTMin));
					}
					if ((tMin <= 1.0f) && (tMin >= 0.0f))
					{
						if (!foundPrec)
						{
							foundPrec = TRUE;
							selectedSurfelPrec = lastSurfel;
							selectedTPrec = tMin;
						}
						else
						{
							if (tMin < selectedTPrec)
							{
								selectedSurfelPrec = lastSurfel;
								selectedTPrec = tMin;
							}
						}
					}
				}
			}
		}
		if (_precise)
		{
			if (foundPrec)
			{
				pResult = p1 + (selectedTPrec * View);
				if (_pZoneID)
					*_pZoneID = selectedSurfelPrec->ZoneId;
				if (_ppSurfel)
					*_ppSurfel = selectedSurfelPrec;

				return TRUE;
			}
			else if (foundSphere)
			{
				pResult = p1 + (selectedT * View);
				if (_pZoneID)
					*_pZoneID = selectedSurfel->ZoneId;
				if (_ppSurfel)
					*_ppSurfel = selectedSurfel;

				return TRUE;
			}
		}
		else if (found)
		{
			pResult = p1 + (selectedT * View);
			if (_pZoneID)
				*_pZoneID = selectedSurfel->ZoneId;
			if (_ppSurfel)
				*_ppSurfel = selectedSurfel;

			return TRUE;
		}
	}
	return FALSE;
}

/**************************************************************************/

Bool	PlaySpaceInfos_W::RayCastY(const Vec3f& p, Float fUpLength,RaycastY_Result* _pResult /*= NULL*/) const
{
	S32	CellX = FLOORINT((p.x-m_vMinAvailable.x) / m_SizeVoxel);
	if( CellX < 0 || CellX >= m_NbCellX )
		return FALSE;

	S32	CellZ = FLOORINT((p.z-m_vMinAvailable.z) / m_SizeVoxel);
	if( CellZ < 0 || CellZ >= m_NbCellY )
		return FALSE;

	if (!m_pSurfaceInfosV3 || m_pSurfaceInfosV3->m_Surfels.IsEmpty())
		return FALSE;

	Float MinY = 0.f;
	Float MaxY = 0.f;
	S8 Dir = 0;
	Vec3f vResultNormal = VEC3F_UP;
	Vec3f vDir = VEC3F_DOWN;
	if( fUpLength > 0.f )
	{
		Dir = PlaySpace_Surfel::SURFDIR_DOWN;
		MinY = p.y;
		MaxY = MinY + fUpLength;
		vResultNormal = VEC3F_DOWN;
		vDir = VEC3F_UP;
	}
	else
	{
		Dir = PlaySpace_Surfel::SURFDIR_UP;
		MaxY = p.y;
		MinY = MaxY + fUpLength;
		vResultNormal = VEC3F_UP;
		vDir = VEC3F_DOWN;
	}

	PlaySpace_CellInfos	*pCell = m_pSurfaceInfosV3->m_Surfels.GetCell(CellX,CellZ);
	if( pCell )
	{
		PlaySpace_Surfel	*pSurfel = pCell->pFirst;
		Bool	HaveCollide = FALSE;
		Float	CurDist = Abs(fUpLength);
		Vec3f	PtInter;
		while (pSurfel)
		{
			if( pSurfel->iDir == Dir 
				&& pSurfel->Point.y >= MinY 
				&& pSurfel->Point.y <= MaxY )
			{
				if (pSurfel->RayCast(p, vDir, PtInter, CurDist))
				{
					HaveCollide = TRUE;
					if (_pResult)
					{
						_pResult->m_vInter = Vec3f(p.x, PtInter.y, p.z);
						_pResult->m_vNormal = vResultNormal;
						_pResult->m_bIsBasin = pSurfel->IsBasin();
						S32 ZoneId = pSurfel->ZoneId;
						_pResult->m_ZoneId = ZoneId;

						// Shape Reco.
						if (m_ShapeAnalyzer.IsAnalysed() && (ZoneId >= 0) && (ZoneId<m_TabZoneInfos.GetSize()))
							_pResult->m_bIsCouch = m_TabZoneInfos[ZoneId].IsCouch;
					}
				}
			}
			pSurfel = pSurfel->pNext;
		}

		return HaveCollide;
	}

	return FALSE;
}

/**************************************************************************/

Bool	PlaySpaceInfos_W::IsCollidingSphere(const Vec3f &p,Float r)
{
	if (!m_pSurfaceInfosV3 || m_pSurfaceInfosV3->m_Surfels.IsEmpty())
		return FALSE;

	S32 SX = (S32)((p.x - m_vMinAvailable.x - r) / m_SizeVoxel + 0.5f);		// + 0.5f, because cell is expand for center.
	S32 SY = (S32)((p.z - m_vMinAvailable.z - r) / m_SizeVoxel + 0.5f);

	SX = Max((S32)0,SX);
	SY = Max((S32)0,SY);

	S32 EX = (S32)((p.x - m_vMinAvailable.x + r) / m_SizeVoxel + 0.5f);
	S32 EY = (S32)((p.z - m_vMinAvailable.z + r) / m_SizeVoxel + 0.5f);

	EX = Min(m_NbCellX-1,EX);
	EY = Min(m_NbCellY-1,EY);

	Box_Z				CollideBox;
	Sphere_Z			SphereCol(p,r);
	Vec3f				BoxSize(m_SizeVoxel,m_SizeVoxel,m_SizeVoxel);
	CollisionReport_Z	CollideReport;

	for (S32 y=SY ; y<=EY ; y++)
	{
		for (S32 x=SX ; x<=EX ; x++)
		{
			Vec3f Pos((Float)x*m_SizeVoxel + m_vMinAvailable.x,0.f,(Float)y*m_SizeVoxel + m_vMinAvailable.z);

			PlaySpace_CellInfos	*pCell = &(m_pSurfaceInfosV3->m_Surfels.m_CellBoard[y * m_NbCellX + x]);
			PlaySpace_Surfel	*pSurfel = pCell->pFirst;
			while (pSurfel)
			{
				// Collision...
				Pos.y = (pSurfel->y * m_SizeVoxel) + m_vMinAvailable.y; //+ m_SizeVoxel*0.5f;
				CollideBox.Set(QUAT_NULL,Pos,BoxSize);
				if (SphereVsBox(SphereCol,CollideBox))
					return TRUE;

				// Next 
				pSurfel = pSurfel->pNext;
			}
		}
	}

	return FALSE;
}

/**************************************************************************/

Bool	PlaySpaceInfos_W::SnapToEdge(Vec3f &_pos, const Vec3f &_Dir, Float DistFromEdge, Float DistMax, Bool _OkForWall, Bool _OkForEdge, Bool _OkForVirtual)
{
	//if (g_TypeScan & PSI_SCAN_NEW)
	//	return FALSE;

	if (!m_pSurfaceInfosV3)
		return FALSE;

	Vec3f	Dest = _pos + (_Dir * DistMax);

	Vec2f Start,End;
	Start.x = (_pos.x - m_vMinAvailable.x) / m_SizeVoxel;
	Start.y = (_pos.z - m_vMinAvailable.z) / m_SizeVoxel;

	End.x = (Dest.x - m_vMinAvailable.x) / m_SizeVoxel;
	End.y = (Dest.z - m_vMinAvailable.z) / m_SizeVoxel;

	Bresenham2DInt_Z	MyBres;
	MyBres.InitSead(Start.x + 0.01f,Start.y + 0.01f,End.x + 0.01f,End.y + 0.01f);
	S32	X,Y;

	Float DeltaX = 0.f;
	Float DeltaY = 0.f;
	if (_Dir.x > 0.5f)
		DeltaX = -0.5f;
	else if (_Dir.x < -0.5f)
		DeltaX = 0.5f;
	else
		DeltaX = 0.f;

	if (_Dir.z > 0.5f)
		DeltaY = -0.5f;
	else if (_Dir.z < -0.5f)
		DeltaY = 0.5f;
	else
		DeltaY = 0.f;

	if (!MyBres.GetNextPointSead(X,Y))
		return FALSE;
	if (   (X < 0)
		|| (Y < 0)
		|| (X >= m_NbCellX)
		|| (Y >= m_NbCellY)
		)
		return FALSE;

	Float	RefY = _pos.y;//-1e20f;

	while (MyBres.GetNextPointSead(X,Y))
	{
		// Get Cell.
		if (   (X < 0)
			|| (Y < 0)
			|| (X >= m_NbCellX)
			|| (Y >= m_NbCellY)
			)
		{
			// Out of playspace => virtual no EDGE.
/*			// Collide !
			_pos.x = (((Float)X)+DeltaX)*m_SizeVoxel + m_vMinAvailable.x;
			_pos.z = (((Float)Y)+DeltaY)*m_SizeVoxel + m_vMinAvailable.z;
			_pos.y = RefY;

			_pos -= _Dir*DistFromEdge;*/

			return FALSE;
		}	

		PlaySpace_CellInfos	*pCell = &(m_pSurfaceInfosV3->m_Surfels.m_CellBoard[Y * m_NbCellX + X]);
//	DRAW_DEBUG_SPHERE3D(Vec3f(pCell->fx + m_SizeVoxel*0.5f, _pos.y + 0.05f, pCell->fz + m_SizeVoxel*0.5f), COLOR_GREEN, 0.015f, .displayDuration(30000.f));
		PlaySpace_Surfel	*pSurfel = pCell->pFirst;
		Bool Find = FALSE;

		PlaySpace_Surfel	*pSurfelBetterGround = NULL;
		PlaySpace_Surfel	*pSurfelBetterWall = NULL;

		while (pSurfel)
		{
			// Collision...
			if (pSurfel->iDir == PlaySpace_Surfel::SURFDIR_UP)
			{
				// Ground.
				Float DeltaH = RefY - pSurfel->Point.y;
				if (DeltaH > -0.07f)	// Accept Surfel upper than RefY.
				{
					if (!pSurfelBetterGround || (pSurfel->Point.y > pSurfelBetterGround->Point.y))
						pSurfelBetterGround = pSurfel;
				}
			}
			else if ((pSurfel->iDir == PlaySpace_Surfel::SURFDIR_LEFT) ||
				(pSurfel->iDir == PlaySpace_Surfel::SURFDIR_RIGHT) ||
				(pSurfel->iDir == PlaySpace_Surfel::SURFDIR_BACK) ||
				(pSurfel->iDir == PlaySpace_Surfel::SURFDIR_FRONT) )
			{
				if (_OkForVirtual || !(pSurfel->Flags & PlaySpace_Surfel::SURFFLAG_VIRTUAL))
				{
					Float DeltaH = pSurfel->Point.y - RefY;
					if ((DeltaH > 0.08f) && (DeltaH < 0.24f))
					{
						if (!pSurfelBetterWall || (pSurfel->Point.y > pSurfelBetterWall->Point.y))
							pSurfelBetterWall = pSurfel;
					}
				}
			}
			// Next 
			pSurfel = pSurfel->pNext;
		}

		// Ground or Hole.
		if (pSurfelBetterGround && ((RefY - pSurfelBetterGround->Point.y) < 0.16f))
			pSurfelBetterGround = NULL;

		// Exit if non wanted type
		if (!_OkForWall && pSurfelBetterWall)
			return FALSE;
		if (!_OkForEdge && pSurfelBetterGround)
			return FALSE;

		// Wall Detect ?
		if (pSurfelBetterGround)
		{
			// Collide.
			_pos.x = (((Float)X) + DeltaX)*m_SizeVoxel + m_vMinAvailable.x;
			_pos.z = (((Float)Y) + DeltaY)*m_SizeVoxel + m_vMinAvailable.z;
			_pos.y = RefY;
			_pos -= _Dir*(DistFromEdge + m_SizeVoxel*0.5f);
			return TRUE;
		}

		if (pSurfelBetterWall)
		{
			Vec3f	StartPos(_pos.x,pSurfelBetterWall->Point.y,_pos.z);
			Float	ResultDist = DistMax;
			Vec3f	PtInter;
			if (pSurfelBetterWall->RayCast(StartPos, _Dir, PtInter, ResultDist))
			{
				_pos = _pos + (ResultDist - DistFromEdge) * _Dir;
				_pos.y = RefY;
				return TRUE;
			}
		}
	}

	return FALSE;
}

/**************************************************************************/

U32		PlaySpaceInfos_W::ComputeCRC()
{
	U32 MyCRC = Name_Z::GetID((U8*)(&(this->CRC_ZoneStart)),&(this->CRC_ZoneEnd) - &(this->CRC_ZoneStart),0);

	return MyCRC;
}

/**************************************************************************/

void	PlaySpaceInfos_W::ComputeOneBorderV3(Vec3fDA &TabPoints,S32 x,S32 y,S32 _iLevel,S32 Dx,S32 Dy,S32 Ex,S32 Ey,S32 SizeX)
{
	PlaySpace_SurfaceInfosV3 *pSurface = m_pSurfaceInfosV3;

	Bool	HaveStart = FALSE;
	Vec3f	PosStart;
	S32		NbPoint = 0;
	Bool	TheEnd = FALSE;

	do {
		// Get Height and Border...

		S32 IsBorder = 1;
		Float	H_Up = -1000.f;
		Float	H_Down = 1000.f;
		PlaySpace_CellInfos *pCell = pSurface->m_Surfels.m_CellBoard.GetArrayPtr() + y * SizeX + x;

		// Scan V3
		if (!pCell->FindBiggestHole(H_Down, H_Up))
			IsBorder = 0;
		
		Float hCur;
		if (!_iLevel)
			hCur = H_Down;
		else
			hCur = H_Up;

		// Is End ?
		TheEnd = ((x==Ex) && (y==Ey));

		// Point is Ok ?
		Bool PointIsOk = FALSE;
		if (!TheEnd && IsBorder)
		{
			PointIsOk = TRUE;
			if (HaveStart)
			{
				Float h = PosStart.y / (Float)NbPoint;
				if (Abs(h - hCur) > 0.15f)
					PointIsOk = FALSE;
			}
		}

		// Add or Draw.
		if (HaveStart)
		{
			if (PointIsOk)
			{
				// One more point
				PosStart.y += hCur;
				NbPoint++;
			}
			else
			{
				// Cut !
				Float h = PosStart.y / (Float)NbPoint;
				PosStart.y = h;
				Vec3f PosEnd;
				PosEnd.x = pCell->fCornerX;
				PosEnd.y = h;
				PosEnd.z = pCell->fCornerZ;
				if ((PosEnd - PosStart).HGetNorm2() > (0.25f * 0.25f))
				{
					TabPoints.Add(PosStart);
					TabPoints.Add(PosEnd);
				}
				HaveStart = FALSE;
			}
		}

		// Start or Restart.
		if (!HaveStart && IsBorder)
		{
			// First Point.
			PosStart.x = pCell->fCornerX;
			PosStart.y = hCur;
			PosStart.z = pCell->fCornerZ;
			HaveStart = TRUE;
			NbPoint = 1;
		}

		// Next.
		x += Dx;
		y += Dy;
	} while (!TheEnd);
}

/**********************************************************/

static void MoveBorderPoint(Vec3fDA &TabPoints,S32 _FromId,Vec3f Delta)
{
	S32		StartId = _FromId;
	S32		EndId = TabPoints.GetSize();

	while (StartId < EndId)
	{
		TabPoints[StartId] += Delta;
		StartId++;
	}
}

Bool	PlaySpaceInfos_W::GetBorder(Vec3fDA &_TabPoints,Bool _Ground,Float _DistMargin)
{
	// Line.
	Float DeltaBorder = 0.08f;

	if (!m_pSurfaceInfosV3)
		return FALSE;

	S32 OldSize = _TabPoints.GetSize();
	S32 SizeX = m_NbCellX;
	S32 SizeY = m_NbCellY;

	S32 iLevel;
	if (_Ground)
		iLevel = 0;
	else
		iLevel = 1;

	ComputeOneBorderV3(_TabPoints,0      ,      0,iLevel, 1, 0,SizeX-1,      0,SizeX);
	MoveBorderPoint(_TabPoints,OldSize,Vec3f(0.f,0.f,-_DistMargin));
	OldSize = _TabPoints.GetSize();

	ComputeOneBorderV3(_TabPoints,SizeX-1,      0,iLevel, 0, 1,SizeX-1,SizeY-1,SizeX);
	MoveBorderPoint(_TabPoints,OldSize,Vec3f(_DistMargin+DeltaBorder,0.f,0.f));
	OldSize = _TabPoints.GetSize();

	ComputeOneBorderV3(_TabPoints,SizeX-1,SizeY-1,iLevel,-1, 0,      0,SizeY-1,SizeX);
	MoveBorderPoint(_TabPoints,OldSize,Vec3f(0.f,0.f,_DistMargin+DeltaBorder));
	OldSize = _TabPoints.GetSize();

	ComputeOneBorderV3(_TabPoints,      0,SizeY-1,iLevel, 0,-1,      0,      0,SizeX);
	MoveBorderPoint(_TabPoints,OldSize,Vec3f(-_DistMargin,0.f,0.f));
	OldSize = _TabPoints.GetSize();

	return TRUE;
}

/**********************************************************/

void	PlaySpaceInfos_W::ActivateTopology()
{
	if (m_TopologyActivated)
		return;

	m_TopologyActivated = TRUE;
	UpdateTools(FALSE,FALSE,FALSE);
}
void	PlaySpaceInfos_W::ActivateShape()
{
	if (m_ShapeActivated)
		return;

	m_ShapeActivated = TRUE;
	UpdateTools(FALSE,FALSE,FALSE);
}

/**********************************************************/

void	PlaySpaceInfos_W::DeActivateTopology()
{
	if (!m_TopologyActivated)
		return;
	m_TopologyActivated = FALSE;
	m_TopologyAnalyzer.Reset();
}

void	PlaySpaceInfos_W::DeActivateShape()
{
	if (!m_ShapeActivated)
		return;
	m_TopologyActivated = FALSE;
	m_ShapeAnalyzer.Reset();
}

/**********************************************************/

void	PlaySpaceInfos_W::SetShapeDescriptor(CallBack_CreateShapeDesc _pCallBack)
{
	m_pCallBack_CreateDecriptors = _pCallBack;
	UpdateTools(FALSE,FALSE,FALSE);
}

/**********************************************************/

void	PlaySpaceInfos_W::UpdateTools(Bool _PlaySpaceChanged,Bool _ForceOnLoad,Bool _CanUseJob)
{
	if (_PlaySpaceChanged)
		m_bRefreshZoneInfos = TRUE;

	// Topology Update.
	if (m_TopologyActivated)
	{
		if (_PlaySpaceChanged)
		{
			m_TopologyAnalyzer.Reset();
			m_ShapeAnalyzer.Reset();
		}

		if (!m_TopologyAnalyzer.IsAnalysed())
		{
			// Have to be Analysed.
			if (IsFinalized() || _ForceOnLoad)
			{
				m_TopologyAnalyzer.Analyze(this,_CanUseJob);
				m_bRefreshZoneInfos = TRUE;
			}
		}
	}
	else
	{
		if (m_TopologyAnalyzer.IsAnalysed())
		{
			m_TopologyAnalyzer.Reset();
			m_ShapeAnalyzer.Reset();
			m_bRefreshZoneInfos = TRUE;
		}
	}

	// Shape.
	if (m_ShapeActivated)
	{
		if (_PlaySpaceChanged)
			m_ShapeAnalyzer.Reset();

		if (!m_ShapeAnalyzer.IsAnalysed())
		{
			if (m_TopologyActivated && m_TopologyAnalyzer.IsAnalysed() && (IsFinalized() || _ForceOnLoad))
			{
				// Create Descriptors if needed
				if (m_pCallBack_CreateDecriptors!=NULL)
				{
					m_ShapeAnalyzer.Reset();
					(*m_pCallBack_CreateDecriptors)(m_ShapeAnalyzer);
				}

				// Analyse !
				m_ShapeAnalyzer.Analyse();
				m_bRefreshZoneInfos = TRUE;
			}
		}
	}
	else
	{
		if (m_ShapeAnalyzer.IsAnalysed())
		{
			m_ShapeAnalyzer.Reset();
			m_bRefreshZoneInfos = TRUE;
		}
	}

	if (m_bRefreshZoneInfos && (IsFinalized() || _ForceOnLoad))
	{
		RefreshTabZoneInfos();
	}
}

/**********************************************************/

void	PlaySpaceInfos_W::RefreshTabZoneInfos()
{
	if (m_TopologyActivated && !m_TopologyAnalyzer.IsAnalysed())
		return;
				
	if (m_ShapeActivated && !m_ShapeAnalyzer.IsAnalysed())
		return;
	
	m_TabZoneInfos.Flush();

	for (S32 i=0 ; i<m_TopologyAnalyzer.m_daSurfaces.GetSize() ; i++)
	{
		S32 ZoneId = m_TopologyAnalyzer.m_daSurfaces[i].m_iZoneId;
		if (ZoneId < 0)
			continue;

		if (ZoneId >= m_TabZoneInfos.GetSize())
		{
			S32 iCur = m_TabZoneInfos.GetSize();
			m_TabZoneInfos.SetSize(ZoneId+1);
			while (iCur <= ZoneId)
			{
				m_TabZoneInfos[iCur].ShapeName = NAME_NULL;
				m_TabZoneInfos[iCur].SlotName = NAME_NULL;
				m_TabZoneInfos[iCur].TopologyId = -1;
				m_TabZoneInfos[iCur].IsCouch = FALSE;
				iCur++;
			}
		}

		m_TabZoneInfos[ZoneId].TopologyId = i;
		m_TabZoneInfos[ZoneId].IsCouch = FALSE;
		if (m_ShapeAnalyzer.GetShapeFromZoneId(ZoneId,m_TabZoneInfos[ZoneId].ShapeName,m_TabZoneInfos[ZoneId].SlotName))
		{
			if (ShapeReco::ShapeAnalyzer_W::ShapeDesc_IsItCouch(m_TabZoneInfos[ZoneId].ShapeName,m_TabZoneInfos[ZoneId].SlotName))
				m_TabZoneInfos[ZoneId].IsCouch = TRUE;
		}
	}

	m_bRefreshZoneInfos = FALSE;
}

/**********************************************************/

Bool	PlaySpaceInfos_W::FilterMeshFromSR()
{
	if (m_pSurfaceSR && (m_pSurfaceSR->GetFrameUpdate()>0))
	{
		m_TheMeshSR.RemoveIsolatedFaces(30);
		m_TheMeshSR.PlanarFilter_AccurateNew();
	}
	return TRUE;
}

/**********************************************************/

Bool	PlaySpaceInfos_W::RefreshSurfaceSR(Playspace_SR_DeviceSR *_pDeviceSR,Bool _OnlyIfNew, Bool _UseFilter, Bool _RefreshBlind)
{
	if (!m_pSurfaceSR)
		return FALSE;

	// Resfresh SR !
	if (_pDeviceSR)
	{
		// This is real Time Refresh.
		m_pSurfaceSR->RefreshSRFromDevice(_pDeviceSR, _UseFilter);
	}
	else
	{
		// Static Update.
		switch (m_pSurfaceSR->GetTypeRefresh())
		{
			case REFRESH_SR_FILE:
			case REFRESH_SR_SCENE:
				m_pSurfaceSR->RefreshSRFromStaticDatas();
				break;
			default:
				EXCEPTIONC_Z(FALSE, "NO SR INIT");
				break;
		}
	}

	if (_RefreshBlind)
		m_pSurfaceSR->RefreshBlindMapFromDevice();

	return TRUE;
}

/**********************************************************/

Bool	PlaySpaceInfos_W::RefreshMeshFromSurfaceSR()
{
	// Reinit Mesh
	m_TheMeshSR.Flush(TRUE);
	m_TheMeshSR.SetEditMode(TRUE);
	m_pSurfaceSR->ThreadSafe_ExtractSRToMesh(m_TheMeshSR, TRUE);
	if (!m_TheMeshSR.m_TabQuad.GetSize())
		return FALSE;

	return TRUE;
}

/**********************************************************/


