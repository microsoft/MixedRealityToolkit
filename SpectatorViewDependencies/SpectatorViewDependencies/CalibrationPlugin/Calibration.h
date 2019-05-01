#pragma once

class Calibration sealed
{
public:
    Calibration() {}

    bool Initialize();

    // image array size should be equal to 4 * image width * image height
    // markerIds array size should be equal to numMarkers
    // markerCornersInWorld array size should be equal to 3 * numTotalCorners
    // numMarkerCornerValues should be equal to markerIds * 4 * 3
    // orientation should contain 7 floats (3 for position, 4 for rotation (quaternion))
    bool ProcessImage(
        unsigned char* image,
        int imageWidth,
        int imageHeight,
        int* markerIds,
        int numMarkers,
        float* markerCornersInWorld,
        float* markerCornersRelativeToCamera,
        float* planarCorners,
        int numMarkerCornerValues,
        float* orientation);

    // numIntrinsics should reflect the number of intrinsics that can be stored in the provided float array
    // intrinsics should be large enough for the following:
    // 2 floats - focal length
    // 2 floats - principal point
    // 3 floats - radial distortion
    // 2 floats - tangential distortion
    // 1 float - image width
    // 1 float - image height
    // 1 float - reprojection error
    bool ProcessIntrinsics(
        float* intrinsics,
        int numIntrinsics);

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
        int numIntrinsics);

    bool ProcessExtrinsics(
        float* intrinsics,
        float* extrinsics,
        int numExtrinsics);

    bool GetLastErrorMessage(
        char* buff,
        int size);

private:
    struct corners
    {
        cv::Point3f topLeft;
        cv::Point3f topRight;
        cv::Point3f bottomRight;
        cv::Point3f bottomLeft;
    };

    void StoreIntrinsics(
        double reprojectionError,
        cv::Mat cameraMat,
        cv::Mat distCoeff,
        int width,
        int height,
        float* intrinsics);

    void StoreExtrinsics(
        double reprojectionError,
        cv::Mat rvec,
        cv::Mat tvec,
        float* extrinsics);

    int width = 0;
    int height = 0;
    std::vector<std::vector<cv::Point3f>> worldPointObservations;
    std::vector<std::vector<cv::Point2f>> imagePointObservations;
    std::vector<std::vector<cv::Point3f>> pointObservationsRelativeToCamera;
    std::vector<std::vector<cv::Point3f>> planarPointObservations;

    int chessboardImageWidth = 0;
    int chessboardImageHeight = 0;
    int chessboardWidth = 0;
    int chessboardHeight = 0;
    std::vector<std::vector<cv::Point2f>> chessboardImagePointObservations;
    std::string lastError;
};