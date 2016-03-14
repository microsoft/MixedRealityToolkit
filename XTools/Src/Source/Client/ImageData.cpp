//////////////////////////////////////////////////////////////////////////
// ImageData.cpp
//
// Simple container for image data.  Uses thread-safe reference counting
// so it can be passed around to different threads for processing
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ImageData.h"

XTOOLS_NAMESPACE_BEGIN

ImageData::ImageData(const byte* pixelData, int32 width, int32 height, int32 bytesPerPixel)
: m_pixelData(new byte[width * height * bytesPerPixel])
, m_imageWidth(width)
, m_imageHeight(height)
, m_bytesPerPixel(bytesPerPixel)
{
	memcpy(m_pixelData.get(), pixelData, width * height * bytesPerPixel);
}


const byte* ImageData::GetPixelData() const
{
	return m_pixelData.get();
}


int32 ImageData::GetWidth() const
{
	return m_imageWidth;
}


int32 ImageData::GetHeight() const
{
	return m_imageHeight;
}


int32 ImageData::GetBytesPerPixel() const
{
	return m_bytesPerPixel;
}

XTOOLS_NAMESPACE_END
