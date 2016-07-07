// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// ImageTagLocation.h
// Stores information about the 2D location of an AR tag within an image.
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

/// Represents a location within an indivdual tag (either one of the four corners of the
/// tag or the center of the tag).  This enumeration can be used to get the full 2D position
/// of an individual tag within an image.
enum ImageTagLocationType
{
	TopLeft,
	TopRight,
	BottomLeft,
	BottomRight,
	Center
};

class ImageTagLocation : public AtomicRefCounted
{
public:
	virtual ~ImageTagLocation() {}

	// Gets the unique tag ID associated with the tag.
	virtual int GetTagId() = 0;

	// Gets the pixel X location of a corner or center of the tag.
	virtual float GetPixelX(ImageTagLocationType locationType) = 0;

	// Gets the pixel Y location of a corner or center of the tag.
	virtual float GetPixelY(ImageTagLocationType locationType) = 0;

	// Gets the model-view matrix from the tag given a camera perspective matrix.
	virtual float GetModelViewMatrix(int32 row, int32 col, float F, float G, float A, float B, float C, float D) = 0;
};

DECLARE_PTR(ImageTagLocation)

XTOOLS_NAMESPACE_END
