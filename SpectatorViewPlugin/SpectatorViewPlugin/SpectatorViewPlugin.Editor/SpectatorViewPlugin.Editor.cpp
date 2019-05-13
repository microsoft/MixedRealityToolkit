#include "..\SharedFiles\pch.h"

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

// Returns True to signal that this dll has been accessed correctly for calibration
extern "C" __declspec(dllexport) bool __stdcall InitializeCalibration()
{
    if (!calibration)
    {
        calibration = std::make_unique<Calibration>();
        calibration->Initialize();
        return true;
    }

    return true;
}

// Returns True if the calibration data was reset
extern "C" __declspec(dllexport) bool __stdcall ResetCalibration()
{
    if (calibration)
    {
        calibration->Initialize();
        return true;
    }

    return false;
}

// Returns True if ArUco markers were found in the provided image. Said markers should also have corresponding coordinates in world space provided.
// image - byte data for RGB image frame
// imageWidth - image width in pixels
// imageHeight - image height in pixels
// numMarkers - the number of ArUco markers detected in world space
// markerCornersInWorld - corners for the detected ArUco markers in world space
// markerCornersRelativeToCamera - corners for the detected ArUco markers in world space corrected for the unity camera offset
// numTotalCorners - total number of corners provided
extern "C" __declspec(dllexport) bool __stdcall ProcessArUcoData(
    unsigned char* image,
    int imageWidth,
    int imageHeight,
    int* markerIds,
    int numMarkers,
    float* markerCornersInWorld,
    float* markerCornersRelativeToCamera,
    int numTotalCorners)
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
            numTotalCorners);
    }

    return false;
}

// Processes chessboard images in order to calculate camera intrinsics
// image - byte data for the image. This data should be in the RGB24 format
// imageWidth - width of image in pixels
// imageHeight - height of image in pixels
// boardWidth - width of the chessboard used
// boardHeight - height of the chessboard used
// cornersImage - helper image byte data for displaying where chessboard corners were detected, should be same format and dimensions as main image
// heatmapImage - helper image byte data for displaying a heatmap of detected chessboard corners, should be same format and dimensions as main image
// cornerImageRadias - radias of circles to draw at chessboard corners in the cornersImage
// heatmapWidth - width of heatmap regions to draw in the heatmapImage
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

// Calculates the camera intrinsics based on the provided chessboard images
// squareSize - size of a chessboard square in meteres
// intrinsis - output intrinsics
// sizeExtrinsics - size of extrinsics element in array in floats
extern "C" __declspec(dllexport) bool __stdcall ProcessChessboardIntrinsics(
    float squareSize,
    float* intrinsics,
    int sizeIntrinsics)
{
    if (calibration)
    {
        return calibration->ProcessChessboardIntrinsics(
            squareSize,
            intrinsics,
            sizeIntrinsics);
    }

    return false;
}

// Calculates extrinsics for each provided data series. This helps with visualizing/debugging.
// Returns True if ArUco marker data was available for processing and enough output extrinsics values were provided
// intrinsics - camera intrinsics to use for extrinsics calculations
// extrinsics - output camera extrinsics
// sizeExtrinsics - size of extrinsics element in array in floats
// numExtrinsics - the number of extrinsics available for output
extern "C" __declspec(dllexport) bool __stdcall ProcessIndividualArUcoExtrinsics(
    float* intrinsics,
    float* extrinsics,
    int sizeExtrinsics,
    int numExtrinsics)
{
    if (calibration)
    {
        return calibration->ProcessIndividualArUcoExtrinsics(
            intrinsics,
            extrinsics,
            sizeExtrinsics,
            numExtrinsics);
    }

    return false;
}

// Calculates camera extrinsics using all of the provided ArUco data series.
// Returns True if ArUco marker data was available for processing and enough output extrinsics values were provided
// intrinsics - camera intrinsics to use for extrinsics calculations
// extrinsics - output camera extrinsics
// numExtrinsics - the number of extrinsics available for output
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
// buff - output character buffer
// size - size of buffer in characters, not memory size
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