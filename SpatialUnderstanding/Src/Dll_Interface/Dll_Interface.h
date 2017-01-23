// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#ifndef _DLL_INTERFACE
#define _DLL_INTERFACE

namespace Dll_Interface
{
#pragma pack(push, 1)
	struct MeshData
	{
		int meshID;				// Unique ID for this mesh
		int lastUpdateID;		// Inc'ed whenever the mesh is updated
		DirectX::XMFLOAT4X4 transform;
		INT32 vertCount;
		INT32 indexCount;
		DirectX::XMFLOAT3* verts;
		DirectX::XMFLOAT3* normals;
		INT32* indices;
	};
	struct RaycastResult
	{
		enum SurfaceTypes
		{
			Invalid,			// If no intersection
			Other,
			Floor,
			FloorLike,			// Not part of the floor topology, but close to the floor and looks like the floor
			Platform,			// Horizontal platform between the ground and the ceiling
			Ceiling,
			WallExternal,
			WallLike,			// Not part of the external wall surface, but vertical surface that looks like a wall structure
		};
		SurfaceTypes SurfaceType;
		float SurfaceArea;		// Zero if unknown (not part of the topology analysis)
		DirectX::XMFLOAT3 IntersectPoint;
		DirectX::XMFLOAT3 IntersectNormal;
	};
	struct PlayspaceStats
	{
		int		IsWorkingOnStats;				// 0 if still working on creating the stats

		float	HorizSurfaceArea;				// In m2 : All horizontal faces UP between Ground – 0.15 and Ground + 1.f (include Ground and convenient horiz surface)
		float	TotalSurfaceArea;				// In m2 : All !
		float	UpSurfaceArea;					// In m2 : All horizontal faces UP (no constraint => including ground)
		float	DownSurfaceArea;				// In m2 : All horizontal faces DOWN (no constraint => including ceiling)
		float	WallSurfaceArea;				// In m2 : All Vertical faces (not only walls)
		float	VirtualCeilingSurfaceArea;		// In m2 : estimation of surface of virtual Ceiling.
		float	VirtualWallSurfaceArea;			// In m2 : estimation of surface of virtual Walls.

		int		NumFloor;						// List of Area of each Floor surface (contains count)
		int		NumCeiling;						// List of Area of each Ceiling surface (contains count)
		int		NumWall_XNeg;					// List of Area of each Wall XNeg surface (contains count)
		int		NumWall_XPos;					// List of Area of each Wall XPos surface (contains count)
		int		NumWall_ZNeg;					// List of Area of each Wall ZNeg surface (contains count)
		int		NumWall_ZPos;					// List of Area of each Wall ZPos surface (contains count)
		int		NumPlatform;					// List of Area of each Horizontal not Floor surface (contains count)

		int		CellCount_IsPaintMode;			// Number paint cells (could deduce surface of painted area) => 8cm x 8cm cell
		int		CellCount_IsSeenQualtiy_None;	// Number of not seen cells => 8cm x 8cm cell
		int		CellCount_IsSeenQualtiy_Seen;	// Number of seen cells => 8cm x 8cm cell
		int		CellCount_IsSeenQualtiy_Good;	// Number of seen cells good quality => 8cm x 8cm cell
	};
	struct PlayspaceAlignment
	{
		DirectX::XMFLOAT3 Center;
		DirectX::XMFLOAT3 HalfDims;
		DirectX::XMFLOAT3 BasisX;
		DirectX::XMFLOAT3 BasisY;
		DirectX::XMFLOAT3 BasisZ;
		float FloorYValue;
		float CeilingYValue;
	};
#pragma pack(pop)

	// Init/term
	EXTERN_C __declspec(dllexport) int SpatialUnderstanding_Init();
	EXTERN_C __declspec(dllexport) void SpatialUnderstanding_Term();

	// Scan flow control
	EXTERN_C __declspec(dllexport) void GeneratePlayspace_InitScan(
		float camPos_X, float camPos_Y, float camPos_Z,
		float camFwd_X, float camFwd_Y, float camFwd_Z,
		float camUp_X, float camUp_Y, float  camUp_Z,
		float searchDst, float optimalSize);
	EXTERN_C __declspec(dllexport) int GeneratePlayspace_UpdateScan(
		_In_ INT32 meshCount, _In_count_(meshCount) Dll_Interface::MeshData* meshes,
		float camPos_X, float camPos_Y, float camPos_Z,
		float camFwd_X, float camFwd_Y, float camFwd_Z,
		float camUp_X, float camUp_Y, float  camUp_Z,
		float deltaTime);
	EXTERN_C __declspec(dllexport) void GeneratePlayspace_RequestFinish();

	// Mesh extraction
	EXTERN_C __declspec(dllexport) int GeneratePlayspace_ExtractMesh_Setup(
		_Out_ INT32* vertexCount,
		_Out_ INT32* indexCount);
	EXTERN_C __declspec(dllexport) int GeneratePlayspace_ExtractMesh_Extract(
		_In_ INT32 bufferVertexCount,
		_In_ DirectX::XMFLOAT3* verticesPos,
		_In_ DirectX::XMFLOAT3* verticesNormal,
		_In_ INT32 bufferIndexCount,
		_In_ INT32* indices);

	// Voxel extraction
	EXTERN_C __declspec(dllexport) int GeneratePlayspace_ExtractVoxel_Setup(
		_Out_ INT32* voxelCount);
	EXTERN_C __declspec(dllexport) int GeneratePlayspace_ExtractVoxel_Extract(
		_In_ INT32 bufferVoxelCount,
		_In_ U8* voxels);
	EXTERN_C __declspec(dllexport) void GeneratePlayspace_ExtractVoxel_Metadata(
		_Out_ int* voxelsPerRow,
		_Out_ float* voxelWidth,
		_Out_ Vec3f* center,
		_Out_ Vec3f* origin,
		_Out_ Vec3f* originHalf,
		_Out_ Vec4f* orientation );

	// Query functions
	EXTERN_C __declspec(dllexport) int QueryPlayspaceStats(
		_In_ Dll_Interface::PlayspaceStats* playspaceStats);
	EXTERN_C __declspec(dllexport) int QueryPlayspaceAlignment(
		_In_ Dll_Interface::PlayspaceAlignment* playspaceAlignment);
	EXTERN_C __declspec(dllexport) int PlayspaceRaycast(
		float rayPos_X, float rayPos_Y, float rayPos_Z,
		float rayVec_X, float rayVec_Y, float rayVec_Z,
		Dll_Interface::RaycastResult* result);

	// Static scan
	EXTERN_C __declspec(dllexport) void SetMeshList_StaticScan(
		_In_ INT32 meshCount,
		_In_count_(meshCount) Dll_Interface::MeshData* meshes);
	EXTERN_C __declspec(dllexport) void GeneratePlayspace_OneTimeScan(
		float camPos_X, float camPos_Y, float camPos_Z,
		float camFwd_X, float camFwd_Y, float camFwd_Z,
		float camUp_X, float camUp_Y, float camUp_Z);
	EXTERN_C __declspec(dllexport) int GeneratePlayspace_UpdateScan_StaticScan(
		float camPos_X, float camPos_Y, float camPos_Z,
		float camFwd_X, float camFwd_Y, float camFwd_Z,
		float camUp_X, float camUp_Y, float  camUp_Z,
		float deltaTime);
}

#endif
