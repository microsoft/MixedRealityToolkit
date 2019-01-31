#pragma once
#include<unordered_map>
#include <opencv2\core.hpp>

struct Marker
{
    int Id;
    float position[3];
    float rotation[3];
};

class MarkerDetector
{
public:
    MarkerDetector();
    ~MarkerDetector();
    bool DetectMarkers(
        unsigned char* imageData,
        int imageWidth,
        int imageHeight,
        float* projectionMatrix,
        float markerSize,
        int arUcoDictionaryId);
    inline int GetDetectedMarkersCount() { return _detectedMarkers.size(); }
    bool GetDetectedMarkerIds(int* _detectedIds, int size);
    bool GetDetectedMarkerPose(int _detectedId, float* position, float* rotation);

private:
    std::unordered_map<int, Marker> _detectedMarkers;
};
