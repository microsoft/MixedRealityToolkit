//////////////////////////////////////////////////////////////////////////
// TagImageImpl.cpp
//
// 
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TagImageImpl.h"

XTOOLS_NAMESPACE_BEGIN

TagImageImpl::TagImageImpl(int32 width, int32 height)
: m_data(new byte[width * height])
, m_width(width)
, m_height(height)
{
	// Initialize the data to all zeros
	memset(m_data.get(), 0, width * height);
}


int32 TagImageImpl::GetWidth() const
{
	return m_width;
}


int32 TagImageImpl::GetHeight() const
{
	return m_height;
}


void TagImageImpl::CopyImageData(byte* pixelBuffer, int32 bufferSize, int32 bytesPerPixel) const
{
	XTASSERT(bufferSize >= m_width * m_height * bytesPerPixel);
	for (int32 y = 0; y < m_height; ++y)
	{
		for (int32 x = 0; x < m_width; ++x)
		{
			byte value = GetPixel(x, y);

			if (bytesPerPixel == 1 || bytesPerPixel == 2)
			{
				pixelBuffer[0] = value;
			}
			else if (bytesPerPixel == 3 || bytesPerPixel == 4)
			{
				pixelBuffer[0] = value;
				pixelBuffer[1] = value;
				pixelBuffer[2] = value;
			}

			// Set the alpha on formats with alpha
			if (bytesPerPixel % 2 == 0)
			{
				pixelBuffer[bytesPerPixel - 1] = 0xFF;
			}

			pixelBuffer += bytesPerPixel;
		}
	}
}


byte TagImageImpl::GetPixel(int32 x, int32 y) const
{
	XTASSERT(x < m_width);
	XTASSERT(y < m_height);
	return m_data[y * m_width + x];
}


void TagImageImpl::SetPixel(int32 x, int32 y, byte value)
{
	XTASSERT(x < m_width);
	XTASSERT(y < m_height);
	m_data[y * m_width + x] = value;
}

XTOOLS_NAMESPACE_END