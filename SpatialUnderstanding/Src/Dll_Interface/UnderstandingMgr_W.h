// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#ifndef _UNDERSTANDINGMGR_W
#define _UNDERSTANDINGMGR_W

#include <PlaySpace_SR_W.h>
#include <PlaySpaceInfos_W.h>
#include <FRGSolver/FRGSolver_W.h>

class PlaySpaceInfos_W;
typedef void(*CallBack_DoAlignWorld)(Quat &_Trnasfo);

class UnderstandingMgr_W
{
public:
	typedef enum {
		Unknow,			// Don't use that.
		ManageInside,	// Manager have a specific frame (transfo at every data exchange : SLOW MODE).
		ShareFrame		// Manager have same frame than outside (Use the callback to manage Frame change due to Alignment)
	} FrameMode;

protected:
	// Playspace and Mgr.
	Playspace_SR_W			m_PlayspaceSR;
	PlaySpaceInfos_W		m_PlaySpaceInfos;
	S32						m_AlignTransfoId;
	S32						m_RefAlignTransfoId;

	// Frame Mode.
	CallBack_DoAlignWorld	m_DoAlignWorldCallBack;
	FrameMode				m_FrameMode;
	Quat					m_FrameQuat;
	Quat					m_FrameInvQuat;
	void					ResetFrameTransfo(const Quat &_NewTransfo = QUAT_NULL);

	// Solver.
	FRGSolver_W				m_Solver;

public:

	// Basis Manager
	UnderstandingMgr_W();
	virtual ~UnderstandingMgr_W();

	Playspace_SR_W&			GetPlayspaceSR() { return m_PlayspaceSR; }
	Playspace_SR_W*			GetPlayspaceSRPtr() { return &m_PlayspaceSR; }

	PlaySpaceInfos_W&		GetPlayspaceInfos() { return m_PlaySpaceInfos; }
	PlaySpaceInfos_W*		GetPlayspaceInfosPtr() { return &m_PlaySpaceInfos; }

	FRGSolver_W&			GetSolver() { return m_Solver; }

	void					FlushSRMesh() { m_PlayspaceSR.Flush(FALSE); }

	virtual void			Init();
	virtual void			Shut();
	virtual void			Reset();

	virtual Bool			MarkHandles();
	virtual	void			Update(Float dTime);

	void					ActivateShapeAnalysis();

	// Add meshes (Only for static process).
	bool					SetSRMeshAnReset(Dll_Interface::MeshData* _TabMesh, S32 _NbMesh);

	// Do a one pass scan (result in PlaySpaceInfos_W)
	// => GetPlayspaceInfos().GetSurfaceInfosV3() for access to mesh or simplified mesh (MESH could contain quads !)
	// Access direct to PlaySpaceInfos_W for min/max, etc
	bool					OnePassScanOnCurrentSR(const Vec3f &_CameraPos, const Vec3f &_CameraDir, Float _MaxPlayspaceSize);
	bool					OnePassScan(const Vec3f &_CameraPos, const  Vec3f &_CameraDir, Float _MaxPlayspaceSize, U32 *_pTabTriIdx, S32 _NbTriIdx, Vec3f *_pTabVtx, S32 _NbVtx);
	bool					OnePassScan(const Vec3f &_CameraPos, const  Vec3f &_CameraDir, Float _MaxPlayspaceSize, Dll_Interface::MeshData* _TabMesh, S32 _NbMesh);
	bool					OnePassScan(const Vec3f &_CameraPos, const  Vec3f &_CameraDir, Float _MaxPlayspaceSize, char *_FileName);

	// Set FrameMode.
	void					SetFrameMode(FrameMode _Mode, CallBack_DoAlignWorld pFunction);
	void					SetDoAlignWorldCallBack(CallBack_DoAlignWorld pFunction) { m_DoAlignWorldCallBack = pFunction; }

	Quat					GetFrameTransfoForInput() { return m_FrameQuat; }
	Mat4x4					GetFrameTransfoForInputMat() { Mat4x4 mat; m_FrameQuat.GetMatrix(mat); return mat; }
	Quat					GetFrameTransfoForOutput() { return m_FrameInvQuat; }
	Mat4x4					GetFrameTransfoForOutputMat() { Mat4x4 mat; m_FrameInvQuat.GetMatrix(mat); return mat; }

	// Do a real time scan
	// For the moment, SR have to be inserted BEFORE Init => Use SetSRMesh.
	Bool					InitScan(const Vec3f& _CameraPos, const Vec3f& _CameraDir, Float _fSearchDist = 7.f, Float _OptimalSize = 7.f);
	PlaySpaceInfos_W::PSI_State	UpdateScanFromStaticDatas(const Vec3f& _CameraPos, const Vec3f& _CameraDir, const Vec3f& _CameraDirUp, Float _dTime, PlaySpaceInfos_W::PSI_UpdateMode _Mode, Float _SurfaceValidation);
	PlaySpaceInfos_W::PSI_State	UpdateScanFromDeviceSR(Playspace_SR_DeviceSR *_pDeviceSR,const Vec3f& _CameraPos, const Vec3f& _CameraDir, const Vec3f& _CameraDirUp, Float _dTime, PlaySpaceInfos_W::PSI_UpdateMode _Mode, Float _SurfaceValidation);

	void					RequestFinishScan(Bool _ForceAbort = FALSE);

	// Singleton of UnderstandingMgr_W
	static UnderstandingMgr_W& GetUnderstandingMgr();
	static void				ReleaseUnderstandingMgr();
};

#endif
