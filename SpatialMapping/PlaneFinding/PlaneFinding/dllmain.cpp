#include "pch.h"
#include "PlaneFinding.h"
#include "Objbase.h"

BOOL APIENTRY DllMain(HMODULE /* hModule */, DWORD ul_reason_for_call, LPVOID /* lpReserved */)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

// utility function for copying an array of planes into a CoTaskMemAlloc'ed buffer that can be handed off to an external caller
void _MarshalPlanes(
    _In_ const vector<PlaneFinding::BoundedPlane>& planes,
    _Out_ INT32* count,
    _Out_ PlaneFinding::BoundedPlane** buffer)
{
    int bufferSize = sizeof(PlaneFinding::BoundedPlane) * planes.size();

    *count = planes.size();
    *buffer = (PlaneFinding::BoundedPlane*)CoTaskMemAlloc(bufferSize);
    memcpy(*buffer, planes.data(), bufferSize);
}


EXTERN_C __declspec(dllexport) void FindSubPlanes(
    _In_ INT32 meshCount,
    _In_count_(meshCount) PlaneFinding::MeshData* meshes,
    _In_ FLOAT snapToGravityThreshold,
    _Out_ INT32* planeCount,
    _Outptr_result_buffer_(planeCount) PlaneFinding::BoundedPlane** planes)
{
    vector<PlaneFinding::BoundedPlane> vPlanes = PlaneFinding::FindPlanes(meshCount, meshes, snapToGravityThreshold);
    _MarshalPlanes(vPlanes, planeCount, planes);
}

EXTERN_C __declspec(dllexport) void MergeSubPlanes(
    _In_ INT32 subPlaneCount,
    _In_count_(subPlaneCount) PlaneFinding::BoundedPlane* subPlanes,
    _In_ FLOAT minArea,
    _In_ FLOAT snapToGravityThreshold,
    _Out_ INT32* planeCount,
    _Outptr_result_buffer_(planeCount) PlaneFinding::BoundedPlane** planes)
{
    vector<PlaneFinding::BoundedPlane> vPlanes = PlaneFinding::MergePlanes(subPlaneCount, subPlanes, minArea, snapToGravityThreshold);
    _MarshalPlanes(vPlanes, planeCount, planes);
}

EXTERN_C __declspec(dllexport) void FindPlanes(
    _In_ INT32 meshCount,
    _In_count_(meshCount) PlaneFinding::MeshData* meshes,
    _In_ FLOAT minArea,
    _In_ FLOAT snapToGravityThreshold,
    _Out_ INT32* planeCount,
    _Outptr_result_buffer_(planeCount) PlaneFinding::BoundedPlane** planes)
{
    vector<PlaneFinding::BoundedPlane> subPlanes = PlaneFinding::FindPlanes(meshCount, meshes, snapToGravityThreshold);
    vector<PlaneFinding::BoundedPlane> vPlanes = PlaneFinding::MergePlanes(subPlanes.size(), subPlanes.data(), minArea, snapToGravityThreshold);
    _MarshalPlanes(vPlanes, planeCount, planes);
}