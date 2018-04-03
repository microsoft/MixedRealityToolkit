#pragma once
#include<vector>
#include <opencv2\core.hpp>

class MarkerDetector
{
public:
	MarkerDetector(int _markerDictionaryId);
	~MarkerDetector();
	void Detect(int _imageWidth, int _imageHeight, unsigned char* _imageData, float _markerSize);
	
	inline int GetNumDetectedMarkers() { return m_detectedMarkers.size(); }
	bool GetDetectedMarkerIds(unsigned int* _detectedMarkerIds);
	bool GetDetectedMarkerPose(int _markerId, float& _xPos, float& _yPos, float& _zPos, float& _xRot, float& _yRot, float& _zRot);
private:
	std::vector<std::vector<cv::Point2f>> m_markerCorners;
	std::vector<cv::Vec3d> m_rotationVecs;
	std::vector<cv::Vec3d> m_translationVecs;
	std::vector<int> m_detectedMarkers;
	int m_dictionaryId;
};
