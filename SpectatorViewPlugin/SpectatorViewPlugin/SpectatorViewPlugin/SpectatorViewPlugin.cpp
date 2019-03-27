#include "pch.h"
#include "SpectatorViewPlugin.h"
#include "MarkerDetector.h"
#include <memory>

std::unique_ptr<MarkerDetector> detector;

// Returns True to signal that this dll has been accessed correctly by its caller.
extern "C" __declspec(dllexport) bool __stdcall Initialize()
{
    if (!detector)
    {
        detector = std::make_unique<MarkerDetector>();
    }

    return true;
}

// Detects ArUco markers in a BGRA image
// imageData - byte data for BGRA image frame
// imageWidth - image width in pixels
// imageHeight - image height in pixels
// focalLength - float[] with 2 elements
// principalPoint - float[] with 2 elements
// radialDistortion - float[] with 3 elements
// tangentialDistortion - float[] with 2 elements
// markerSize - size of ArUco marker in meters
// arUcoMarkerDictionaryid - id for opencv ArUco marker dictionary to use for detection
extern "C" __declspec(dllexport) bool __stdcall DetectMarkers(
    unsigned char* imageData,
    int imageWidth,
    int imageHeight,
    float* focalLength,
    float* principalPoint,
    float* radialDistortion,
    float* tangentialDistortion,
    float markerSize,
    int arUcoMarkerDictionaryId)
{
    if (detector)
    {
        return detector->DetectMarkers(
            imageData,
            imageWidth,
            imageHeight,
            focalLength,
            principalPoint,
            radialDistortion,
            tangentialDistortion,
            markerSize,
            arUcoMarkerDictionaryId);
    }

    return false;
}

// Returns the number of detected markers in the last processed image
extern "C" __declspec(dllexport) int __stdcall GetDetectedMarkersCount()
{
    if (detector)
    {
        return detector->GetDetectedMarkersCount();
    }

    return -1;
}

// Attempts to populate the provided array with ArUco marker ids
// Returns False if the array size is less than the number of detected markers
// detectedIds - int[] populated with ids
// size - size of the int[]
extern "C" __declspec(dllexport) bool __stdcall GetDetectedMarkerids(
    int* detectedIds,
    int size)
{
    if (detector)
    {
        return detector->GetDetectedMarkerIds(detectedIds, size);
    }

    return false;
}

// Attempts to obtain position and rotation information for a specified marker
// Returns False if the specificed marker has not been detected
// detectedId - marker id
// position - float[] with 3 elements populated with marker's position
// rotation - float[] with 3 elements populated with marker's rotation (rodrigues vector not euler angles)
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
