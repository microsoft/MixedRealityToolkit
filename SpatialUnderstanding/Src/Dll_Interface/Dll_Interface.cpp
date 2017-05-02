// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include <pch.h>
#include <Objbase.h>
#include <Dll_Interface\Dll_Interface.h>
#include <UnderstandingMgr_W.h>
#include <PlaySpace\PlaySpace_Mesh_W.h>
#include <System_Z.h>

// Prototypes
void MeshData_to_SurfaceInfo(const Dll_Interface::MeshData& meshData, Playspace_SR_SurfaceInfo& surfaceInfo);

// Functions
EXTERN_C __declspec(dllexport) int SpatialUnderstanding_Init()
{
	SetAndRestoreFloatControlDownward floatControlDownward;
	UnderstandingMgr_W::GetUnderstandingMgr().Init();
	return 1;
}

EXTERN_C __declspec(dllexport) void SpatialUnderstanding_Term()
{
	SetAndRestoreFloatControlDownward floatControlDownward;
	UnderstandingMgr_W::ReleaseUnderstandingMgr();
}

EXTERN_C __declspec(dllexport) void SetModeFrame_Inside()
{
	SetAndRestoreFloatControlDownward floatControlDownward;
	UnderstandingMgr_W::GetUnderstandingMgr().SetFrameMode(UnderstandingMgr_W::FrameMode::ManageInside, NULL);
}

EXTERN_C __declspec(dllexport) void SetMeshList_StaticScan(
	_In_ INT32 meshCount,
	_In_count_(meshCount) Dll_Interface::MeshData* meshes)
{
	SetAndRestoreFloatControlDownward floatControlDownward;
	UnderstandingMgr_W::GetUnderstandingMgr().SetSRMeshAnReset(meshes, meshCount);
}

EXTERN_C __declspec(dllexport) void GeneratePlayspace_InitScan(
	float camPos_X, float camPos_Y, float camPos_Z,
	float camFwd_X, float camFwd_Y, float camFwd_Z,
	float camUp_X,  float camUp_Y, float  camUp_Z,
	float searchDst, float optimalSize)
{
	SetAndRestoreFloatControlDownward floatControlDownward;
	Vec3f camPos = Vec3f(camPos_X, camPos_Y, camPos_Z);
	Vec3f camFwd = Vec3f(camFwd_X, camFwd_Y, camFwd_Z);

#if DebugData_SaveTrace
	DebugData_Save_DynamicScan_InitScan(camPos, camFwd, searchDst, optimalSize);
#endif

	UnderstandingMgr_W::GetUnderstandingMgr().InitScan(camPos, camFwd, searchDst, optimalSize);
}

EXTERN_C __declspec(dllexport) int GeneratePlayspace_UpdateScan_StaticScan(
	float camPos_X, float camPos_Y, float camPos_Z,
	float camFwd_X, float camFwd_Y, float camFwd_Z,
	float camUp_X, float camUp_Y, float  camUp_Z,
	float deltaTime)
{
	SetAndRestoreFloatControlDownward floatControlDownward;

	// Camera data
	Vec3f camPos = Vec3f(camPos_X, camPos_Y, camPos_Z);
	Vec3f camFwd = Vec3f(camFwd_X, camFwd_Y, camFwd_Z);
	Vec3f camUp = Vec3f(camUp_X, camUp_Y, camUp_Z);

	// Update scan
	UnderstandingMgr_W &UnderstandingMgr = UnderstandingMgr_W::GetUnderstandingMgr();
	PlaySpaceInfos_W::PSI_State updateState = UnderstandingMgr.UpdateScanFromStaticDatas(camPos, camFwd, camUp,deltaTime, PlaySpaceInfos_W::PSI_UpdateMode::PSI_Mode_Paint, 0.0f);
	UnderstandingMgr.Update(deltaTime);

	return (updateState == PlaySpaceInfos_W::PSI_State_Scanning_Finish) ? 1 : 0;
}

EXTERN_C __declspec(dllexport) int GeneratePlayspace_UpdateScan(
	_In_ INT32 meshCount, _In_count_(meshCount) Dll_Interface::MeshData* meshes,
	float camPos_X, float camPos_Y, float camPos_Z,
	float camFwd_X, float camFwd_Y, float camFwd_Z,
	float camUp_X, float camUp_Y, float  camUp_Z,
	float deltaTime)
{
	SetAndRestoreFloatControlDownward floatControlDownward;

	// Camera data
	Vec3f camPos = Vec3f(camPos_X, camPos_Y, camPos_Z);
	Vec3f camFwd = Vec3f(camFwd_X, camFwd_Y, camFwd_Z);
	Vec3f camUp = Vec3f(camUp_X, camUp_Y, camUp_Z);

#if DebugData_SaveTrace
	DebugData_Save_DynamicScan_UpdateScan(meshCount, meshes, camPos, camFwd, camUp, deltaTime);
#endif

	// Static data
	static Playspace_SR_DeviceSR deviceData;
	static int surfaceInfoCount = 0;
	static Playspace_SR_SurfaceInfo* surfaceInfos = NULL;

	// Allocate surfaceInfos, if needed
	if (surfaceInfoCount <= meshCount)
	{
		if (surfaceInfos != NULL)
		{
			Delete_Z surfaceInfos;
			surfaceInfos = NULL;
		}
		surfaceInfoCount = meshCount;
		surfaceInfos = New_Z Playspace_SR_SurfaceInfo[meshCount];
	}

	// And copy them
	for (int i = 0; i < meshCount; ++i)
	{
		MeshData_to_SurfaceInfo(meshes[i], surfaceInfos[i]);
	}

	// Mesh data
	deviceData.NbSurfaces = meshCount;
	deviceData.TabSurfaces = surfaceInfos;
	deviceData.GlobalTransfo.SetIdentity();

	// Update scan
	UnderstandingMgr_W &UnderstandingMgr = UnderstandingMgr_W::GetUnderstandingMgr();
	PlaySpaceInfos_W::PSI_State updateState = UnderstandingMgr.UpdateScanFromDeviceSR(&deviceData, camPos, camFwd, camUp, deltaTime, PlaySpaceInfos_W::PSI_UpdateMode::PSI_Mode_Paint, 0.0f);
	UnderstandingMgr.Update(deltaTime);

	return (updateState == PlaySpaceInfos_W::PSI_State_Scanning_Finish) ? 1 : 0;
}

EXTERN_C __declspec(dllexport) void GeneratePlayspace_RequestFinish()
{
	SetAndRestoreFloatControlDownward floatControlDownward;
	UnderstandingMgr_W::GetUnderstandingMgr().RequestFinishScan(false);
}

// Extracting the mesh is a two step process, the first generates the mesh for extraction & saves it off.
//	The caller is able to see vertex counts, etc. so they can allocate the proper amount of memory
static Playspace_Mesh* extractedMesh = NULL;
EXTERN_C __declspec(dllexport) int GeneratePlayspace_ExtractMesh_Setup(
	_Out_ INT32* vertexCount,
	_Out_ INT32* indexCount)
{
	SetAndRestoreFloatControlDownward floatControlDownward;

	// Defaults
	*vertexCount = 0;
	*indexCount = 0;

	// Validate
	UnderstandingMgr_W &UnderstandingMgr = UnderstandingMgr_W::GetUnderstandingMgr();
	PlaySpaceInfos_W& playspaceInfos = UnderstandingMgr.GetPlayspaceInfos();
	if (playspaceInfos.m_pSurfaceInfosV3 == NULL)
	{
		return 0;
	}

	// Setup
	Playspace_Mesh* pMesh = &(playspaceInfos.m_pSurfaceInfosV3->m_Mesh);
	if (pMesh == NULL)
	{
		return 0;
	}

	// Allocate a mesh, if we don't have one already
	if (extractedMesh == NULL)
	{
		extractedMesh = New_Z Playspace_Mesh();
	}

	// Construct extractedMesh
	extractedMesh->CopyFrom(*pMesh, true, true);
	extractedMesh->InvalidatePointsLinks();
	extractedMesh->InvalidateFaceToolNormal();
	extractedMesh->InvalidatePointsToolNormal();
	extractedMesh->Triangularize(true);

	// Fill in the count data
	*vertexCount = extractedMesh->m_TabPoints.GetSize();
	*indexCount = extractedMesh->m_TabQuad.GetSize() * 3;

	return 1;
}

EXTERN_C __declspec(dllexport) int GeneratePlayspace_ExtractMesh_Extract(
	_In_ INT32 bufferVertexCount,
	_In_ DirectX::XMFLOAT3* verticesPos,
	_In_ DirectX::XMFLOAT3* verticesNormal,
	_In_ INT32 bufferIndexCount,
	_In_ INT32* indices)
{
	SetAndRestoreFloatControlDownward floatControlDownward;

	// Validate
	if ((extractedMesh == NULL) ||
		(extractedMesh->m_TabPoints.GetSize() > (int)bufferVertexCount) ||
		((extractedMesh->m_TabQuad.GetSize() * 3) > (int)bufferIndexCount))
	{
		return 0;
	}
		
	// Rotate back to the external world space (same orientation as the input data - not the aligned internal representation)
	UnderstandingMgr_W &UnderstandingMgr = UnderstandingMgr_W::GetUnderstandingMgr();
	Quat quatDLLToWorld = UnderstandingMgr.GetFrameTransfoForOutput();
	Mat4x4	matDLLToWorld;
	quatDLLToWorld.GetMatrix(matDLLToWorld);

	// Setup the vertices
	int vertexCount = extractedMesh->m_TabPoints.GetSize();
	for (int vertIdx = 0; vertIdx < vertexCount; vertIdx++)
	{
		Vec4f vertex = extractedMesh->m_TabPoints[vertIdx];

		vertex = VecFloat4x4Transform3(matDLLToWorld, vertex);

		verticesPos[vertIdx].x = vertex.x;
		verticesPos[vertIdx].y = vertex.y;
		verticesPos[vertIdx].z = vertex.z;
	}

	// Setup the triangles
	int outputMeshIdx = 0;
	int indexCount = extractedMesh->m_TabQuad.GetSize();
	for (int i = 0; i < indexCount; ++i)
	{
		indices[outputMeshIdx++] = extractedMesh->m_TabQuad[i].TabPoints[0];
		indices[outputMeshIdx++] = extractedMesh->m_TabQuad[i].TabPoints[1];
		indices[outputMeshIdx++] = extractedMesh->m_TabQuad[i].TabPoints[2];
	}

	return 1;
}

// Extracting the voxels is a two step process, the first generates the voxel fiels for extraction & saves it off.
//	The caller is able to see the voxel count so they can allocate the proper amount of memory
static U8 * extractedVoxelArray = NULL;
static int extractedVoxelArraySize = 0;
static int extractedVoxelsPerRow = 0;
static float extractedVoxelWidth = 0.0f;
static Vec3f extractCenter;
static Vec3f extractOrigin;
static Vec3f extractOrigin_Half;
static Quat extractOrientation;
EXTERN_C __declspec(dllexport) int GeneratePlayspace_ExtractVoxel_Setup(
	_Out_ INT32* voxelCount)
{
	SetAndRestoreFloatControlDownward floatControlDownward;

	// Defaults
	*voxelCount = 0;

	// Validate
	UnderstandingMgr_W &UnderstandingMgr = UnderstandingMgr_W::GetUnderstandingMgr();
	PlaySpaceInfos_W & playspaceInfos = UnderstandingMgr.GetPlayspaceInfos();
	if (playspaceInfos.m_pSurfaceSR == NULL)
	{
		return 0;
	}

	// Setup
	Playspace_SR_W * psrw = playspaceInfos.m_pSurfaceSR;
	if (psrw == NULL)
	{
		return 0;
	}
	psrw->ExternalLockBlind(true);	// LOCK!!!
	Playspace_SR_BlindMap & srBlindMap = psrw->m_BlindMap;
	if (&srBlindMap == NULL)
	{
		return 0;
	}

	extractedVoxelsPerRow = SR_BLINDRAY_MAP_SIZE;
	extractedVoxelWidth = SR_BLINDRAY_MAP_TO_WORLD;
	extractCenter = srBlindMap.m_fCenter;
	extractOrigin = srBlindMap.m_fOrigin;
	extractOrigin_Half = srBlindMap.m_fOrigin_Half;
	extractOrientation = UnderstandingMgr.GetFrameTransfoForOutput();

	PSR_BlindMap_VoxelHUDA & dynBlindMap = srBlindMap.m_BlindMap;
	if (&dynBlindMap == NULL)
	{
		return 0;
	}
	*voxelCount = dynBlindMap.GetSize();

	// If there were previous voxels in the array, clear the old stuff first
	if (extractedVoxelArray != NULL)
	{
		delete[] extractedVoxelArray;
		extractedVoxelArraySize = 0;
	}

	// construct extractedVoxelArray
	PSR_BlindMap_Voxel * psrBlindMap = dynBlindMap.GetArrayPtr();
	extractedVoxelArray = new U8[*voxelCount];
	extractedVoxelArraySize = *voxelCount;
	for (int i = 0; i < extractedVoxelArraySize; i++)
	{
		extractedVoxelArray[i] = psrBlindMap[i].Flags;
	}

	psrw->ExternalLockBlind(false);	// UNLOCK!!!

	return 1;
}

EXTERN_C __declspec(dllexport) int GeneratePlayspace_ExtractVoxel_Extract(
	_In_ INT32 bufferVoxelCount,
	_In_ U8* voxels)
{
	SetAndRestoreFloatControlDownward floatControlDownward;

	// Validate
	if ((extractedVoxelArray == NULL) ||
		(extractedVoxelArraySize > (int)bufferVoxelCount))
	{
		return 0;
	}

	for (int i = 0; i < extractedVoxelArraySize; i++)
	{
		voxels[i] = extractedVoxelArray[i];
	}

	return 1;
}

EXTERN_C __declspec(dllexport) void GeneratePlayspace_ExtractVoxel_Metadata(
	_Out_ int* voxelsPerRow,
	_Out_ float* voxelWidth,
	_Out_ Vec3f* center,
	_Out_ Vec3f* origin,
	_Out_ Vec3f* originHalf,
	_Out_ Vec4f* orientation )
{
	SetAndRestoreFloatControlDownward floatControlDownward;

	*voxelsPerRow = extractedVoxelsPerRow;
	*voxelWidth = extractedVoxelWidth;

	// Rotate back to the external world space (same orientation as the input data - not the aligned internal representation)
	Quat quatDLLToWorld = extractOrientation;
	Mat4x4	matDLLToWorld;
	quatDLLToWorld.GetMatrix(matDLLToWorld);

	VecFloat4 hold;
	hold.x = extractCenter.x;
	hold.y = extractCenter.y;
	hold.z = extractCenter.z;
	hold.w = 1.0f;
	*center = VecFloat4x4Transform3(matDLLToWorld, hold);

	hold.x = extractOrigin.x;
	hold.y = extractOrigin.y;
	hold.z = extractOrigin.z;
	hold.w = 1.0f;
	*origin = VecFloat4x4Transform3(matDLLToWorld, hold);

	hold.x = extractOrigin_Half.x;
	hold.y = extractOrigin_Half.y;
	hold.z = extractOrigin_Half.z;
	hold.w = 1.0f;
	*originHalf = VecFloat4x4Transform3(matDLLToWorld, hold);

	hold.x = extractOrientation.v.x;
	hold.y = extractOrientation.v.y;
	hold.z = extractOrientation.v.z;
	hold.w = extractOrientation.w;
	*orientation = hold;
}

/******************** QUERIES **************************/

EXTERN_C __declspec(dllexport) int QueryPlayspaceStats(
	_In_ Dll_Interface::PlayspaceStats* playspaceStats
)
{
	SetAndRestoreFloatControlDownward floatControlDownward;

	// Query the stats
	UnderstandingMgr_W &UnderstandingMgr = UnderstandingMgr_W::GetUnderstandingMgr();
	PlaySpaceInfos_W& playspaceInfos = UnderstandingMgr.GetPlayspaceInfos();
	PlaySpaceInfos_ScanningStats stats;
	playspaceInfos.GetUnifiedScanV3Stats(stats);

	// Convert
	playspaceStats->IsWorkingOnStats			= stats.IsWorkingOnStats ? 0 : 1;

	playspaceStats->HorizSurfaceArea			= stats.MeshStats.HorizSurface;
	playspaceStats->TotalSurfaceArea			= stats.MeshStats.TotalSurface;
	playspaceStats->UpSurfaceArea				= stats.MeshStats.UpSurface;
	playspaceStats->DownSurfaceArea				= stats.MeshStats.DownSurface;
	playspaceStats->WallSurfaceArea				= stats.MeshStats.WallSurface;
	playspaceStats->VirtualCeilingSurfaceArea	= stats.MeshStats.VirtualCeilingSurface;
	playspaceStats->VirtualWallSurfaceArea		= stats.MeshStats.VirtualWallSurface;

	playspaceStats->NumFloor					= stats.MeshStats.Areas[0].GetSize();
	playspaceStats->NumCeiling					= stats.MeshStats.Areas[1].GetSize();
	playspaceStats->NumWall_XNeg				= stats.MeshStats.Areas[2].GetSize();
	playspaceStats->NumWall_XPos				= stats.MeshStats.Areas[3].GetSize();
	playspaceStats->NumWall_ZNeg				= stats.MeshStats.Areas[4].GetSize();
	playspaceStats->NumWall_ZPos				= stats.MeshStats.Areas[5].GetSize();
	playspaceStats->NumPlatform					= stats.MeshStats.Areas[6].GetSize();

	playspaceStats->CellCount_IsPaintMode		= (stats.MeshStats.ColoredCells.GetSize() > 0) ? stats.MeshStats.ColoredCells[0] : -1;
	playspaceStats->CellCount_IsSeenQualtiy_None = (stats.MeshStats.ColoredCells.GetSize() > 1) ? stats.MeshStats.ColoredCells[1] : -1;
	playspaceStats->CellCount_IsSeenQualtiy_Seen = (stats.MeshStats.ColoredCells.GetSize() > 2) ? stats.MeshStats.ColoredCells[2] : -1;
	playspaceStats->CellCount_IsSeenQualtiy_Good = (stats.MeshStats.ColoredCells.GetSize() > 3) ? stats.MeshStats.ColoredCells[3] : -1;

	return 1;
}

EXTERN_C __declspec(dllexport) int QueryPlayspaceAlignment(
	_In_ Dll_Interface::PlayspaceAlignment* playspaceAlignment)
{
	SetAndRestoreFloatControlDownward floatControlDownward;

	// Alignment matrix
	UnderstandingMgr_W &UnderstandingMgr = UnderstandingMgr_W::GetUnderstandingMgr();
	Quat quatDLLToWorld = UnderstandingMgr.GetFrameTransfoForOutput();
	Mat4x4	matDLLToWorld;
	quatDLLToWorld.GetMatrix(matDLLToWorld);

	// Fill it in
	PlaySpaceInfos_W& playspaceInfos = UnderstandingMgr.GetPlayspaceInfos();
	Util_L::Transform_Vec3f_to_XMFLOAT3(playspaceInfos.m_vCenter, playspaceAlignment->Center, matDLLToWorld);
	Util_L::Convert_Vec3f_to_XMFLOAT3((playspaceInfos.m_vMaxAvailable - playspaceInfos.m_vMinAvailable) * 0.5f, playspaceAlignment->HalfDims);
	Util_L::Transform_Vec3f_to_XMFLOAT3(Vec3f(1.0f, 0.0f, 0.0f), playspaceAlignment->BasisX, matDLLToWorld);
	Util_L::Transform_Vec3f_to_XMFLOAT3(Vec3f(0.0f, 1.0f, 0.0f), playspaceAlignment->BasisY, matDLLToWorld);
	Util_L::Transform_Vec3f_to_XMFLOAT3(Vec3f(0.0f, 0.0f, 1.0f), playspaceAlignment->BasisZ, matDLLToWorld);
	playspaceAlignment->FloorYValue = playspaceInfos.m_YGround;
	playspaceAlignment->CeilingYValue = playspaceInfos.m_YCeiling;

	return 1;
}

EXTERN_C __declspec(dllexport) int PlayspaceRaycast(
	float rayPos_X, float rayPos_Y, float rayPos_Z, 
	float rayVec_X, float rayVec_Y, float rayVec_Z,
	Dll_Interface::RaycastResult* result)
{
	SetAndRestoreFloatControlDownward floatControlDownward;

	// Basic checks
	if (result == NULL)
	{
		return 0;
	}

	// Setup
	UnderstandingMgr_W &UnderstandingMgr = UnderstandingMgr_W::GetUnderstandingMgr();

	Vec3f pt1 = Vec3f(rayPos_X, rayPos_Y, rayPos_Z);
	Vec3f pt2 = pt1 + Vec3f(rayVec_X, rayVec_Y, rayVec_Z);
	PlaySpaceInfos_W& playspaceInfos = UnderstandingMgr.GetPlayspaceInfos();
	result->SurfaceType = Dll_Interface::RaycastResult::Invalid;

	// Convert to local aligned space
	Mat4x4 matToDLL = UnderstandingMgr.GetFrameTransfoForInputMat();
	pt1 = VecFloat4x4Transform3(matToDLL, Vec4f(pt1));
	pt2 = VecFloat4x4Transform3(matToDLL, Vec4f(pt2));

	// Do the query
	Vec3f intersectPoint;
	S32 zoneID;
	PlaySpace_Surfel* surfel;
	if (playspaceInfos.RayCastVoxel(pt1, pt2, intersectPoint, &zoneID, FALSE, &surfel))
	{
		// Fill in results (partial)
		Mat4x4 matFromDll = UnderstandingMgr.GetFrameTransfoForOutputMat();
		Vec3f intersectPtOut = VecFloat4x4Transform3(matFromDll, Vec4f(intersectPoint));
		result->IntersectPoint.x = intersectPtOut.x;
		result->IntersectPoint.y = intersectPtOut.y;
		result->IntersectPoint.z = intersectPtOut.z;
		Vec3f normalOut = surfel  ? VecFloat4x4Transform3(matFromDll, Vec4f(surfel->Normal)) : -Vec3f(rayVec_X, rayVec_Y, rayVec_Z).Normalize();
		result->IntersectNormal.x = normalOut.x;
		result->IntersectNormal.y = normalOut.y;
		result->IntersectNormal.z = normalOut.z;

		// Zone info
		if ((zoneID >= 0) && 
			(zoneID < playspaceInfos.m_TabZoneInfos.GetSize()))
		{
			PlaySpace_ZoneInfos& zoneInfo = playspaceInfos.m_TabZoneInfos[zoneID];
			if ((zoneInfo.TopologyId >= 0) &&
				(zoneInfo.TopologyId < playspaceInfos.m_TopologyAnalyzer.m_daSurfaces.GetSize()))
			{
				// Pull the data out of the indexed surface
				TopologyAnalyzer_W::Surface surface = playspaceInfos.m_TopologyAnalyzer.m_daSurfaces[zoneInfo.TopologyId];
				result->SurfaceType = surface.m_bIsCeiling ? Dll_Interface::RaycastResult::Ceiling :
					surface.m_bIsGround ? Dll_Interface::RaycastResult::Floor : Dll_Interface::RaycastResult::Other;
				result->SurfaceArea = surface.m_fArea;

				// Check for floor-like
				if (surfel && (result->SurfaceType == Dll_Interface::RaycastResult::Other) &&
					(surfel->Normal.y > 0.707f) && (surface.m_fHeightFromGround < 0.15f))
				{
					result->SurfaceType = Dll_Interface::RaycastResult::FloorLike;
				}

				// Check for wall-like
				if (surfel && (result->SurfaceType == Dll_Interface::RaycastResult::Other) &&
					(Abs(surfel->Normal.y) < 0.707f) && (surface.m_fArea > 0.015f))
				{
					result->SurfaceType = Dll_Interface::RaycastResult::WallLike;
				}

				// Check for platform
				if (surfel && (result->SurfaceType == Dll_Interface::RaycastResult::Other) &&
					(surfel->Normal.y > 0.707f) && (surface.m_fArea > 0.005f))
				{
					result->SurfaceType = Dll_Interface::RaycastResult::Platform;
				}
			}
			else
			{
				result->SurfaceType = Dll_Interface::RaycastResult::Other;
				result->SurfaceArea = 0.0f;
			}
				
			// Check the surfel
			if (surfel &&
				((result->SurfaceType == Dll_Interface::RaycastResult::Other) || 
				 (result->SurfaceType == Dll_Interface::RaycastResult::WallLike)))
			{
				// Check for external wall
				if ((Abs(surfel->Normal.y) < 0.707f) && (surfel->Flags & PlaySpace_Surfel::SURFFLAG_EXTERNAL))
				{
					result->SurfaceType = Dll_Interface::RaycastResult::WallExternal;
				}
				else if (Abs(surfel->Normal.y) < 0.707f)
				{
					// Wall like
					float dstFromWall = playspaceInfos.m_TopologyAnalyzer.GetDistWallNear(surfel->Point, 1.0f);
					if (dstFromWall < 0.015)
					{
						result->SurfaceType = Dll_Interface::RaycastResult::WallLike;
					}
				}
			}
		}

		return 1;
	}

	return 0;
}

/******************** ONE-TIME SCAN **************************/

EXTERN_C __declspec(dllexport) void GeneratePlayspace_OneTimeScan(
	float camPos_X, float camPos_Y, float camPos_Z,
	float camFwd_X, float camFwd_Y, float camFwd_Z,
	float camUp_X, float camUp_Y, float camUp_Z)
{
	SetAndRestoreFloatControlDownward floatControlDownward;

	Vec3f camPos = Vec3f(camPos_X, camPos_Y, camPos_Z);
	Vec3f camFwd = Vec3f(camFwd_X, camFwd_Y, camFwd_Z);

	// Do the one pass process
	UnderstandingMgr_W &UnderstandingMgr = UnderstandingMgr_W::GetUnderstandingMgr();
	UnderstandingMgr.OnePassScanOnCurrentSR(camPos, camFwd, 8.f);
}

/******************** UTILITIES **************************/

void MeshData_to_SurfaceInfo(const Dll_Interface::MeshData& meshData, Playspace_SR_SurfaceInfo& surfaceInfo)
{
	surfaceInfo.ID = meshData.meshID;
	surfaceInfo.FrameUpdate = meshData.lastUpdateID;

	surfaceInfo.NbVertex = meshData.vertCount;
	surfaceInfo.pVertexPos = (Vec3f*)meshData.verts;

	surfaceInfo.NbIndex = meshData.indexCount;
	surfaceInfo.pIndex = (S32*)meshData.indices;
	surfaceInfo.IndexIsClockWise = false;

	surfaceInfo.PosToWorldMtx = Util_L::Convert_XMAT4x4_to_Mat4x4(meshData.transform);
}
