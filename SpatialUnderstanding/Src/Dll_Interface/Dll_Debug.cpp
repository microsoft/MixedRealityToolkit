// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include <pch.h>
#include <Dll_Interface\Dll_Debug.h>
#include <UnderstandingMgr_W.h>
#include <System_Z.h>

/******************** DEBUG DATA **************************/
#define DebugData_SaveTrace		false
#define DebugData_FilePath		"C:\\Data\\Users\\<username>\\AppData\\Local\\Packages\\HoloToolkit-Unity_pzq3xp76mxafg\\TempState\\dynMeshTest.out"
#define DebugData_Version		2
/******************** DEBUG DATA **************************/

void DebugData_SaveMeshList(
	FILE* f,
	int meshCount,
	Dll_Interface::MeshData* meshList)
{
	fwrite(&meshCount, 1, sizeof(int), f);									// meshCount
	for (int i = 0; i < meshCount; ++i)
	{
		Dll_Interface::MeshData& mesh = meshList[i];
		fwrite(&mesh.meshID, 1, sizeof(INT32), f);							// meshID
		fwrite(&mesh.lastUpdateID, 1, sizeof(INT32), f);					// lastUpdateID
		fwrite(&mesh.transform, 1, sizeof(DirectX::XMFLOAT4X4), f);			// transform
		fwrite(&mesh.vertCount, 1, sizeof(INT32), f);						// vertCount
		fwrite(&mesh.indexCount, 1, sizeof(INT32), f);						// indexCount
		fwrite(mesh.verts, mesh.vertCount, sizeof(DirectX::XMFLOAT3), f);	// verts
		fwrite(mesh.normals, mesh.vertCount, sizeof(DirectX::XMFLOAT3), f);	// normals
		fwrite(mesh.indices, mesh.indexCount, sizeof(INT32), f);			// indices
	}
}
bool DebugData_LoadMeshList(
	FILE* f,
	int& meshCount,
	Dll_Interface::MeshData*& meshList)
{
	fread(&meshCount, sizeof(int), 1, f);									// meshCount
	if (meshCount <= 0)
	{
		return false;
	}
	meshList = new Dll_Interface::MeshData[meshCount];
	for (int i = 0; i < meshCount; ++i)
	{
		Dll_Interface::MeshData& mesh = meshList[i];
		fread(&mesh.meshID, 1, sizeof(INT32), f);							// meshID
		fread(&mesh.lastUpdateID, 1, sizeof(INT32), f);						// lastUpdateID
		fread(&mesh.transform, 1, sizeof(DirectX::XMFLOAT4X4), f);			// transform
		fread(&mesh.vertCount, 1, sizeof(INT32), f);						// vertCount
		fread(&mesh.indexCount, 1, sizeof(INT32), f);						// indexCount
		mesh.verts = new DirectX::XMFLOAT3[mesh.vertCount];
		fread(mesh.verts, mesh.vertCount, sizeof(DirectX::XMFLOAT3), f);	// verts
		mesh.normals = new DirectX::XMFLOAT3[mesh.vertCount];
		fread(mesh.normals, mesh.vertCount, sizeof(DirectX::XMFLOAT3), f);	// normals
		mesh.indices = new INT32[mesh.indexCount];
		fread(mesh.indices, mesh.indexCount, sizeof(INT32), f);				// indices
	}
	return true;
}
void DebugData_StaticMesh_Save(
	_In_ INT32 meshCount,
	_In_count_(meshCount) Dll_Interface::MeshData* meshes)
{
	// File
	FILE* f;
	fopen_s(&f, DebugData_FilePath, "wb+");

	// Version
	int version = DebugData_Version; fwrite(&version, 1, sizeof(int), f);	// Version

																			// Meshs
	DebugData_SaveMeshList(f, meshCount, meshes);

	// Done
	fclose(f);
}
EXTERN_C __declspec(dllexport) int DebugData_StaticMesh_LoadAndSet(const char* filePath, bool reCenterMesh)
{
	SetAndRestoreFloatControlDownward floatControlDownward;
	UnderstandingMgr_W &UnderstandingMgr = UnderstandingMgr_W::GetUnderstandingMgr();

	char lastChar = filePath[strlen(filePath) - 1];
	if (lastChar == 'W')
	{
		return UnderstandingMgr.GetPlayspaceSR().LoadSRMeshRaw(filePath);
	}

	// Load data
	FILE* f;
	fopen_s(&f, (filePath == NULL) ? DebugData_FilePath : filePath, "rb");
	if (f == NULL)
	{
		return false;
	}
	int version = 0; fread(&version, sizeof(int), 1, f);					// Version
	if (version != DebugData_Version)
	{
		return false;
	}
	int meshCount = 0;
	Dll_Interface::MeshData* meshes = NULL;
	if (DebugData_LoadMeshList(f, meshCount, meshes))
	{
		return false;
	}
	fclose(f);

	// Possibly reCenter the mesh, if requested (this will look at all the transform & make the average the origin - it does not do a per vertex centering)
	if (reCenterMesh)
	{
		int translationSumCount = 0;
		Vec3f translationSum = Vec3f(0, 0, 0);
		for (int i = 0; i < meshCount; ++i)
		{
			translationSum += Vec3f(meshes[i].transform._41, meshes[i].transform._42, meshes[i].transform._43);
			++translationSumCount;
		}
		translationSum *= (1.0f / (float)max(translationSumCount, 1));
		for (int i = 0; i < meshCount; ++i)
		{
			Vec3f translation = Vec3f(meshes[i].transform._41, meshes[i].transform._42, meshes[i].transform._43) - translationSum;
			meshes[i].transform._41 = translation.x;
			meshes[i].transform._42 = translation.y;
			meshes[i].transform._43 = translation.z;
		}
	}

	// Set
	UnderstandingMgr.SetSRMeshAnReset(meshes, meshCount);

	return true;
}
void DebugData_Save_DynamicScan_InitScan(
	Vec3f camPos,
	Vec3f camFwd,
	float searchDst,
	float optimalSize)
{
	// File
	FILE* f;
	fopen_s(&f, DebugData_FilePath, "wb+");

	// Version
	int version = DebugData_Version; fwrite(&version, 1, sizeof(int), f);	// Version

																			// Data
	fwrite(&camPos, 1, sizeof(Vec3f), f);
	fwrite(&camFwd, 1, sizeof(Vec3f), f);
	fwrite(&searchDst, 1, sizeof(float), f);
	fwrite(&optimalSize, 1, sizeof(float), f);

	// Done
	fclose(f);
}
int DebugData_LoadAndSet_DynamicScan_InitScan(FILE* f)
{
	SetAndRestoreFloatControlDownward floatControlDownward;

	// Version
	int version = 0; fread(&version, sizeof(int), 1, f);
	if (version != DebugData_Version)
	{
		return 0;
	}

	// Data
	Vec3f camPos, camFwd;
	float searchDst, optimalSize;
	fread(&camPos, 1, sizeof(Vec3f), f);
	fread(&camFwd, 1, sizeof(Vec3f), f);
	fread(&searchDst, 1, sizeof(float), f);
	fread(&optimalSize, 1, sizeof(float), f);

	// Call it
	Dll_Interface::GeneratePlayspace_InitScan(
		camPos.x, camPos.y, camPos.z,
		camFwd.x, camFwd.y, camFwd.z,
		0.0f, 1.0f, 0.0f,
		searchDst, optimalSize);

	return 1;
}
void DebugData_Save_DynamicScan_UpdateScan(
	_In_ INT32 meshCount,
	_In_count_(meshCount) Dll_Interface::MeshData* meshes,
	Vec3f camPos,
	Vec3f camFwd,
	Vec3f camUp,
	float deltaTime)
{
	// File
	FILE* f;
	fopen_s(&f, DebugData_FilePath, "ab");

	// Data
	fwrite(&camPos, 1, sizeof(Vec3f), f);
	fwrite(&camFwd, 1, sizeof(Vec3f), f);
	fwrite(&camUp, 1, sizeof(Vec3f), f);
	fwrite(&deltaTime, 1, sizeof(float), f);
	DebugData_SaveMeshList(f, meshCount, meshes);

	// Done
	fclose(f);
}
int DebugData_LoadAndSet_DynamicScan_UpdateScan(FILE* f)
{
	SetAndRestoreFloatControlDownward floatControlDownward;

	// Data
	Vec3f camPos, camFwd, camUp;
	float deltaTime;
	if (fread(&camPos, 1, sizeof(Vec3f), f) == 0)
	{
		return 0;
	}
	fread(&camFwd, 1, sizeof(Vec3f), f);
	fread(&camUp, 1, sizeof(Vec3f), f);
	fread(&deltaTime, 1, sizeof(float), f);
	int meshCount = 0;
	Dll_Interface::MeshData* meshes = NULL;
	DebugData_LoadMeshList(f, meshCount, meshes);

	// Call it
	GeneratePlayspace_UpdateScan(
		meshCount, meshes,
		camPos.x, camPos.y, camPos.z,
		camFwd.x, camFwd.y, camFwd.z,
		camUp.x, camUp.y, camUp.z,
		deltaTime);

	return 1;
}
EXTERN_C __declspec(dllexport) int DebugData_LoadAndSet_DynamicScan(const char* filePath)
{
	FILE* f = NULL;
	fopen_s(&f, filePath, "rb");
	if (f == NULL)
	{
		return 0;
	}

	// Init
	if (!DebugData_LoadAndSet_DynamicScan_InitScan(f))
	{
		return 0;
	}

	// Dynamic updates
	while (DebugData_LoadAndSet_DynamicScan_UpdateScan(f))
	{
		Sleep((DWORD)(1000.0f / 60.0f));
	}

	return 1;
}

EXTERN_C __declspec(dllexport) void DebugData_GeneratePlayspace_OneTimeScan()
{
	SetAndRestoreFloatControlDownward floatControlDownward;

	DirectX::XMFLOAT3 cameraPosition = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT3 cameraForward = DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);
	DirectX::XMFLOAT3 cameraOrientUp = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);

	Dll_Interface::GeneratePlayspace_OneTimeScan(cameraPosition.x, cameraPosition.y, cameraPosition.z,
		cameraForward.x, cameraForward.y, cameraForward.z,
		cameraOrientUp.x, cameraOrientUp.y, cameraOrientUp.z);
}
