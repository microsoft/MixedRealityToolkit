// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include "stdafx.h"
#include "ImageTagLocator.h"
#include "ImageTagLocationImpl.h"

XTOOLS_NAMESPACE_BEGIN

ImageTagLocator::ImageTagLocator(const byte* data, int32 pixelWidth, int32 pixelHeight, int32 bytesPerPixel, ImageTagLocationListener* locationCallback)
: m_imageData(data, data + (pixelWidth * pixelHeight * bytesPerPixel))
, m_imageWidth(pixelWidth)
, m_imageHeight(pixelHeight)
, m_bytesPerPixel(bytesPerPixel)
, m_callbackList(ListenerList::Create())
, m_outputQueue(10 * sizeof(ImageTagLocationPtr))
, m_isComplete(false)
, m_isStopping(0)
{
	m_callbackList->AddListener(locationCallback);
}

ImageTagLocator::~ImageTagLocator()
{
	Cancel();
}

void ImageTagLocator::Update()
{
	ImageTagLocationPtr location;
	while (m_outputQueue.TryPop(location))
	{
		if (location == nullptr)
		{
			m_isComplete = true;
			m_callbackList->NotifyListeners(&ImageTagLocationListener::OnTagLocatingCompleted);
		}
		else
		{
			m_callbackList->NotifyListeners(&ImageTagLocationListener::OnTagLocated, location);
		}
	}
}

void ImageTagLocator::Cancel()
{
	m_isStopping = 1;
}

void ImageTagLocator::FindTags(apriltag_detector_t* td)
{
	image_u8* im = CreateImage();
	zarray_t *detections = apriltag_detector_detect(td, im);

	for (int i = 0; i < zarray_size(detections); i++) {
		apriltag_detection_t *det;
		zarray_get(detections, i, &det);

		ImageTagLocationPtr newTagLocator = new ImageTagLocationImpl(det);

		for (; !m_outputQueue.TryPush(newTagLocator) && !m_isStopping;) {}
	}

	zarray_destroy(detections);
	image_u8_destroy(im);

	for (; !m_outputQueue.TryPush(nullptr) && !m_isStopping;) {}
}

image_u8* ImageTagLocator::CreateImage() const
{
	image_u8* im = image_u8_create(m_imageWidth, m_imageHeight);
	for (int32 y = 0; y < m_imageHeight; y++)
	{
		for (int32 x = 0; x < m_imageWidth; x++)
		{
			im->buf[y * im->stride + x] = GetMonochromePixel(x, y);
		}
	}

	return im;
}

byte ImageTagLocator::GetMonochromePixel(int32 x, int32 y) const
{
	const int32 offset = y * m_imageWidth * m_bytesPerPixel + x * m_bytesPerPixel;
	switch (m_bytesPerPixel)
	{
	case 1:
	case 2:
		// 1-byte monochrome or 2-byte monochrome + alpha
		return m_imageData[offset];
	case 3:
	case 4:
		// 3 bytes RGB/BGR or 4 bytes BGRA/RGBA: perform simple grayscaling of the RGB data
		return (m_imageData[offset] + m_imageData[offset + 1] + m_imageData[offset + 2]) / 3;
	default:
		XTASSERT(false);
		return 0;
	}
}

XTOOLS_NAMESPACE_END
