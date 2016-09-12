// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#ifndef _DLL_TOPOLOGY
#define _DLL_TOPOLOGY

namespace Dll_Interface
{
#pragma pack(push, 1)
	struct TopologyResult
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 normal;
		float width;
		float length;
	};
#pragma pack(pop)

	// Topology queries - Wall
	EXTERN_C __declspec(dllexport) int QueryTopology_FindPositionsOnWalls(
		_In_ float minHeightOfWallSpace,
		_In_ float minWidthOfWallSpace,
		_In_ float minHeightAboveFloor,
		_In_ float minFacingClearance,
		_In_ int locationCount,
		_Inout_ TopologyResult* locationData);
	EXTERN_C __declspec(dllexport) int QueryTopology_FindLargePositionsOnWalls(
		_In_ float minHeightOfWallSpace,
		_In_ float minWidthOfWallSpace,
		_In_ float minHeightAboveFloor,
		_In_ float minFacingClearance,
		_In_ int locationCount,
		_Inout_ TopologyResult* locationData);
	EXTERN_C __declspec(dllexport) int QueryTopology_FindLargestWall(
		_Inout_ TopologyResult* wall);

	// Topology queries - Floor
	EXTERN_C __declspec(dllexport) int QueryTopology_FindPositionsOnFloor(
		_In_ float minArea,
		_In_ int locationCount,
		_Inout_ TopologyResult* locationData);
	EXTERN_C __declspec(dllexport) int QueryTopology_FindLargestPositionsOnFloor(
		_In_ int locationCount,
		_Inout_ TopologyResult* locationData);

	// Sittable
	EXTERN_C __declspec(dllexport) int QueryTopology_FindPositionsSittable(
		_In_ float minHeight,
		_In_ float maxHeight,
		_In_ float minFacingClearance,
		_In_ int locationCount,
		_Inout_ TopologyResult* locationData);
}

#endif
