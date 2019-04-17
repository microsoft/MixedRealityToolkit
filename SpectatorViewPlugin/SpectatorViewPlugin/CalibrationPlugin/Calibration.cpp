#include "pch.h"
#include "Calibration.h"

using namespace CalibrationPlugin::Native;

bool Calibration::ProcessImage(
    const Platform::Array<byte>^ image,
    int imageWidth,
    int imageHeight,
    const Platform::Array<ArUcoCorners^>^ worldCorners,
    UnityCameraOrientation^ orientation)
{
    if ((width != imageWidth && width != 0) ||
        (height != imageHeight && height != 0))
    {
        throw ref new Platform::InvalidArgumentException(L"Calibration does not support images with different sizes");
    }

    width = imageWidth;
    height = imageHeight;

    std::map<int, ArUcoCorners^> worldCornersMap;
    for (unsigned int i = 0; i < worldCorners->Length; i++)
    {
        worldCornersMap.insert(std::pair<int, ArUcoCorners^>(worldCorners[i]->markerId, worldCorners[i]));
    }

    // This dll assumes that its handed pixels in BGRA format
    cv::Mat bgraImage(height, width, CV_8UC4, image->Data);

    // ArUco detection with opencv does not support images with alpha channels
    // So, we convert the image to grayscale for processing
    cv::Mat grayImage;
    cv::cvtColor(bgraImage, grayImage, CV_BGRA2GRAY);

    std::vector<int> arUcoMarkerIds;
    std::vector<std::vector<cv::Point2f>> arUcoMarkerCorners;
    std::vector<std::vector<cv::Point2f>> arUcoRejectedCandidates;
    auto arUcoDetectorParameters = cv::aruco::DetectorParameters::create();
    auto arUcoDictionary = cv::aruco::getPredefinedDictionary(cv::aruco::PREDEFINED_DICTIONARY_NAME(cv::aruco::DICT_6X6_250));

    // Detect markers
    cv::aruco::detectMarkers(
        grayImage,
        arUcoDictionary,
        arUcoMarkerCorners,
        arUcoMarkerIds,
        arUcoDetectorParameters,
        arUcoRejectedCandidates);

    std::vector<cv::Vec3f> worldPoints;
    std::vector<cv::Vec2f> imagePoints;
    for (size_t i = 0; i < arUcoMarkerIds.size(); i++)
    {
        auto id = arUcoMarkerIds.at(i);
        if (worldCornersMap.find(id) != worldCornersMap.end())
        {
            auto worldCorners = worldCornersMap.at(id);
            auto imageCorners = arUcoMarkerCorners.at(i);

            // OpenCV reports ArUco marker corners in a clockwise manner
            worldPoints.push_back(cv::Vec3f(worldCorners->topLeft.x, worldCorners->topLeft.y, worldCorners->topLeft.z));
            worldPoints.push_back(cv::Vec3f(worldCorners->topRight.x, worldCorners->topRight.y, worldCorners->topRight.z));
            worldPoints.push_back(cv::Vec3f(worldCorners->bottomRight.x, worldCorners->bottomRight.y, worldCorners->bottomRight.z));
            worldPoints.push_back(cv::Vec3f(worldCorners->bottomLeft.x, worldCorners->bottomLeft.y, worldCorners->bottomLeft.z));

            imagePoints.push_back(cv::Vec2f(imageCorners.at(0)));
            imagePoints.push_back(cv::Vec2f(imageCorners.at(1)));
            imagePoints.push_back(cv::Vec2f(imageCorners.at(2)));
            imagePoints.push_back(cv::Vec2f(imageCorners.at(3)));
        }
    }

    worldPointObservations.push_back(worldPoints);
    imagePointObservations.push_back(imagePoints);
    return true;
}

MultiIntrinsics^ Calibration::ProcessIntrinsics()
{
    auto multiIntrinsics = ref new MultiIntrinsics();

    // basic calculation
    cv::Mat cameraMat, distCoeffMat, rvecsMat, tvecsMat;
    double reprojectionError = cv::calibrateCamera(worldPointObservations, imagePointObservations, cv::Size(width, height), cameraMat, distCoeffMat, rvecsMat, tvecsMat);
    multiIntrinsics->Standard = CreateIntrinsics(reprojectionError, cameraMat, distCoeffMat);

    // precalculated camera mat - 0.5 principal points, equal focal points
    cv::Mat cameraMatPreCalc, distCoeffMatPreCalc, rvecsMatPreCalc, tvecsMatPreCalc;
    cameraMatPreCalc.at<double>(0, 0) = 1.0f* width; // X focal length
    cameraMatPreCalc.at<double>(0, 2) = 0.5f * width; // X principal point
    cameraMatPreCalc.at<double>(1, 1) = 1.0f * height; // Y focal length
    cameraMatPreCalc.at<double>(1, 2) = 0.5f * height; // Y principal point
    cameraMatPreCalc.at<double>(2, 2) = 1.0; // Default value for camera intrinsic matrix
    double reprojectionErrorPre = cv::calibrateCamera(worldPointObservations, imagePointObservations, cv::Size(width, height), cameraMatPreCalc, distCoeffMatPreCalc, rvecsMatPreCalc, tvecsMatPreCalc);
    multiIntrinsics->PreCalcMatrix = CreateIntrinsics(reprojectionError, cameraMat, distCoeffMat);

    // TODO - standardized world points (updated based on camera position/orientation)

    // TODO - planar world points

    return multiIntrinsics;
}

Intrinsics^ Calibration::CreateIntrinsics(
    double reprojectionError,
    cv::Mat cameraMat,
    cv::Mat distCoeff)
{
    auto intrinsics = ref new Intrinsics();
    intrinsics->reprojectionError = reprojectionError;
    intrinsics->focalLength = Windows::Foundation::Numerics::float2(
        static_cast<float>(cameraMat.at<double>(0, 0)),
        static_cast<float>(cameraMat.at<double>(1, 1)));
    intrinsics->principalPoint = Windows::Foundation::Numerics::float2(
        static_cast<float>(cameraMat.at<double>(0, 2)),
        static_cast<float>(cameraMat.at<double>(1, 2)));
    intrinsics->radialDistortion = Windows::Foundation::Numerics::float3(
        static_cast<float>(distCoeff.at<double>(0, 0)),
        static_cast<float>(distCoeff.at<double>(0, 1)),
        static_cast<float>(distCoeff.at<double>(0, 4)));
    intrinsics->tangentialDistortion = Windows::Foundation::Numerics::float2(
        static_cast<float>(distCoeff.at<double>(0, 2)),
        static_cast<float>(distCoeff.at<double>(0, 3)));

    return intrinsics;
}