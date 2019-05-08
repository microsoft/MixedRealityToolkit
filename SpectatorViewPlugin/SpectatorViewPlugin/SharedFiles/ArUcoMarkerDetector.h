#pragma once
#include "pch.h"

struct Marker
{
    int id;
    float position[3]; // vector
    float rotation[3]; // rodrigues vector
};

class ArUcoMarkerDetector
{
public:
    ArUcoMarkerDetector();
    ~ArUcoMarkerDetector();
    bool DetectMarkers(
        unsigned char* imageData,
        int imageWidth,
        int imageHeight,
        float* focalLength,
        float* principalPoint,
        float* radialDistortion,
        float* tangentialDistortion,
        float markerSize,
        int arUcoMarkerDictionaryId);
    inline int GetDetectedMarkersCount() { return static_cast<int>(_detectedMarkers.size()); }
    bool GetDetectedMarkerIds(int* _detectedIds, int size);
    bool GetDetectedMarkerPose(int _detectedId, float* position, float* rotation);

private:
    std::unordered_map<int, Marker> _detectedMarkers;
};
