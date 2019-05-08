#pragma once
#include "..\SharedFiles\pch.h"

class Calibration sealed
{
public:
    Calibration() {}

    void Initialize();

    bool ProcessChessboardImage(
        unsigned char* image,
        int imageWidth,
        int imageHeight,
        int boardWidth,
        int boardHeight,
        unsigned char* cornersImage,
        unsigned char* heatmapImage,
        int cornerImageRadias,
        int heatmapWidth);

    bool ProcessChessboardIntrinsics(
        float squareSize,
        float* intrinsics,
        int sizeIntrinsics);

    bool ProcessArUcoData(
        unsigned char* image,
        int imageWidth,
        int imageHeight,
        int* markerIds,
        int numMarkers,
        float* markerCornersInWorld,
        float* markerCornersRelativeToCamera,
        int numMarkerCornerValues);

    bool ProcessIndividualArUcoExtrinsics(
        float* intrinsics,
        float* extrinsics,
        int sizeExtrinsics,
        int numExtrinsics);

    bool ProcessGlobalArUcoExtrinsics(
        float* intrinsics,
        float* extrinsics,
        int sizeExtrinsics);

    bool GetLastErrorMessage(
        char* buff,
        int size);

private:
    void StoreIntrinsics(
        double reprojectionError,
        cv::Mat cameraMat,
        cv::Mat distCoeff,
        int width,
        int height,
        float* intrinsics);

    void StoreExtrinsics(
        float succeeded,
        cv::Mat rvec,
        cv::Mat tvec,
        float* extrinsics);

    int width = 0;
    int height = 0;
    std::vector<std::vector<cv::Point3f>> worldPointObservations;
    std::vector<std::vector<cv::Point2f>> imagePointObservations;
    std::vector<std::vector<cv::Point3f>> pointObservationsRelativeToCamera;

    int chessboardImageWidth = 0;
    int chessboardImageHeight = 0;
    int chessboardWidth = 0;
    int chessboardHeight = 0;
    std::vector<std::vector<cv::Point2f>> chessboardImagePointObservations;
    std::string lastError;
};