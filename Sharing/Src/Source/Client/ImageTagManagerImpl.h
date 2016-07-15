// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once

#include "ImageTagLocator.h"

XTOOLS_NAMESPACE_BEGIN

class ImageTagManagerImpl : public ImageTagManager
{
public:
	ImageTagManagerImpl();
	virtual ~ImageTagManagerImpl();

	virtual void Update() XTOVERRIDE;
	virtual bool FindTags(const byte* data, int32 pixelWidth, int32 pixelHeight, int32 bytesPerPixel, ImageTagLocationListener* locationCallback) XTOVERRIDE;
	virtual TagImagePtr CreateTagImage(int32 tagId) const XTOVERRIDE;

private:
	void FindTagsWorker();

	MemberFuncThreadPtr m_workerThread;
	Event m_event;
	volatile int m_stopping;
	ImageTagLocatorPtr m_locator;
	apriltag_family_t* m_tf;
};

XTOOLS_NAMESPACE_END
