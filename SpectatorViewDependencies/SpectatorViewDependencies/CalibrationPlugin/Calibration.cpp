#include "stdafx.h"
#include "Calibration.h"

bool Calibration::Initialize()
{
    worldPointObservations.clear();
    imagePointObservations.clear();
    return true;
}

bool Calibration::ProcessImage(
    unsigned char* image,
    int imageWidth,
    int imageHeight,
    int* markerIds,
    int numMarkers,
    float* markerCornersInWorld,
    int numMarkerCornerValues,
    float* orientation)
{
    if (numMarkerCornerValues != numMarkers * 4 * 3)
    {
        lastError = "The number of total corners did not equal what was expected";
        throw std::exception();
        return false;
    }

    if ((width != imageWidth && width != 0) ||
        (height != imageHeight && height != 0))
    {
        lastError = "Calibration does not support different image dimensions";
        throw std::exception();
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
            cv::Point3f{
                markerCornersInWorld[floatsPerMarker * i],
                markerCornersInWorld[floatsPerMarker * i + 1],
                markerCornersInWorld[floatsPerMarker * i + 2]
            },
            cv::Point3f{
                markerCornersInWorld[floatsPerMarker * i + 3],
                markerCornersInWorld[floatsPerMarker * i + 4],
                markerCornersInWorld[floatsPerMarker * i + 5]
            },
            cv::Point3f{
                markerCornersInWorld[floatsPerMarker * i + 6],
                markerCornersInWorld[floatsPerMarker * i + 7],
                markerCornersInWorld[floatsPerMarker * i + 8]
            },
            cv::Point3f{
                markerCornersInWorld[floatsPerMarker * i + 9],
                markerCornersInWorld[floatsPerMarker * i + 10],
                markerCornersInWorld[floatsPerMarker * i + 11]
            },
        };
        worldCornersMap[markerId] = currCorners;
    }

    // Unity textures default to ARGB, so we convert to RGBA
    cv::Mat rgbaImage(height, width, CV_8UC4, image);
    for (int i = 0; i < rgbaImage.rows; i++)
    {
        for (int j = 0; j < rgbaImage.cols; j++)
        {
            unsigned char alpha = rgbaImage.at<cv::Vec4b>(i, j)[0];
            rgbaImage.at<cv::Vec4b>(i, j)[0] = rgbaImage.at<cv::Vec4b>(i, j)[3];
            rgbaImage.at<cv::Vec4b>(i, j)[3] = alpha;
        }
    }

    // ArUco detection with opencv does not support images with alpha channels
    // So, we convert the image to grayscale for processing
    cv::Mat upsideDownGrayImage;
    cv::cvtColor(rgbaImage, upsideDownGrayImage, CV_RGBA2GRAY);

    // The obtained image data needs to be flipped vertically for processing
    cv::Mat grayImage;
    cv::flip(upsideDownGrayImage, grayImage, 0);

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

    auto text = "Detected " + std::to_string(arUcoMarkerIds.size()) + " markers in the provided image";
    lastError = text.data();

    std::vector<cv::Point3f> worldPoints;
    std::vector<cv::Point2f> imagePoints;
    for (size_t i = 0; i < arUcoMarkerIds.size(); i++)
    {
        auto id = arUcoMarkerIds.at(i);
        if (worldCornersMap.find(id) != worldCornersMap.end())
        {
            auto worldCorners = worldCornersMap.at(id);
            auto imageCorners = arUcoMarkerCorners.at(i);

            // OpenCV reports ArUco marker corners in a clockwise manner
            worldPoints.push_back(worldCorners.topLeft);
            worldPoints.push_back(worldCorners.topRight);
            worldPoints.push_back(worldCorners.bottomRight);
            worldPoints.push_back(worldCorners.bottomLeft);

            imagePoints.push_back(imageCorners.at(0));
            imagePoints.push_back(imageCorners.at(1));
            imagePoints.push_back(imageCorners.at(2));
            imagePoints.push_back(imageCorners.at(3));
        }
        else
        {
            text = "Marker " + std::to_string(arUcoMarkerIds.size()) + " was detected in image but did not have known world coordinates";
            lastError = text.data();
        }
    }

    if (worldPoints.size() > 0 &&
        imagePoints.size() > 0)
    {
        worldPointObservations.push_back(worldPoints);
        imagePointObservations.push_back(imagePoints);
    }
    else
    {
        lastError = "Unable to find any markers in provided image";
        return false;
    }

    return true;
}

bool Calibration::ProcessIntrinsics(float* intrinsics, int numIntrinsics)
{
    const size_t intrinsicSize = 12;
    if (numIntrinsics != 2)
    {
        lastError = "Calibration expects to return two intrinsics values";
        throw std::exception();
        return false;
    }

    if (worldPointObservations.size() < 1 ||
        imagePointObservations.size() < 1)
    {
        lastError = "Image processing must succeed before conducting calibration";
        return false;
    }

    // precalculated camera mat - 0.5 principal points, equal focal points
    cv::Mat cameraMat(3, 3, CV_64F, cv::Scalar(0));
    cv::Mat distCoeffMat(1, 5, CV_64F, cv::Scalar(0));
    cv::Mat rvecsMat, tvecsMat;
    cameraMat.at<double>(0, 0) = 1.0f* width; // X focal length
    cameraMat.at<double>(0, 2) = 0.5f * width; // X principal point
    cameraMat.at<double>(1, 1) = 1.0f * height; // Y focal length
    cameraMat.at<double>(1, 2) = 0.5f * height; // Y principal point
    cameraMat.at<double>(2, 2) = 1.0; // Default value for camera intrinsic matrix
    double reprojectionError = cv::calibrateCamera(
        worldPointObservations,
        imagePointObservations,
        cv::Size(width, height),
        cameraMat,
        distCoeffMat,
        rvecsMat,
        tvecsMat,
        CV_CALIB_USE_INTRINSIC_GUESS);
    StoreIntrinsics(reprojectionError, cameraMat, distCoeffMat, intrinsics);

    // TODO - standardized world points (updated based on camera position/orientation)
    // TODO - planar world points

    return true;
}

bool Calibration::ProcessExtrinsics(
    float* intrinsics,
    float* extrinsics,
    int numExtrinsics)
{
    if (numExtrinsics != 3)
    {
        lastError = "Three output extrinsics should be provided";
        return false;
    }

    if (worldPointObservations.size() < 1 ||
        imagePointObservations.size() < 1)
    {
        lastError = "Image processing must succeed before conducting calibration";
        return false;
    }

    const size_t extrinsicsSize = 7;
    cv::Mat cameraMat(3, 3, CV_64F, cv::Scalar(0));
    cv::Mat distCoeffMat(1, 5, CV_64F, cv::Scalar(0));

    cameraMat.at<double>(0, 0) = intrinsics[0]; // X Focal length
    cameraMat.at<double>(1, 1) = intrinsics[1]; // Y Focal length
    cameraMat.at<double>(0, 2) = intrinsics[2]; // X Principal point
    cameraMat.at<double>(1, 2) = intrinsics[3]; // Y Principal point
    cameraMat.at<double>(2, 2) = 1.0;
    distCoeffMat.at<double>(0, 0) = intrinsics[4]; // 1st Radial distortion coefficient
    distCoeffMat.at<double>(0, 1) = intrinsics[5]; // 2nd Radial distortion coefficient
    distCoeffMat.at<double>(0, 4) = intrinsics[6]; // 3rd Radial distortion coefficient
    distCoeffMat.at<double>(0, 2) = intrinsics[7]; // 1st Tangential distortion coefficient
    distCoeffMat.at<double>(0, 3) = intrinsics[8]; // 2nd Tangential distortion coefficient

    cv::Mat rvecsMatIterative, tvecsMatIterative;
    auto iterativeError = cv::solvePnP(
        worldPointObservations,
        imagePointObservations,
        cameraMat,
        distCoeffMat,
        rvecsMatIterative,
        tvecsMatIterative,
        false,
        CV_ITERATIVE);
    StoreExtrinsics(
        iterativeError,
        rvecsMatIterative,
        tvecsMatIterative,
        extrinsics,
        0);

    cv::Mat rvecsMatP3P, tvecsMatP3P;
    auto p3pError = cv::solvePnP(
        worldPointObservations,
        imagePointObservations,
        cameraMat,
        distCoeffMat,
        rvecsMatP3P,
        tvecsMatP3P,
        false,
        CV_P3P);
    StoreExtrinsics(
        p3pError,
        rvecsMatP3P,
        tvecsMatP3P,
        extrinsics,
        extrinsicsSize);

    cv::Mat rvecsMatEPNP, tvecsMatEPNP;
    auto EPNPError = cv::solvePnP(
        worldPointObservations,
        imagePointObservations,
        cameraMat,
        distCoeffMat,
        rvecsMatEPNP,
        tvecsMatEPNP,
        false,
        CV_EPNP);
    StoreExtrinsics(
        EPNPError,
        rvecsMatEPNP,
        tvecsMatEPNP,
        extrinsics,
        2 * extrinsicsSize);

    return true;
}

void Calibration::StoreIntrinsics(
    double reprojectionError,
    cv::Mat cameraMat,
    cv::Mat distCoeff,
    float* intrinsics)
{
    intrinsics[0] = static_cast<float>(cameraMat.at<double>(0, 0)); // X Focal length
    intrinsics[1] = static_cast<float>(cameraMat.at<double>(1, 1)); // Y Focal length
    intrinsics[2] = static_cast<float>(cameraMat.at<double>(0, 2)); // X Principal point
    intrinsics[3] = static_cast<float>(cameraMat.at<double>(1, 2)); // Y Principal point
    intrinsics[4] = static_cast<float>(distCoeff.at<double>(0, 0)); // 1st Radial distortion coefficient
    intrinsics[5] = static_cast<float>(distCoeff.at<double>(0, 1)); // 2nd Radial distortion coefficient
    intrinsics[6] = static_cast<float>(distCoeff.at<double>(0, 4)); // 3rd Radial distortion coefficient
    intrinsics[7] = static_cast<float>(distCoeff.at<double>(0, 2)); // 1st Tangential distortion coefficient
    intrinsics[8] = static_cast<float>(distCoeff.at<double>(0, 3)); // 2nd Tangential distortion coefficient
    intrinsics[9] = static_cast<float>(width); // Image width
    intrinsics[10] = static_cast<float>(height); // Image height
    intrinsics[11] = static_cast<float>(reprojectionError); // Reprojection error
}

void Calibration::StoreExtrinsics(
    double reprojectionError,
    cv::Mat rvec,
    cv::Mat tvec,
    float* extrinsics,
    int offset)
{
    extrinsics[offset] = static_cast<float>(reprojectionError);
    extrinsics[offset + 1] = static_cast<float>(tvec.at<double>(0, 0));
    extrinsics[offset + 2] = static_cast<float>(tvec.at<double>(0, 1));
    extrinsics[offset + 3] = static_cast<float>(tvec.at<double>(0, 2));
    extrinsics[offset + 4] = static_cast<float>(rvec.at<double>(0, 0));
    extrinsics[offset + 5] = static_cast<float>(rvec.at<double>(0, 1));
    extrinsics[offset + 6] = static_cast<float>(rvec.at<double>(0, 2));
}

bool Calibration::GetLastErrorMessage(
    char* buff,
    int size)
{
    if (!lastError.empty() &&
        size >= static_cast<int>(lastError.length()))
    {
        memcpy(buff, lastError.c_str(), lastError.length() * sizeof(char));
        return true;
    }

    return false;
}