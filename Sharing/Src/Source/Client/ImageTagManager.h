// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// ImageTagManager.h
// Class for locating AprilTag AR tags within an image.
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "ImageTagLocationListener.h"
#include "TagImage.h"

XTOOLS_NAMESPACE_BEGIN

DECLARE_PTR_PRE(ImageTagManager)

class ImageTagManager : public AtomicRefCounted
{
public:
	virtual ~ImageTagManager() {}

	// Called to continue processing image tag finding.  Callbacks for asynchronous tag
	// searches will not be called unless Update is called on each frame.
	virtual void Update() = 0;

	// Asynchronously finds tags within an image.  The location callback is invoked during update
	// to notify the caller of results.
	virtual bool FindTags(const byte* data, int32 pixelWidth, int32 pixelHeight, int32 bytesPerPixel, ImageTagLocationListener* locationCallback) = 0;

	// Create a tag image for a given tag.
	virtual TagImagePtr CreateTagImage(int32 tagId) const = 0;

	// Creates a new ImageTagManager instance.
	static ref_ptr<ImageTagManager> Create();
};

DECLARE_PTR_POST(ImageTagManager)

XTOOLS_NAMESPACE_END
