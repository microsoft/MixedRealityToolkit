// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include <pch.h>
#include <Dll_Interface\Dll_Topology.h>
#include <UnderstandingMgr_W.h>
#include <System_Z.h>

int OutputLocations(
	int locationCount,
	Dll_Interface::TopologyResult* locationData,
	Vec3fDA& outPos,
	Vec3f normal,
	float width = 0.0f,
	float length = 0.0f)
{
	// If they didn't provide any data, just report the size
	if ((locationCount == 0) || (locationData == NULL))
	{
		locationCount = outPos.GetSize();
		return locationCount;
	}

	UnderstandingMgr_W &UnderstandingMgr = UnderstandingMgr_W::GetUnderstandingMgr();

	// Transform out of dll
	Quat quatDLLToWorld = UnderstandingMgr.GetFrameTransfoForOutput();
	Mat4x4	matDLLToWorld;
	quatDLLToWorld.GetMatrix(matDLLToWorld);

	// Transform the normal
	normal = VecFloat4x4Transform3(matDLLToWorld, Vec4f(normal));

	// Copy the data out (and transform it back)
	locationCount = min(outPos.GetSize(), locationCount);
	for (int i = 0; i < locationCount; ++i)
	{
		Util_L::Transform_Vec3f_to_XMFLOAT3(outPos[i], locationData[i].position, matDLLToWorld);
		Util_L::Convert_Vec3f_to_XMFLOAT3(normal, locationData[i].normal);
		locationData[i].width = width;
		locationData[i].length = length;
	}

	return locationCount;
}

int OutputLocations(
	int locationCount, 
	Dll_Interface::TopologyResult* locationData,
	Vec3fDA& outPos, 
	Vec3fDA& outNormal,
	float width = 0.0f,
	float length = 0.0f)
{
	// If they didn't provide any data, just report the size
	if ((locationCount == 0) || (locationData == NULL))
	{
		locationCount = outPos.GetSize();
		return locationCount;
	}

	UnderstandingMgr_W &UnderstandingMgr = UnderstandingMgr_W::GetUnderstandingMgr();

	// Transform out of dll
	Quat quatDLLToWorld = UnderstandingMgr.GetFrameTransfoForOutput();
	Mat4x4	matDLLToWorld;
	quatDLLToWorld.GetMatrix(matDLLToWorld);

	// Copy the data out (and transform it back)
	locationCount = min(outPos.GetSize(), locationCount);
	for (int i = 0; i < locationCount; ++i)
	{
		Util_L::Transform_Vec3f_to_XMFLOAT3(outPos[i], locationData[i].position, matDLLToWorld);
		Util_L::Transform_Vec3f_to_XMFLOAT3(outNormal[i], locationData[i].normal, matDLLToWorld);
		locationData[i].width = width;
		locationData[i].length = length;
	}

	return locationCount;
}

int OutputLocations(
	int locationCount,
	Dll_Interface::TopologyResult* locationData,
	Vec3fDA& outPos,
	Vec3fDA& outNormal,
	FloatDA& widths,
	float length = 0.0f)
{
	// If they didn't provide any data, just report the size
	if ((locationCount == 0) || (locationData == NULL))
	{
		locationCount = outPos.GetSize();
		return locationCount;
	}

	UnderstandingMgr_W &UnderstandingMgr = UnderstandingMgr_W::GetUnderstandingMgr();

	// Transform out of dll
	Quat quatDLLToWorld = UnderstandingMgr.GetFrameTransfoForOutput();
	Mat4x4	matDLLToWorld;
	quatDLLToWorld.GetMatrix(matDLLToWorld);

	// Copy the data out (and transform it back)
	locationCount = min(outPos.GetSize(), locationCount);
	for (int i = 0; i < locationCount; ++i)
	{
		Util_L::Transform_Vec3f_to_XMFLOAT3(outPos[i], locationData[i].position, matDLLToWorld);
		Util_L::Transform_Vec3f_to_XMFLOAT3(outNormal[i], locationData[i].normal, matDLLToWorld);
		locationData[i].width = widths[i];
		locationData[i].length = length;
	}

	return locationCount;
}

int OutputLocations(
	int locationCount,
	Dll_Interface::TopologyResult* locationData,
	Vec3fDA& outPos,
	Vec3fDA& outNormal,
	FloatDA& widths,
	FloatDA& lengths)
{
	// If they didn't provide any data, just report the size
	if ((locationCount == 0) || (locationData == NULL))
	{
		locationCount = outPos.GetSize();
		return locationCount;
	}

	UnderstandingMgr_W &UnderstandingMgr = UnderstandingMgr_W::GetUnderstandingMgr();

	// Transform out of dll
	Quat quatDLLToWorld = UnderstandingMgr.GetFrameTransfoForOutput();
	Mat4x4	matDLLToWorld;
	quatDLLToWorld.GetMatrix(matDLLToWorld);

	// Copy the data out (and transform it back)
	locationCount = min(outPos.GetSize(), locationCount);
	for (int i = 0; i < locationCount; ++i)
	{
		Util_L::Transform_Vec3f_to_XMFLOAT3(outPos[i], locationData[i].position, matDLLToWorld);
		Util_L::Transform_Vec3f_to_XMFLOAT3(outNormal[i], locationData[i].normal, matDLLToWorld);
		locationData[i].width = widths[i];
		locationData[i].length = lengths[i];
	}

	return locationCount;
}

EXTERN_C __declspec(dllexport) int QueryTopology_FindPositionsOnWalls(
	_In_ float minHeightOfWallSpace,
	_In_ float minWidthOfWallSpace,
	_In_ float minHeightAboveFloor,
	_In_ float minFacingClearance,
	_In_ int locationCount,
	_Inout_ Dll_Interface::TopologyResult* locationData)
{
	SetAndRestoreFloatControlDownward floatControlDownward;

	Vec3fDA outPos, outNormal;
	UnderstandingMgr_W::GetUnderstandingMgr().GetPlayspaceInfos().m_TopologyAnalyzer.GetAllPosOnWall(minHeightAboveFloor, minHeightOfWallSpace, minWidthOfWallSpace, minFacingClearance, outPos, outNormal, false);

	return OutputLocations(locationCount, locationData, outPos, outNormal, minWidthOfWallSpace);
}

EXTERN_C __declspec(dllexport) int QueryTopology_FindLargePositionsOnWalls(
	_In_ float minHeightOfWallSpace,
	_In_ float minWidthOfWallSpace,
	_In_ float minHeightAboveFloor,
	_In_ float minFacingClearance,
	_In_ int locationCount,
	_Inout_ Dll_Interface::TopologyResult* locationData)
{
	SetAndRestoreFloatControlDownward floatControlDownward;

	Vec3fDA outPos, outNormal;
	FloatDA outWidth;
	UnderstandingMgr_W::GetUnderstandingMgr().GetPlayspaceInfos().m_TopologyAnalyzer.GetAllLargePosOnWall(minHeightAboveFloor, minHeightOfWallSpace, minWidthOfWallSpace, minFacingClearance, outPos, outNormal, outWidth, false);

	return OutputLocations(locationCount, locationData, outPos, outNormal, outWidth);
}

EXTERN_C __declspec(dllexport) int QueryTopology_FindLargestWall(
	_Inout_ Dll_Interface::TopologyResult* wallOut)
{
	SetAndRestoreFloatControlDownward floatControlDownward;

	UnderstandingMgr_W &UnderstandingMgr = UnderstandingMgr_W::GetUnderstandingMgr();

	const TopologyAnalyzer_W::Wall* wall = UnderstandingMgr.GetPlayspaceInfos().m_TopologyAnalyzer.GetLargestWall(false, false);
	if (wall == NULL)
	{
		return 0;
	}

	// Transform out of dll
	Quat quatDLLToWorld = UnderstandingMgr.GetFrameTransfoForOutput();
	Mat4x4	matDLLToWorld;
	quatDLLToWorld.GetMatrix(matDLLToWorld);

	// Output
	Util_L::Transform_Vec3f_to_XMFLOAT3(wall->m_vCentroid, wallOut->position, matDLLToWorld);
	Util_L::Transform_Vec3f_to_XMFLOAT3(wall->m_vNormal, wallOut->normal, matDLLToWorld);
	wallOut->width = wall->m_fWidth;
	wallOut->length = wall->m_fHeight;

	return 1;
}

EXTERN_C __declspec(dllexport) int QueryTopology_FindPositionsOnFloor(
	_In_ float minLengthOfFloorSpace,
	_In_ float minWidthOfFloorSpace,
	_In_ int locationCount,
	_Inout_ Dll_Interface::TopologyResult* locationData)
{
	SetAndRestoreFloatControlDownward floatControlDownward;

	Vec3fDA outPos, outNormal;
	UnderstandingMgr_W::GetUnderstandingMgr().GetPlayspaceInfos().m_TopologyAnalyzer.GetAllRectanglePosOnFloor(minWidthOfFloorSpace, minLengthOfFloorSpace, outPos, outNormal);

	return OutputLocations(locationCount, locationData, outPos, outNormal, 0.0f);
}

EXTERN_C __declspec(dllexport) int QueryTopology_FindLargestPositionsOnFloor(
	_In_ int locationCount,
	_Inout_ Dll_Interface::TopologyResult* locationData)
{
	SetAndRestoreFloatControlDownward floatControlDownward;

	Vec3fDA outPos;
	UnderstandingMgr_W::GetUnderstandingMgr().GetPlayspaceInfos().m_TopologyAnalyzer.GetLargestPosOnFloor(outPos);

	return OutputLocations(locationCount, locationData, outPos, Vec3f(0.0f, 1.0f, 0.0f), 0.0f);
}

EXTERN_C __declspec(dllexport) int QueryTopology_FindPositionsSittable(
	_In_ float minHeight,
	_In_ float maxHeight,
	_In_ float minFacingClearance,
	_In_ int locationCount,
	_Inout_ Dll_Interface::TopologyResult* locationData)
{
	SetAndRestoreFloatControlDownward floatControlDownward;

	Vec3fDA outPos, outNormal;
	UnderstandingMgr_W::GetUnderstandingMgr().GetPlayspaceInfos().m_TopologyAnalyzer.GetAllPosSittable(minHeight, maxHeight, minFacingClearance, outPos, outNormal);

	return OutputLocations(locationCount, locationData, outPos, outNormal, 0.0f);
}

EXTERN_C __declspec(dllexport) int QueryTopology_FindLargePositionsSittable(
  _In_ float minHeight,
  _In_ float maxHeight,
  _In_ float minFacingClearance,
  _In_ float minWidth,
  _In_ int locationCount,
  _Inout_ Dll_Interface::TopologyResult* locationData)
{
  Vec3fDA outPos, outNormal;
  UnderstandingMgr_W::GetUnderstandingMgr().GetPlayspaceInfos().m_TopologyAnalyzer.GetAllLargePosSittable(minHeight, maxHeight, minFacingClearance, minWidth, outPos, outNormal);

  return OutputLocations(locationCount, locationData, outPos, outNormal, 0.0f);
}

