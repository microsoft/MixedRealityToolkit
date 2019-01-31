#include "pch.h"
#include "MarkerDetector.h"

#include <opencv2\aruco.hpp>

MarkerDetector::MarkerDetector() {}

MarkerDetector::~MarkerDetector() {}

template <class T>
void OutputDebugMatrix(const std::wstring prompt, const cv::Mat mat)
{
    auto output = prompt;
    for (int m = 0; m < mat.rows; m++)
    {
        for (int n = 0; n < mat.cols; n++)
        {
            auto value = mat.at<T>(m, n);
            output += (L", " + std::to_wstring(value));
        }
    }

    OutputDebugString(output.data());
}

bool MarkerDetector::DetectMarkers(
    unsigned char* imageData,
    int imageWidth,
    int imageHeight,
    float* projectionMatrix,
    float markerSize,
    int arUcoDictionaryId)
{
    // Unity's RGB24 Texture will have 3 unsigned chars per pixel which aligns with opencv's CV_8UC3 mat format
    cv::Mat rawImage(imageHeight, imageWidth, CV_8UC3, imageData);

    // Unity defines textures bottom to top, but opencv defines matrices top to bottom
    // So, we flip our imageData vertically for processing
    cv::Mat image;
    cv::flip(rawImage, image, 0); // flipCode == 0 results in a vertical flip

    std::vector<int> arUcoMarkerIds;
    std::vector<std::vector<cv::Point2f>> arUcoMarkers;
    std::vector<std::vector<cv::Point2f>> arUcoRejectedCandidates;
    auto arUcoDetectorParameters = cv::aruco::DetectorParameters::create();
    auto arUcoDictionary = cv::aruco::getPredefinedDictionary(cv::aruco::PREDEFINED_DICTIONARY_NAME(arUcoDictionaryId));

    // Detect markers
    cv::aruco::detectMarkers(
        image,
        arUcoDictionary,
        arUcoMarkers,
        arUcoMarkerIds,
        arUcoDetectorParameters,
        arUcoRejectedCandidates);

    auto logText = L"Completed marker detection: " + std::to_wstring(arUcoMarkerIds.size()) + L" ids found";
    OutputDebugString(logText.data());

    cv::Mat cameraMatrix(3, 3, CV_64F, cv::Scalar(0));
    cameraMatrix.at<double>(0, 0) = projectionMatrix[0]; // 0,0 in the unity projection matrix equates to the x focal length
    cameraMatrix.at<double>(0, 2) = projectionMatrix[2]; // 0,2 in the unity projection matrix equates to the x principal point
    cameraMatrix.at<double>(1, 1) = projectionMatrix[5]; // 1,1 in the unity projection matrix equates to the y focal length
    cameraMatrix.at<double>(1, 2) = projectionMatrix[6]; // 1,2 in the unity projection matrix equates to the y principal point
    cameraMatrix.at<double>(2, 2) = 1.0;
    OutputDebugMatrix<double>(L"Camera Matrix: ", cameraMatrix);

    cv::Mat distCoefficientsMatrix(1, 5, CV_64F, cv::Scalar(0));
    OutputDebugMatrix<double>(L"Distortion Coefficients: ", distCoefficientsMatrix);

    std::vector<cv::Vec3d> rotationVecs;
    std::vector<cv::Vec3d> translationVecs;
    cv::aruco::estimatePoseSingleMarkers(
        arUcoMarkers,
        markerSize,
        cameraMatrix,
        distCoefficientsMatrix,
        rotationVecs,
        translationVecs);

    for (size_t i = 0; i < arUcoMarkerIds.size(); i++)
    {
        auto id = arUcoMarkerIds[i];

        auto posText = L"OpenCV Marker Position: " + std::to_wstring(translationVecs[i][0]) + L", " + std::to_wstring(translationVecs[i][1]) + L", " + std::to_wstring(translationVecs[i][2]);
        OutputDebugString(posText.data());

        auto rotText = L"OpenCV Marker Rotation: " + std::to_wstring(rotationVecs[i][0]) + L", " + std::to_wstring(rotationVecs[i][1]) + L", " + std::to_wstring(rotationVecs[i][2]);
        OutputDebugString(rotText.data());

        Marker marker;
        marker.Id = id;
        marker.position[0] = static_cast<float>(translationVecs[i][0]);
        marker.position[1] = static_cast<float>(translationVecs[i][1]);
        marker.position[2] = static_cast<float>(translationVecs[i][2]);
        marker.rotation[0] = static_cast<float>(rotationVecs[i][0]);
        marker.rotation[1] = static_cast<float>(rotationVecs[i][1]);
        marker.rotation[2] = static_cast<float>(rotationVecs[i][2]);
        _detectedMarkers[id] = marker;
    }

    return true;
}

bool MarkerDetector::GetDetectedMarkerIds(int* _detectedIds, int size)
{
    if (_detectedMarkers.size() > static_cast<size_t>(size))
    {
        return false;
    }

    int index = 0;
    for (auto markerPair : _detectedMarkers)
    {
        markerPair.second.Id;
        _detectedIds[index] = markerPair.second.Id;
        index++;
    }

    return true;
}

bool MarkerDetector::GetDetectedMarkerPose(int _detectedId, float* position, float* rotation)
{
    if (_detectedMarkers.find(_detectedId) == _detectedMarkers.end())
    {
        return false;
    }

    auto marker = _detectedMarkers.at(_detectedId);
    memcpy(position, marker.position, sizeof(marker.position));
    memcpy(rotation, marker.rotation, sizeof(marker.rotation));

    return true;
}