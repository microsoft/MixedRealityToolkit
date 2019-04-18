#include "stdafx.h"
#include "Calibration.h"

bool Calibration::ProcessImage(
    unsigned char* image,
    int imageWidth,
    int imageHeight,
    int* markerIds,
    int numMarkers,
    float* markerCornersInWorld,
    int numTotalCorners,
    float* orientation)
{
    if (numTotalCorners != numMarkers * 4)
    {
        OutputDebugString(L"The number of total corners did not equal what was expected");
        return false;
    }

    if ((width != imageWidth && width != 0) ||
        (height != imageHeight && height != 0))
    {
        OutputDebugString(L"Calibration does not support different image dimensions");
        return false;
    }

    width = imageWidth;
    height = imageHeight;

    std::unordered_map<int, corners> worldCornersMap;
    int floatsPerCorner = 3;
    int floatsPerMarker = floatsPerCorner * 4;
    for (int i = 0; i < numMarkers; i++)
    {
        auto markerId = markerIds[i];
        corners currCorners{
            float3{
                markerCornersInWorld[floatsPerMarker * i],
                markerCornersInWorld[floatsPerMarker * i + 1],
                markerCornersInWorld[floatsPerMarker * i + 2]
            },
            float3{
                markerCornersInWorld[floatsPerMarker * i + 3],
                markerCornersInWorld[floatsPerMarker * i + 4],
                markerCornersInWorld[floatsPerMarker * i + 5]
            },
            float3{
                markerCornersInWorld[floatsPerMarker * i + 6],
                markerCornersInWorld[floatsPerMarker * i + 7],
                markerCornersInWorld[floatsPerMarker * i + 8]
            },
            float3{
                markerCornersInWorld[floatsPerMarker * i + 9],
                markerCornersInWorld[floatsPerMarker * i + 10],
                markerCornersInWorld[floatsPerMarker * i + 11]
            },
        };
        worldCornersMap[markerId] = currCorners;
    }

    // This dll assumes that its handed pixels in BGRA format
    cv::Mat bgraImage(height, width, CV_8UC4, image);

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

    auto text = L"Detected " + std::to_wstring(arUcoMarkerIds.size()) + L" markers in the provided image";
    OutputDebugString(text.data());

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
            worldPoints.push_back(cv::Vec3f(worldCorners.topLeft.x, worldCorners.topLeft.y, worldCorners.topLeft.z));
            worldPoints.push_back(cv::Vec3f(worldCorners.topRight.x, worldCorners.topRight.y, worldCorners.topRight.z));
            worldPoints.push_back(cv::Vec3f(worldCorners.bottomRight.x, worldCorners.bottomRight.y, worldCorners.bottomRight.z));
            worldPoints.push_back(cv::Vec3f(worldCorners.bottomLeft.x, worldCorners.bottomLeft.y, worldCorners.bottomLeft.z));

            imagePoints.push_back(cv::Vec2f(imageCorners.at(0)));
            imagePoints.push_back(cv::Vec2f(imageCorners.at(1)));
            imagePoints.push_back(cv::Vec2f(imageCorners.at(2)));
            imagePoints.push_back(cv::Vec2f(imageCorners.at(3)));
        }
        else
        {
            text = L"Marker " + std::to_wstring(arUcoMarkerIds.size()) + L" was detected in image but did not have known world coordinates";
            OutputDebugString(text.data());
        }
    }

    worldPointObservations.push_back(worldPoints);
    imagePointObservations.push_back(imagePoints);
    return true;
}

bool Calibration::ProcessIntrinsics(float* intrinsics, int numIntrinsics)
{
    const size_t intrinsicSize = 12;
    if (numIntrinsics != 2)
    {
        OutputDebugString(L"Calibration expects to return two intrinsics values");
        return false;
    }

    if (worldPointObservations.size() < 1)
    {
        OutputDebugString(L"Images must be processed before conducting calibration");
        return false;
    }

    std::vector<float> output;

    // basic calculation
    cv::Mat cameraMat, distCoeffMat, rvecsMat, tvecsMat;
    double reprojectionError = cv::calibrateCamera(worldPointObservations, imagePointObservations, cv::Size(width, height), cameraMat, distCoeffMat, rvecsMat, tvecsMat);
    CreateIntrinsics(reprojectionError, cameraMat, distCoeffMat, output);

    // precalculated camera mat - 0.5 principal points, equal focal points
    cv::Mat cameraMatPreCalc, distCoeffMatPreCalc, rvecsMatPreCalc, tvecsMatPreCalc;
    cameraMatPreCalc.at<double>(0, 0) = 1.0f* width; // X focal length
    cameraMatPreCalc.at<double>(0, 2) = 0.5f * width; // X principal point
    cameraMatPreCalc.at<double>(1, 1) = 1.0f * height; // Y focal length
    cameraMatPreCalc.at<double>(1, 2) = 0.5f * height; // Y principal point
    cameraMatPreCalc.at<double>(2, 2) = 1.0; // Default value for camera intrinsic matrix
    double reprojectionErrorPreCalc = cv::calibrateCamera(worldPointObservations, imagePointObservations, cv::Size(width, height), cameraMatPreCalc, distCoeffMatPreCalc, rvecsMatPreCalc, tvecsMatPreCalc);
    CreateIntrinsics(reprojectionErrorPreCalc, cameraMatPreCalc, distCoeffMatPreCalc, output);

    // TODO - standardized world points (updated based on camera position/orientation)
    // TODO - planar world points

    return memcpy(intrinsics, output.data(), numIntrinsics * intrinsicSize);
    return true;
}

void Calibration::CreateIntrinsics(
    double reprojectionError,
    cv::Mat cameraMat,
    cv::Mat distCoeff,
    std::vector<float>& intrinsics)
{
    intrinsics.push_back(static_cast<float>(cameraMat.at<double>(0, 0))); // X Focal length
    intrinsics.push_back(static_cast<float>(cameraMat.at<double>(1, 1))); // Y Focal length
    intrinsics.push_back(static_cast<float>(cameraMat.at<double>(0, 2))); // X Principal point
    intrinsics.push_back(static_cast<float>(cameraMat.at<double>(1, 2))); // Y Principal point
    intrinsics.push_back(static_cast<float>(distCoeff.at<double>(0, 0))); // 1st Radial distortion coefficient
    intrinsics.push_back(static_cast<float>(distCoeff.at<double>(0, 1))); // 2nd Radial distortion coefficient
    intrinsics.push_back(static_cast<float>(distCoeff.at<double>(0, 4))); // 3rd Radial distortion coefficient
    intrinsics.push_back(static_cast<float>(distCoeff.at<double>(0, 2))); // 1st Tangential distortion coefficient
    intrinsics.push_back(static_cast<float>(distCoeff.at<double>(0, 3))); // 2nd Tsangential distortion coefficient
    intrinsics.push_back(static_cast<float>(width)); // Image width
    intrinsics.push_back(static_cast<float>(height)); // Image height
    intrinsics.push_back(static_cast<float>(reprojectionError)); // Reprojection error
}