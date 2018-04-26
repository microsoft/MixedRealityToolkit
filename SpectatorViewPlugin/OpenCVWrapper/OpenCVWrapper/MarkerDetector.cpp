#include "pch.h"
#include "MarkerDetector.h"
#include <opencv2\highgui.hpp>
#include <opencv2\aruco.hpp>

MarkerDetector::MarkerDetector(int _markerDictionaryId)
{
	m_dictionaryId = _markerDictionaryId;
}

MarkerDetector::~MarkerDetector()
{
	m_markerCorners.clear();
	m_detectedMarkers.clear();
	m_rotationVecs.clear();
	m_translationVecs.clear();
}

void MarkerDetector::Detect(int _imageWidth, int _imageHeight, unsigned char* _imageData, float _markerSize)
{
	// As unity defines textures bottom to top we need to flip our data here
	std::vector<unsigned char> flippedData(_imageWidth * _imageHeight * 3);
	for (int i = 0; i < _imageHeight; i++)
	{
		memcpy(&flippedData[(i * _imageWidth * 3)], _imageData + ((_imageHeight - 1 - i) * _imageWidth * 3), sizeof(unsigned char)*_imageWidth * 3);
	}

	cv::Mat inputImage(_imageHeight, _imageWidth, CV_8UC3, &flippedData[0]);

	std::vector<int> markerIds;
	std::vector<std::vector<cv::Point2f>> rejectedCandidates;
	cv::Ptr<cv::aruco::DetectorParameters> parameters = cv::aruco::DetectorParameters::create();
	cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::PREDEFINED_DICTIONARY_NAME(m_dictionaryId));

	cv::aruco::detectMarkers(inputImage, dictionary, m_markerCorners, m_detectedMarkers, parameters, rejectedCandidates);

	if (m_markerCorners.size() <= 0)
	{
		return;
	}

	cv::Mat cameraMatrix(3, 3, CV_64F);
	cameraMatrix.at<double>(0, 0) = 1006.224;
	cameraMatrix.at<double>(0, 1) = 0.0;
	cameraMatrix.at<double>(0, 2) = 448;
	cameraMatrix.at<double>(1, 0) = 0.0;
	cameraMatrix.at<double>(1, 1) = 1006.224;
	cameraMatrix.at<double>(1, 2) = 252;
	cameraMatrix.at<double>(2, 0) = 0.0;
	cameraMatrix.at<double>(2, 1) = 0.0;
	cameraMatrix.at<double>(2, 2) = 1.0;

	cv::Mat distortionCoefficientsMatrix(1, 5, CV_64F);
	distortionCoefficientsMatrix.at<double>(0, 0) = -0.005678121;
	distortionCoefficientsMatrix.at<double>(0, 1) = -1.156654;
	distortionCoefficientsMatrix.at<double>(0, 2) = 0.0052870559368031209;
	distortionCoefficientsMatrix.at<double>(0, 3) = 0.016626680853497420;
	distortionCoefficientsMatrix.at<double>(0, 4) = -0.0000021207464374725242;

	cv::aruco::estimatePoseSingleMarkers(m_markerCorners, _markerSize, cameraMatrix, distortionCoefficientsMatrix, m_rotationVecs, m_translationVecs);
}

bool MarkerDetector::GetDetectedMarkerIds(unsigned int* _detectedMarkerIds)
{
	if (m_detectedMarkers.size() <= 0)
	{
		return false;
	}

	memcpy(_detectedMarkerIds, &m_detectedMarkers[0], sizeof(m_detectedMarkers[0])*m_detectedMarkers.size());

	return true;
}

bool MarkerDetector::GetDetectedMarkerPose(int _markerId, float& _xPos, float& _yPos, float& _zPos, float& _xRot, float& _yRot, float& _zRot)
{
	if (m_detectedMarkers.size() <= 0)
	{
		return false;
	}

	for (int i = 0; i < m_detectedMarkers.size(); i++)
	{
		if (m_detectedMarkers[i] == _markerId)
		{
			_xPos = m_translationVecs[i][0];
			_yPos = m_translationVecs[i][1];
			_zPos = m_translationVecs[i][2];

			_xRot = m_rotationVecs[i][0];
			_yRot = m_rotationVecs[i][1];
			_zRot = m_rotationVecs[i][2];

			return true;;
		}
	}

	return false;
}
