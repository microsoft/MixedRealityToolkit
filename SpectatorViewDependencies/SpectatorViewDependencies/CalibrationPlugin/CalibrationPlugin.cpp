#include "stdafx.h"
#include "Calibration.h"

std::unique_ptr<Calibration> calibration;

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
