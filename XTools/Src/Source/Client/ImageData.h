//////////////////////////////////////////////////////////////////////////
// ImageData.h
//
// Simple container for image data.  Uses thread-safe reference counting
// so it can be passed around to different threads for processing
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class ImageData : public AtomicRefCounted
{
public:
	ImageData(const byte* pixelData, int32 width, int32 height, int32 bytesPerPixel);

	const byte*		GetPixelData() const;
	int32			GetWidth() const;
	int32			GetHeight() const;
	int32			GetBytesPerPixel() const;
	
private:
	scoped_array<byte>	m_pixelData;
	int32				m_imageWidth;
	int32				m_imageHeight;
	int32				m_bytesPerPixel;
};

DECLARE_PTR(ImageData)

XTOOLS_NAMESPACE_END