#include "pch.h"
#include "SpectatorViewPlugin.h"
#include <opencv2\aruco.hpp>
#include <opencv2\core.hpp>
#include <opencv2\highgui.hpp>
#include <opencv2\aruco\charuco.hpp>
#include <ctime>
#include "MarkerDetector.h"

MarkerDetector* detector = NULL;

/// Void function to test whether Unity has been able to successful load the dll 
extern "C" __declspec(dllexport) void __stdcall CheckLibraryHasLoaded()
{
	return;
}

extern "C" __declspec(dllexport) void __stdcall MarkerDetector_Initialize()
{
	if (detector == NULL)
	{
		detector = new MarkerDetector(10);
	}
}

extern "C" __declspec(dllexport) void __stdcall MarkerDetector_Terminate()
{
	if (detector != NULL)
	{
		delete detector;
	}
}

/// Looks for markers in an image
/// _imageWidth : width of the image data
/// _imageHeight: height of the image data
/// _imageData : input image data
/// _markerSize : The physical size of the target marker in meters
extern "C" __declspec(dllexport) bool __stdcall MarkerDetector_DetectMarkers(int _imageWidth, int _imageHeight, unsigned char* _imageData, float _markerSize)
{
	if (detector == NULL)
	{
		return false;
	}

	detector->Detect(_imageWidth, _imageHeight, _imageData, _markerSize);

	return true;
}

/// Returns the number of marker detected after MarkerDetector_DetectMarkers has been called 
/// _numMarkersDetected : the number of markers detected
extern "C" __declspec(dllexport) bool __stdcall MarkerDetector_GetNumMarkersDetected(int& _numMarkersDetected)
{
	if (detector == NULL)
	{
		return false;
	}

	_numMarkersDetected = detector->GetNumDetectedMarkers();

	return true;
}

/// Get an array of detected marker ids after MarkerDetector_DetectMarkers has been called 
/// _detectedMarkers : array of detected marker ids
extern "C" __declspec(dllexport) bool __stdcall MarkerDetector_GetDetectedMarkerIds(unsigned int* _detectedMarkers)
{
	if (detector == NULL)
	{
		return false;
	}

	detector->GetDetectedMarkerIds(_detectedMarkers);

	return true;
}

/// Gets the marker position and rotation in camera space for a given marker
/// _markerId : the requested marker id
/// _xPos : x position
/// _yPos : y position
/// _zPos : z position
/// _xRot : x rotation
/// _yRot : y rotatiom
/// _zRot : z rotation
extern "C" __declspec(dllexport) bool __stdcall MarkerDetector_GetDetectedMarkerPose(int _markerId, float& _xPos, float& _yPos, float& _zPos, float& _xRot, float& _yRot, float& _zRot)
{
	if (detector == NULL)
	{
		return false;
	}

	detector->GetDetectedMarkerPose(_markerId, _xPos, _yPos, _zPos, _xRot, _yRot, _zRot);

	return true;
}
