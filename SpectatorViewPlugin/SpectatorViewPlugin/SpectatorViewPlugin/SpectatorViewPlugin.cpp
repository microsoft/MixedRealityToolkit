#include "pch.h"
#include "SpectatorViewPlugin.h"
#include "MarkerDetector.h"

std::unique_ptr<MarkerDetector> detector;

extern "C" __declspec(dllexport) bool __stdcall Initialize()
{
    if (!detector)
    {
        detector = std::make_unique<MarkerDetector>();
    }

    return true;
}

extern "C" __declspec(dllexport) bool __stdcall DetectMarkers(
    unsigned char* imageData,
    int imageWidth,
    int imageHeight,
    float* projectionMatrix,
    float markerSize,
    int arUcoMarkerDictionaryId)
{
    if (detector)
    {
        return detector->DetectMarkers(
            imageData,
            imageWidth,
            imageHeight,
            projectionMatrix,
            markerSize,
            arUcoMarkerDictionaryId);
    }

    return false;
}

extern "C" __declspec(dllexport) int __stdcall GetDetectedMarkersCount()
{
    if (detector)
    {
        return detector->GetDetectedMarkersCount();
    }

    return -1;
}

extern "C" __declspec(dllexport) bool __stdcall GetDetectedMarkerIds(
    int* detectedIds,
    int size)
{
    if (detector)
    {
        return detector->GetDetectedMarkerIds(detectedIds, size);
    }

    return false;
}

extern "C" __declspec(dllexport) bool __stdcall GetDetectedMarkerPose(
    int detectedId,
    float* position,
    float* rotation)
{
    if (detector)
    {
        return detector->GetDetectedMarkerPose(detectedId, position, rotation);
    }

    return false;
}
