#include "SpectatorViewPlugin.h"
#include "..\SharedFiles\ArUcoMarkerDetector.h"
#include "..\SharedFiles\Calibration.h"

std::unique_ptr<Calibration> calibration;
std::unique_ptr<ArUcoMarkerDetector> detector;

// Returns True to signal that this dll has been accessed correctly by its caller.
extern "C" __declspec(dllexport) bool __stdcall Initialize()
{
    if (!detector)
    {
        detector = std::make_unique<ArUcoMarkerDetector>();
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

// Returns True to signal that this dll has been accessed correctly by its caller.
extern "C" __declspec(dllexport) bool __stdcall InitializeCalibration()
{
    if (!calibration)
    {
        calibration = std::make_unique<Calibration>();
    }

    return calibration->Initialize();
}

extern "C" __declspec(dllexport) bool __stdcall ProcessArUcoData(
    unsigned char* image,
    int imageWidth,
    int imageHeight,
    int* markerIds,
    int numMarkers,
    float* markerCornersInWorld,
    float* markerCornersRelativeToCamera,
    float* planarCorners,
    int numTotalCorners,
    float* orientation)
{
    if (calibration)
    {
        return calibration->ProcessArUcoData(
            image,
            imageWidth,
            imageHeight,
            markerIds,
            numMarkers,
            markerCornersInWorld,
            markerCornersRelativeToCamera,
            planarCorners,
            numTotalCorners,
            orientation);
    }

    return false;
}

extern "C" __declspec(dllexport) bool __stdcall ProcessArUcoIntrinsics(
    float* intrinsics,
    int numIntrinsics)
{
    if (calibration)
    {
        return calibration->ProcessArUcoIntrinsics(
            intrinsics,
            numIntrinsics);
    }

    return false;
}

extern "C" __declspec(dllexport) bool __stdcall ProcessIndividualArUcoExtrinsics(
    float* intrinsics,
    float* extrinsics,
    int numExtrinsics)
{
    if (calibration)
    {
        return calibration->ProcessIndividualArUcoExtrinsics(
            intrinsics,
            extrinsics,
            numExtrinsics);
    }

    return false;
}

extern "C" __declspec(dllexport) bool __stdcall ProcessGlobalArUcoExtrinsics(
    float* intrinsics,
    float* extrinsics,
    int numExtrinsics)
{
    if (calibration)
    {
        return calibration->ProcessGlobalArUcoExtrinsics(
            intrinsics,
            extrinsics,
            numExtrinsics);
    }

    return false;
}

// Gets the last error message associated with calibration
extern "C" __declspec(dllexport) bool __stdcall GetLastErrorMessage(
    char* buff,
    int size)
{
    if (calibration)
    {
        return calibration->GetLastErrorMessage(buff, size);
    }

    return false;
}

extern "C" __declspec(dllexport) bool __stdcall ProcessChessboardImage(
    unsigned char* image,
    int imageWidth,
    int imageHeight,
    int boardWidth,
    int boardHeight,
    unsigned char* cornersImage,
    unsigned char* heatmapImage,
    int cornerImageRadias,
    int heatmapWidth)
{
    if (calibration)
    {
        return calibration->ProcessChessboardImage(
            image,
            imageWidth,
            imageHeight,
            boardWidth,
            boardHeight,
            cornersImage,
            heatmapImage,
            cornerImageRadias,
            heatmapWidth);
    }

    return false;
}

extern "C" __declspec(dllexport) bool __stdcall ProcessChessboardIntrinsics(
    float squareSize,
    float* intrinsics,
    int numIntrinsics)
{
    if (calibration)
    {
        return calibration->ProcessChessboardIntrinsics(
            squareSize,
            intrinsics,
            numIntrinsics);
    }

    return false;
}
