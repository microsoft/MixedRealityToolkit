// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#ifndef _DLL_TOPOLOGY
#define _DLL_TOPOLOGY

#include <Dll_Interface.h>

namespace Dll_Interface
{
	// DLL Interface
	EXTERN_C __declspec(dllexport) bool DebugData_LoadAndSetMesh(const char* filePath);
	EXTERN_C __declspec(dllexport) int DebugData_StaticMesh_LoadAndSet(const char* filePath, bool reCenterMesh = false);

	// Prototypes
	void DebugData_StaticMesh_Save(_In_ INT32 meshCount, _In_count_(meshCount) Dll_Interface::MeshData* meshes);
	void DebugData_Save_DynamicScan_InitScan(Vec3f camPos, Vec3f camFwd, float searchDst, float optimalSize);
	void DebugData_Save_DynamicScan_UpdateScan(_In_ INT32 meshCount, _In_count_(meshCount) Dll_Interface::MeshData* meshes, Vec3f camPos, Vec3f camFwd, Vec3f camUp, float deltaTime);
}

#endif
