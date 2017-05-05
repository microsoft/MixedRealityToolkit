// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include <pch.h>
#include <UnderstandingMgr_W.h>
#include <System_Z.h>

/**********************************************************/
static UnderstandingMgr_W *pUnderstandingMgr = NULL;

UnderstandingMgr_W &UnderstandingMgr_W::GetUnderstandingMgr()
{
	if (!pUnderstandingMgr)
		pUnderstandingMgr = New_Z UnderstandingMgr_W();

	return *pUnderstandingMgr;
}

/**********************************************************/

void UnderstandingMgr_W::ReleaseUnderstandingMgr()
{
	if (pUnderstandingMgr)
	{
		Delete_Z pUnderstandingMgr;
		pUnderstandingMgr = NULL;
	}
}

/**********************************************************/

void	UnderstandingMgr_W::ResetFrameTransfo(const Quat &_NewTransfo)
{
	m_FrameQuat = _NewTransfo;
	m_FrameInvQuat = -m_FrameQuat;
}

/**********************************************************/

UnderstandingMgr_W::UnderstandingMgr_W()
{
}

/**********************************************************/

UnderstandingMgr_W::~UnderstandingMgr_W()
{
	m_PlaySpaceInfos.Reset();
}

/**********************************************************/

void UnderstandingMgr_W::Init()
{
	CHECKFTOLMODE();

	// Met les descrpteurs pas défaut.
	m_PlaySpaceInfos.SetPlayspaceSR(&m_PlayspaceSR);

	// Init.
	m_DoAlignWorldCallBack = NULL;
	m_FrameMode = Unknow;
	ResetFrameTransfo();

	// Default to inside frame mode.
	SetFrameMode(FrameMode::ManageInside, NULL);

	// Activate Tools.
	m_PlaySpaceInfos.ActivateTopology();
}

/**********************************************************/

void UnderstandingMgr_W::Reset()
{
	m_PlaySpaceInfos.Reset();
}

/**********************************************************/

void UnderstandingMgr_W::Shut()
{
	m_PlaySpaceInfos.Reset();
}

/**********************************************************/

Bool UnderstandingMgr_W::MarkHandles()
{
	return TRUE;
}

/**********************************************************/

void UnderstandingMgr_W::ActivateShapeAnalysis()
{
	m_PlaySpaceInfos.ActivateShape();
}

/**********************************************************/

void UnderstandingMgr_W::Update(Float dTime)
{
	CHECKFTOLMODE();

	// Update Playspace.
	m_PlaySpaceInfos.UpdatePlaySpace(this, dTime);

	// Frame count
	IncGlobalFrameCount();
}

/**********************************************************/

bool UnderstandingMgr_W::SetSRMeshAnReset(Dll_Interface::MeshData* _TabMesh, S32 _NbMesh)
{
	CHECKFTOLMODE();

	// Reset Frame.
	ResetFrameTransfo();

	// First, inject Mesh in SR.
	if (!m_PlayspaceSR.SetSRMesh(_TabMesh, _NbMesh, m_FrameQuat))
		return FALSE;

	return TRUE;
}

/**********************************************************/

bool	UnderstandingMgr_W::OnePassScanOnCurrentSR(const Vec3f &_CameraPos, const Vec3f &_CameraDir, Float _MaxPlayspaceSize)
{
	CHECKFTOLMODE();

	m_PlaySpaceInfos.SetPlayspaceSR(&m_PlayspaceSR);

	// Set Camera (to be modified)
	Vec3f normedCameraDir;
	Vec3f normedCameraDirUp;

	normedCameraDir.CNormalize();
	normedCameraDirUp = normedCameraDir ^ (normedCameraDir^VEC3F_UP);
	normedCameraDirUp.CNormalize();

	Util_L::SetCurrentCamera(_CameraPos, normedCameraDir, normedCameraDirUp);
	if (m_FrameMode == ManageInside)
		Util_L::ApplyTranfoToCamera(m_FrameQuat);

	// Set Scan mode : MESH and NEW (to be suppressed)
	PlaySpaceInfos_W::g_TypeScan &= ~PlaySpaceInfos_W::PSI_SCAN_OLD;
	PlaySpaceInfos_W::g_TypeScan |= PlaySpaceInfos_W::PSI_SCAN_NEW;
	PlaySpaceInfos_W::g_TypeScan |= PlaySpaceInfos_W::PSI_SCAN_MESH;

	if (!m_PlaySpaceInfos.OnePassComputePlaySpace(_CameraPos, _CameraDir, _MaxPlayspaceSize))
		return FALSE;

	// Report Align.
	ResetFrameTransfo(m_FrameQuat * m_PlaySpaceInfos.m_AlignTransfo);

	return TRUE;
}

/**********************************************************/

bool	UnderstandingMgr_W::OnePassScan(const Vec3f &_CameraPos, const Vec3f &_CameraDir, Float _MaxPlayspaceSize, U32 *_pTabTriIdx, S32 _NbTriIdx, Vec3f *_pTabVtx, S32 _NbVtx)
{
	CHECKFTOLMODE();

	// Reset Frame Transfo.
	ResetFrameTransfo();

	// First, inject Mesh in SR.
	if (!m_PlayspaceSR.SetSRMesh(_pTabTriIdx, _NbTriIdx, _pTabVtx, _NbVtx))
		return FALSE;

	return OnePassScanOnCurrentSR(_CameraPos, _CameraDir, _MaxPlayspaceSize);
}

/**********************************************************/

bool	UnderstandingMgr_W::OnePassScan(const Vec3f &_CameraPos, const Vec3f &_CameraDir, Float _MaxPlayspaceSize, Dll_Interface::MeshData* _TabMesh, S32 _NbMesh)
{
	CHECKFTOLMODE();

	// Reset Frame Transfo.
	ResetFrameTransfo();

	// First, inject Mesh in SR.
	if (!m_PlayspaceSR.SetSRMesh(_TabMesh, _NbMesh, m_FrameQuat))
		return FALSE;

	return OnePassScanOnCurrentSR(_CameraPos, _CameraDir, _MaxPlayspaceSize);
}

/**********************************************************/

bool	UnderstandingMgr_W::OnePassScan(const Vec3f &_CameraPos, const  Vec3f &_CameraDir, Float _MaxPlayspaceSize, char *_FileName)
{
	CHECKFTOLMODE();

	// Reset Frame Transfo.
	ResetFrameTransfo();

	// First, inject Mesh in SR.
	if (!m_PlayspaceSR.LoadSRMeshRaw(_FileName))
		return FALSE;

	return OnePassScanOnCurrentSR(_CameraPos, _CameraDir, _MaxPlayspaceSize);
}

/**********************************************************/

Bool	UnderstandingMgr_W::InitScan(const Vec3f& _CameraPos, const Vec3f& _CameraDir, Float _fSearchDist, Float _OptimalSize)
{
	CHECKFTOLMODE();

	// Set Scan mode : MESH and NEW (to be suppressed)
	PlaySpaceInfos_W::g_TypeScan &= ~PlaySpaceInfos_W::PSI_SCAN_OLD;
	PlaySpaceInfos_W::g_TypeScan |= PlaySpaceInfos_W::PSI_SCAN_NEW;
	PlaySpaceInfos_W::g_TypeScan |= PlaySpaceInfos_W::PSI_SCAN_MESH;

	// Init the process
	m_AlignTransfoId = -1;
	m_RefAlignTransfoId = -1;
	return m_PlaySpaceInfos.InitProcess(_CameraPos, _CameraDir, _fSearchDist, _OptimalSize);
}

/**********************************************************/

PlaySpaceInfos_W::PSI_State	UnderstandingMgr_W::UpdateScanFromStaticDatas(const Vec3f& _CameraPos, const Vec3f& _CameraDir, const Vec3f& _CameraDirUp, Float _dTime, PlaySpaceInfos_W::PSI_UpdateMode _Mode, Float _SurfaceValidation)
{
	return UpdateScanFromDeviceSR(NULL, _CameraPos, _CameraDir, _CameraDirUp, _dTime, _Mode, _SurfaceValidation);
}

/**********************************************************/

PlaySpaceInfos_W::PSI_State	UnderstandingMgr_W::UpdateScanFromDeviceSR(Playspace_SR_DeviceSR *_pDeviceSR, const Vec3f& _CameraPos, const Vec3f& _CameraDir, const Vec3f& _CameraDirUp, Float _dTime, PlaySpaceInfos_W::PSI_UpdateMode _Mode, Float _SurfaceValidation)
{
	CHECKFTOLMODE();

	Util_L::SetCurrentCamera(_CameraPos, _CameraDir, _CameraDirUp);

	// If we are doing a static scan, we will not have an SR device (want to support both)
	PlaySpaceInfos_W::PSI_State CurState = PlaySpaceInfos_W::PSI_State_NonInitialized;
	if (_pDeviceSR == NULL)
	{
		if (m_FrameMode == ManageInside)
			Util_L::ApplyTranfoToCamera(m_FrameQuat);
		CurState = m_PlaySpaceInfos.UpdateScanV3_Unified(_pDeviceSR, _dTime, _Mode, _SurfaceValidation, m_AlignTransfoId);
	}
	else
	{
		Playspace_SR_DeviceSR AlignDeviceSR = *_pDeviceSR;
		if (m_FrameMode == ManageInside)
		{
			Util_L::ApplyTranfoToCamera(m_FrameQuat);
			Mat4x4	AlignMatrix;
			m_FrameQuat.GetMatrix(AlignMatrix);
			AlignDeviceSR.GlobalTransfo = AlignMatrix * AlignDeviceSR.GlobalTransfo;
		}
		CurState = m_PlaySpaceInfos.UpdateScanV3_Unified(&AlignDeviceSR, _dTime, _Mode, _SurfaceValidation, m_AlignTransfoId);
	}

	if (CurState == PlaySpaceInfos_W::PSI_State::PSI_State_Scanning_Align)
	{
		if (m_RefAlignTransfoId != m_AlignTransfoId)
		{
			// Do Align ONCE !
			if (m_FrameMode == ManageInside)
			{
				ResetFrameTransfo(m_FrameQuat * m_PlaySpaceInfos.m_AlignTransfo);
				m_RefAlignTransfoId = m_AlignTransfoId;
			}
			else if ((m_FrameMode == ShareFrame) && m_DoAlignWorldCallBack)
			{
				(*m_DoAlignWorldCallBack)(m_PlaySpaceInfos.GetAutoFitTransfo());
				m_RefAlignTransfoId = m_AlignTransfoId;
			}
		}
	}
	if (CurState == PlaySpaceInfos_W::PSI_State::PSI_State_Scanning_Finish)
	{
		m_PlaySpaceInfos.FinalizeScan();
	}
	return CurState;
}


/**********************************************************/

void	UnderstandingMgr_W::RequestFinishScan(Bool _ForceAbort)
{
	CHECKFTOLMODE();

	if (m_PlaySpaceInfos.m_ScanningJobsLastStats.MeshStats.HorizSurface < 0.5f)
		m_PlaySpaceInfos.RequestFinishUnifiedScanning(true);
	else
		m_PlaySpaceInfos.RequestFinishUnifiedScanning(_ForceAbort);
}

/**********************************************************/

void	UnderstandingMgr_W::SetFrameMode(FrameMode _Mode, CallBack_DoAlignWorld pFunction)
{
	CHECKFTOLMODE();

	if (_Mode == ShareFrame)
	{
		EXCEPTIONC_Z(pFunction != NULL, "CallBack have to be set !");
		SetDoAlignWorldCallBack(pFunction);
	}

	if (_Mode == m_FrameMode)
		return;
	m_FrameMode = _Mode;

	if (_Mode == ManageInside)
	{
		SetDoAlignWorldCallBack(NULL);
	}
	if (_Mode == ShareFrame)
	{
		// Report Transfo to outside World.
		pFunction(m_FrameQuat);
		m_FrameQuat = QUAT_NULL;
	}
}


