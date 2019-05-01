#include "stdafx.h"
#include "Calibration.h"

std::unique_ptr<Calibration> calibration;

// Returns True to signal that this dll has been accessed correctly by its caller.
extern "C" __declspec(dllexport) bool __stdcall InitializeCalibration()
{
    if (!calibration)
    {
        calibration = std::make_unique<Calibration>();
    }

    return calibration->Initialize();
}

// Processes a BGRA image for calibration
// image array size should be equal to 4 * image width * image height
// markerIds array size should be equal to numMarkers
// markerCornersInWorld array size should be equal to 3 * numTotalCorners
// numTotalCorners should be equal to markerIds * 4
// orientation should contain 7 floats (3 for position, 4 for rotation (quaternion))
extern "C" __declspec(dllexport) bool __stdcall ProcessImage(
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
        return calibration->ProcessImage(
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

// numIntrinsics should reflect the number of intrinsics that can be stored in the provided float array
// intrinsics should be large enough for the following:
// 2 floats - focal length
// 2 floats - principal point
// 3 floats - radial distortion
// 2 floats - tangential distortion
// 1 float - image width
// 1 float - image height
extern "C" __declspec(dllexport) bool __stdcall ProcessIntrinsics(
    float* intrinsics,
    int numIntrinsics)
{
    if (calibration)
    {
        return calibration->ProcessIntrinsics(
            intrinsics,
            numIntrinsics);
    }

    return false;
}

extern "C" __declspec(dllexport) bool __stdcall ProcessExtrinsics(
    float* intrinsics,
    float* extrinsics,
    int numExtrinsics)
{
    if (calibration)
    {
        return calibration->ProcessExtrinsics(
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
