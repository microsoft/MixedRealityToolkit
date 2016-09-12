// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include <DirectXMath.h>

using namespace TestApp;

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;

// The Blank Page item template is documented at http://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

MainPage::MainPage()
{
	InitializeComponent();
}

EXTERN_C __declspec(dllexport) int SpatialUnderstanding_Init();
EXTERN_C __declspec(dllexport) void SpatialUnderstanding_Term();

EXTERN_C __declspec(dllexport) void SetModeFrame_Inside();
EXTERN_C __declspec(dllexport) bool DebugData_StaticMesh_LoadAndSet(const char* filePath, bool reCenterMesh);
EXTERN_C __declspec(dllexport) int DebugData_LoadAndSet_DynamicScan_InitScan(FILE* f);
EXTERN_C __declspec(dllexport) int DebugData_LoadAndSet_DynamicScan_UpdateScan(FILE* f);
EXTERN_C __declspec(dllexport) void DebugData_GeneratePlayspace_OneTimeScan();

EXTERN_C __declspec(dllexport) void GeneratePlayspace_InitScan(
	float camPos_X, float camPos_Y, float camPos_Z,
	float camFwd_X, float camFwd_Y, float camFwd_Z,
	float camUp_X,  float camUp_Y,  float camUp_Z,
	float searchDst, float optimalSize);
EXTERN_C __declspec(dllexport) bool GeneratePlayspace_UpdateScan_StaticScan(
	float camPos_X, float camPos_Y, float camPos_Z,
	float camFwd_X, float camFwd_Y, float camFwd_Z,
	float camUp_X, float camUp_Y, float camUp_Z,
	float deltaTime);
EXTERN_C __declspec(dllexport) void GeneratePlayspace_RequestFinish();

EXTERN_C __declspec(dllexport) bool GeneratePlayspace_ExtractMesh_Setup(
	_Out_ INT32* vertexCount,
	_Out_ INT32* indexCount);
EXTERN_C __declspec(dllexport) bool GeneratePlayspace_ExtractMesh_Extract(
	_In_ INT32 bufferVertexCount,
	_In_ DirectX::XMFLOAT3* verticesPos,
	_In_ DirectX::XMFLOAT3* verticesNormal,
	_In_ INT32 bufferIndexCount,
	_In_ INT32* indices);


void TestApp::MainPage::Button_Click(Platform::Object^ sender, Windows::UI::Xaml::RoutedEventArgs^ e)
{
	// Init
	SpatialUnderstanding_Init();

	// One-time scan
	//RunTest_OneTimeScan();

	// Real-time scan (static input data)
	//RunTest_RealTimeScan_StaticInputData();

	// Real-time scan (dynamic input data)
	RunTest_RealTimeScan_DynamicInputData();

	// Init
	SpatialUnderstanding_Term();
}

// DEFINES
//#define INPUT_MESH "\\test.out"
//#define INPUT_MESH "\\TEST2.RAW"
#define INPUT_MESH "\\largeSR.out"
#define DYNMESHTEST "\\dynMeshTest_0.out"

// Note: To run these tests, copy the test data file listed above to your app's localstate folder
// example: "C:\Users\<username>\AppData\Local\Packages\TestApp_c74ckhh66gw46\LocalState"

const char* TestApp::MainPage::CalcInputMeshFilename()
{
	// Data file path
	Windows::Storage::StorageFolder^ localFolder = Windows::Storage::ApplicationData::Current->LocalFolder;
	Platform::String^ dir = localFolder->Path;
	Platform::String^ fullPath = dir + INPUT_MESH;

	// some ugly code to get the LocalState folder into a simple char array:
	const wchar_t* str = fullPath->Data();
	size_t num;
	wcstombs_s(&num, fileBuffer, 1000, str, fullPath->Length() + 1);

	return fileBuffer;
}

const char* TestApp::MainPage::CalcInputDynMeshTestFilename()
{
	// Data file path
	Windows::Storage::StorageFolder^ localFolder = Windows::Storage::ApplicationData::Current->LocalFolder;
	Platform::String^ dir = localFolder->Path;
	Platform::String^ fullPath = dir + DYNMESHTEST;

	// some ugly code to get the LocalState folder into a simple char array:
	const wchar_t* str = fullPath->Data();
	size_t num;
	wcstombs_s(&num, fileBuffer, 1000, str, fullPath->Length() + 1);

	return fileBuffer;
}

void TestApp::MainPage::RunTest_OneTimeScan()
{
	SetModeFrame_Inside();
	DebugData_StaticMesh_LoadAndSet(CalcInputMeshFilename(), true);

	DebugData_GeneratePlayspace_OneTimeScan();

	// Pull out the mesh
	int vertexCount, indexCount;
	GeneratePlayspace_ExtractMesh_Setup(&vertexCount, &indexCount);
	DirectX::XMFLOAT3* verticesPos = new DirectX::XMFLOAT3[vertexCount];
	DirectX::XMFLOAT3* verticesNormal = new DirectX::XMFLOAT3[vertexCount];
	int* indices = new int[indexCount];
	GeneratePlayspace_ExtractMesh_Extract(vertexCount, verticesPos, verticesNormal, indexCount, indices);

	// Cleanup
	delete[] verticesPos;
	delete[] verticesNormal;
	delete[] indices;
}

#define REALTIME_TEST_TIME	10.0f
#define REALTIME_TEST_COUNT	10
#define REALTIME_FRAMETIME  (1.0f / 60.0f)
void TestApp::MainPage::RunTest_RealTimeScan_StaticInputData()
{
	// Setup
	DirectX::XMFLOAT3 camPos = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
	DirectX::XMFLOAT3 camFwd = DirectX::XMFLOAT3(0.0f, 0.0f, 1.0f);
	DirectX::XMFLOAT3 camUp  = DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f);

	// First Set Frame Mode 
	SetModeFrame_Inside();

	// Debug mesh data
	DebugData_StaticMesh_LoadAndSet(CalcInputMeshFilename(), true);

	// Init the scan
	GeneratePlayspace_InitScan(camPos.x, camPos.y, camPos.z, camFwd.x, camFwd.y, camFwd.z, camUp.x, camUp.y, camUp.z, 7.0f, 7.0f);

	// Real-Time loop
	float timePerUpdate = REALTIME_TEST_TIME / (float)REALTIME_TEST_COUNT;
	for (int i = 0; i < REALTIME_TEST_COUNT; ++i)
	{
		int frameCount = (int)ceil(timePerUpdate / REALTIME_FRAMETIME);
		for (int j = 0; j < frameCount; ++j)
		{
			GeneratePlayspace_UpdateScan_StaticScan(camPos.x, camPos.y, camPos.z, camFwd.x, camFwd.y, camFwd.z, camUp.x, camUp.y, camUp.z, REALTIME_FRAMETIME);

			Sleep((int)(REALTIME_FRAMETIME * 1000.0f));
		}
	}

	// Request finish & then wait till done
	GeneratePlayspace_RequestFinish();
	while (true)
	{
		if (!GeneratePlayspace_UpdateScan_StaticScan(camPos.x, camPos.y, camPos.z, camFwd.x, camFwd.y, camFwd.z, camUp.x, camUp.y, camUp.z, REALTIME_FRAMETIME))
		{
			break;
		}
		Sleep((int)(REALTIME_FRAMETIME * 1000.0f));
	}

	// Pull out the mesh
	int vertexCount, indexCount;
	GeneratePlayspace_ExtractMesh_Setup(&vertexCount, &indexCount);
	DirectX::XMFLOAT3* verticesPos = new DirectX::XMFLOAT3[vertexCount];
	DirectX::XMFLOAT3* verticesNormal = new DirectX::XMFLOAT3[vertexCount];
	int* indices = new int[indexCount];
	GeneratePlayspace_ExtractMesh_Extract(vertexCount, verticesPos, verticesNormal, indexCount, indices);

	// Look through he data (this is just for debugging)
	for (int i = 0; i < vertexCount; ++i)
	{
		DirectX::XMFLOAT3 pos = verticesPos[i];
		DirectX::XMFLOAT3 norm = verticesNormal[i];
	}

	// Cleanup
	delete[] verticesPos;
	delete[] verticesNormal;
	delete[] indices;
}

void TestApp::MainPage::RunTest_RealTimeScan_DynamicInputData()
{
	// Open the trace file
	const char* fileName = CalcInputDynMeshTestFilename();
	FILE* f = NULL;
	fopen_s(&f, fileName, "rb");
	if (f == NULL)
	{
		return;
	}

	// Init
	if (!DebugData_LoadAndSet_DynamicScan_InitScan(f))
	{
		return;
	}

	// Dynamic updates
	while (DebugData_LoadAndSet_DynamicScan_UpdateScan(f))
	{
		Sleep((DWORD)(1000.0f / 60.0f));
	}

	// Pull out the mesh
	int vertexCount, indexCount;
	GeneratePlayspace_ExtractMesh_Setup(&vertexCount, &indexCount);
	DirectX::XMFLOAT3* verticesPos = new DirectX::XMFLOAT3[vertexCount];
	DirectX::XMFLOAT3* verticesNormal = new DirectX::XMFLOAT3[vertexCount];
	int* indices = new int[indexCount];
	GeneratePlayspace_ExtractMesh_Extract(vertexCount, verticesPos, verticesNormal, indexCount, indices);

	// Look through he data (this is just for debugging)
	for (int i = 0; i < vertexCount; ++i)
	{
		DirectX::XMFLOAT3 pos = verticesPos[i];
		DirectX::XMFLOAT3 norm = verticesNormal[i];
	}

	// Cleanup
	delete[] verticesPos;
	delete[] verticesNormal;
	delete[] indices;
}
