#pragma once

class Calibration sealed
{
public:
    Calibration() {}

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

private:
    struct float3
    {
        float x;
        float y;
        float z;
    };

    struct corners
    {
        float3 topLeft;
        float3 topRight;
        float3 bottomRight;
        float3 bottomLeft;
    };

    void CreateIntrinsics(
        double reprojectionError,
        cv::Mat cameraMat,
        cv::Mat distCoeff,
        std::vector<float>& intrinsics);

    int width = 0;
    int height = 0;
    std::vector<std::vector<cv::Vec3f>> worldPointObservations;
    std::vector<std::vector<cv::Vec2f>> imagePointObservations;
};