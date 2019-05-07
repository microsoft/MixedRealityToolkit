#include "Calibration.h"

const int intrinsicsSize = 12;
const int extrinsicsSize = 7;

bool Calibration::Initialize()
{
    worldPointObservations.clear();
    imagePointObservations.clear();
    pointObservationsRelativeToCamera.clear();
    width = 0;
    height = 0;
    chessboardImageWidth = 0;
    chessboardImageHeight = 0;
    chessboardWidth = 0;
    chessboardHeight = 0;
    chessboardImagePointObservations.clear();
    return true;
}

bool Calibration::ProcessChessboardImage(
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
    if ((chessboardImageWidth != imageWidth && chessboardImageWidth != 0) ||
        (chessboardImageHeight != imageHeight && chessboardImageHeight != 0) ||
        (chessboardWidth != boardWidth && chessboardWidth != 0) ||
        (chessboardHeight != boardHeight && chessboardHeight != 0))
    {
        lastError = "Calibration does not support different image dimensions/different chessboard dimensions";
        throw std::exception();
        return false;
    }

    chessboardImageWidth = imageWidth;
    chessboardImageHeight = imageHeight;
    chessboardWidth = boardWidth;
    chessboardHeight = boardHeight;

    cv::Mat rgbImage(chessboardImageHeight, chessboardImageWidth, CV_8UC3, image);

    // Convert the image to grayscale for processing
    cv::Mat upsideDownGrayImage;
    cv::cvtColor(rgbImage, upsideDownGrayImage, CV_RGB2GRAY);

    // The obtained image data needs to be flipped vertically for processing
    cv::Mat grayImage;
    cv::flip(upsideDownGrayImage, grayImage, 0);

    std::vector<cv::Point2f> chessboardImagePoints;
    cv::Size patternSize = cv::Size(boardWidth - 1, boardHeight - 1);
    bool locatedBoard = cv::findChessboardCorners(
        grayImage,
        patternSize,
        chessboardImagePoints,
        cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_NORMALIZE_IMAGE + cv::CALIB_CB_FAST_CHECK);

    if (locatedBoard)
    {
        chessboardImagePointObservations.push_back(chessboardImagePoints);

        cv::Mat rgbImageHelper;
        cv::flip(rgbImage, rgbImageHelper, 0);
        cv::drawChessboardCorners(rgbImageHelper, patternSize, chessboardImagePoints, true);
        cv::flip(rgbImageHelper, rgbImage, 0);

        cv::Mat cornersHelper;
        cv::Mat corners(chessboardImageHeight, chessboardImageWidth, CV_8UC3, cornersImage);
        cv::flip(corners, cornersHelper, 0);
        for (auto corner : chessboardImagePoints)
        {
            cv::circle(cornersHelper, corner, cornerImageRadias, cv::Scalar(255, 255, 255), CV_FILLED);
        }
        cv::flip(cornersHelper, corners, 0);

        cv::Mat heatmap(chessboardImageHeight, chessboardImageWidth, CV_8UC3, heatmapImage);
        cv::Mat heatmapHelper;
        cv::flip(heatmap, heatmapHelper, 0);

        int heatmapHeight = static_cast<int>(heatmapWidth * heatmap.rows / static_cast<double>(heatmap.cols));

        cv::Mat currHeatmap(heatmapHeight, heatmapWidth, CV_8U, cv::Scalar(0));
        for (auto corner : chessboardImagePoints)
        {
            int x = static_cast<int>(corner.x / static_cast<double>(heatmap.cols)  * heatmapWidth);
            x = (x > heatmapWidth - 1) ? heatmapWidth - 1 : x;
            int y = static_cast<int>(corner.y / static_cast<double>(heatmap.rows)  * heatmapHeight);
            y = (y > heatmapHeight - 1) ? heatmapHeight - 1 : y;
            cv::Point2i loc = cv::Point2i(x, y);

            currHeatmap.at<unsigned char>(loc)++;
        }

        for (int i = 0; i < heatmapHelper.rows; i++)
        {
            for (int j = 0; j < heatmapHelper.cols; j++)
            {
                int x = static_cast<int>(j / static_cast<double>(heatmap.cols) * heatmapWidth);
                x = (x > heatmapWidth - 1) ? heatmapWidth - 1 : x;
                int y = static_cast<int>(i / static_cast<double>(heatmap.rows) * heatmapHeight);
                y = (y > heatmapHeight - 1) ? heatmapHeight - 1 : y;
                cv::Point2i loc = cv::Point2i(x, y);

                int value = static_cast<int>(heatmapHelper.at<cv::Vec3b>(i, j)[0]) + currHeatmap.at<unsigned char>(loc);
                value = (value > 255) ? 255 : value;

                heatmapHelper.at<cv::Vec3b>(i, j)[0] = static_cast<unsigned char>(value);
                heatmapHelper.at<cv::Vec3b>(i, j)[1] = static_cast<unsigned char>(value);
                heatmapHelper.at<cv::Vec3b>(i, j)[2] = static_cast<unsigned char>(value);
            }
        }

        cv::flip(heatmapHelper, heatmap, 0);
        memcpy(heatmapImage, heatmap.data, heatmap.cols * heatmap.rows * 3 * sizeof(unsigned char));
    }

    return locatedBoard;
}

bool Calibration::ProcessChessboardIntrinsics(
    float squareSize,
    float* intrinsics,
    int sizeIntrinsics)
{
    if (sizeIntrinsics != intrinsicsSize)
    {
        lastError = "Intrinsics should have " + std::to_string(intrinsicsSize) + " float values";
        return false;
    }

    std::vector<cv::Point3f> boardDimensions;
    for (int i = 0; i < chessboardHeight - 1; i++)
    {
        for (int j = 0; j < chessboardWidth - 1; j++)
        {
            boardDimensions.push_back(
                cv::Point3f(
                (float)(j * squareSize),
                    (float)(i * squareSize),
                    0.0f));
        }
    }

    std::vector<std::vector<cv::Point3f>> boardPointObservations;
    for (size_t i = 0; i < chessboardImagePointObservations.size(); i++)
    {
        boardPointObservations.push_back(boardDimensions);
    }

    cv::Mat cameraMat = cv::initCameraMatrix2D(
        boardPointObservations,
        chessboardImagePointObservations,
        cv::Size(chessboardImageWidth, chessboardImageHeight),
        chessboardImageWidth / static_cast<double>(chessboardImageHeight));
    cv::Mat distCoeff, rvec, tvec;
    double reprojectionError = cv::calibrateCamera(
        boardPointObservations,
        chessboardImagePointObservations,
        cv::Size(chessboardImageWidth, chessboardImageHeight),
        cameraMat,
        distCoeff,
        rvec,
        tvec,
        CV_CALIB_USE_INTRINSIC_GUESS);

    StoreIntrinsics(reprojectionError, cameraMat, distCoeff, chessboardImageWidth, chessboardImageHeight, intrinsics);
    return true;
}

bool Calibration::ProcessArUcoData(
    unsigned char* image,
    int imageWidth,
    int imageHeight,
    int* markerIds,
    int numMarkers,
    float* markerCornersInWorld,
    float* markerCornersRelativeToCamera,
    int numMarkerCornerValues)
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

    std::unordered_map<int, corners> cornerRelativeToCameraMap;
    for (int i = 0; i < numMarkers; i++)
    {
        auto markerId = markerIds[i];
        corners currCorners{
            cv::Point3f{
                markerCornersRelativeToCamera[floatsPerMarker * i],
                markerCornersRelativeToCamera[floatsPerMarker * i + 1],
                markerCornersRelativeToCamera[floatsPerMarker * i + 2]
            },
            cv::Point3f{
                markerCornersRelativeToCamera[floatsPerMarker * i + 3],
                markerCornersRelativeToCamera[floatsPerMarker * i + 4],
                markerCornersRelativeToCamera[floatsPerMarker * i + 5]
            },
            cv::Point3f{
                markerCornersRelativeToCamera[floatsPerMarker * i + 6],
                markerCornersRelativeToCamera[floatsPerMarker * i + 7],
                markerCornersRelativeToCamera[floatsPerMarker * i + 8]
            },
            cv::Point3f{
                markerCornersRelativeToCamera[floatsPerMarker * i + 9],
                markerCornersRelativeToCamera[floatsPerMarker * i + 10],
                markerCornersRelativeToCamera[floatsPerMarker * i + 11]
            },
        };
        cornerRelativeToCameraMap[markerId] = currCorners;
    }

    cv::Mat rgbImage(height, width, CV_8UC3, image);

    // Convert the image to grayscale for processing
    cv::Mat upsideDownGrayImage;
    cv::cvtColor(rgbImage, upsideDownGrayImage, CV_RGB2GRAY);

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
    std::vector<cv::Point3f> pointsRelativeToCamera;
    std::vector<cv::Point3f> planarPoints;
    for (size_t i = 0; i < arUcoMarkerIds.size(); i++)
    {
        auto id = arUcoMarkerIds.at(i);
        if (worldCornersMap.find(id) != worldCornersMap.end() &&
            cornerRelativeToCameraMap.find(id) != cornerRelativeToCameraMap.end())
        {
            auto worldCorners = worldCornersMap.at(id);
            auto cornersRelativeToCamera = cornerRelativeToCameraMap.at(id);
            auto imageCorners = arUcoMarkerCorners.at(i);

            // OpenCV reports ArUco marker corners in a clockwise manner
            worldPoints.push_back(worldCorners.topLeft);
            worldPoints.push_back(worldCorners.topRight);
            worldPoints.push_back(worldCorners.bottomRight);
            worldPoints.push_back(worldCorners.bottomLeft);

            pointsRelativeToCamera.push_back(cornersRelativeToCamera.topLeft);
            pointsRelativeToCamera.push_back(cornersRelativeToCamera.topRight);
            pointsRelativeToCamera.push_back(cornersRelativeToCamera.bottomRight);
            pointsRelativeToCamera.push_back(cornersRelativeToCamera.bottomLeft);

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

    cv::Mat rgbImageHelper;
    cv::flip(rgbImage, rgbImageHelper, 0);
    for (auto point : imagePoints)
    {
        cv::circle(rgbImageHelper, point, 3, cv::Scalar(0, 255, 0), CV_FILLED);
    }
    cv::flip(rgbImageHelper, rgbImage, 0);

    if (worldPoints.size() > 0 &&
        imagePoints.size() > 0)
    {
        worldPointObservations.push_back(worldPoints);
        imagePointObservations.push_back(imagePoints);
        pointObservationsRelativeToCamera.push_back(pointsRelativeToCamera);
    }
    else
    {
        lastError = "Unable to find any markers in provided image";
        return false;
    }

    return true;
}

bool Calibration::ProcessIndividualArUcoExtrinsics(
    float* intrinsics,
    float* extrinsics,
    int sizeExtrinsics,
    int numExtrinsics)
{
    if (sizeExtrinsics != extrinsicsSize)
    {
        lastError = "Extrinsics should have " + std::to_string(extrinsicsSize) + " float values";
        return false;
    }

    if (numExtrinsics < static_cast<int>(worldPointObservations.size()))
    {
        lastError = "Provide at least as many extrinsics as images that were processed";
        return false;
    }

    if (worldPointObservations.size() < 1 ||
        imagePointObservations.size() < 1)
    {
        lastError = "Image processing must succeed before conducting calibration";
        return false;
    }

    for (size_t i = 0; i < worldPointObservations.size() && i < imagePointObservations.size(); i++)
    {
        {
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
                worldPointObservations[i],
                imagePointObservations[i],
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
                &(extrinsics[i * extrinsicsSize]));
        }
    }

    return true;
}

bool Calibration::ProcessGlobalArUcoExtrinsics(
    float* intrinsics,
    float* extrinsics,
    int sizeExtrinsics)
{
    if (sizeExtrinsics != extrinsicsSize)
    {
        lastError = "Extrinsics should have " + std::to_string(extrinsicsSize) + " float values";
        return false;
    }

    if (pointObservationsRelativeToCamera.size() < 1 ||
        imagePointObservations.size() < 1)
    {
        lastError = "Image processing must succeed before conducting calibration";
        return false;
    }

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

    std::vector<cv::Point3f> allPointsRelativeToCamera;
    for (auto points : pointObservationsRelativeToCamera)
    {
        for (auto point : points)
        {
            allPointsRelativeToCamera.push_back(point);
        }
    }

    std::vector<cv::Point2f> allPointsInImage;
    for (auto points : imagePointObservations)
    {
        for (auto point : points)
        {
            allPointsInImage.push_back(point);
        }
    }

    cv::Mat rvecsMatIterative, tvecsMatIterative;
    auto iterativeError = cv::solvePnP(
        allPointsRelativeToCamera,
        allPointsInImage,
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
        extrinsics);

    return true;
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

void Calibration::StoreIntrinsics(
    double reprojectionError,
    cv::Mat cameraMat,
    cv::Mat distCoeff,
    int width,
    int height,
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
    float succeeded,
    cv::Mat rvec,
    cv::Mat tvec,
    float* extrinsics)
{
    extrinsics[0] = static_cast<float>(succeeded);
    extrinsics[1] = static_cast<float>(tvec.at<double>(0, 0));
    extrinsics[2] = static_cast<float>(tvec.at<double>(0, 1));
    extrinsics[3] = static_cast<float>(tvec.at<double>(0, 2));
    extrinsics[4] = static_cast<float>(rvec.at<double>(0, 0));
    extrinsics[5] = static_cast<float>(rvec.at<double>(0, 1));
    extrinsics[6] = static_cast<float>(rvec.at<double>(0, 2));
}