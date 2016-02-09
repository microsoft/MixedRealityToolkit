//////////////////////////////////////////////////////////////////////////
// ImageLuminanceSource.cpp
//
// Implements ZXing's LuminanceSource interface to allow and ImageData
// object to be used in QR Code detection and decoding
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ImageLuminanceSource.h"

XTOOLS_NAMESPACE_BEGIN

ImageLuminanceSource::ImageLuminanceSource(const ImageDataPtr& image)
: LuminanceSource(image->GetWidth(), image->GetHeight())
, m_image(image)
{

}


zxing::ArrayRef<char> ImageLuminanceSource::getRow(int y, zxing::ArrayRef<char> row) const
{
	const char* pixelData = reinterpret_cast<const char*>(m_image->GetPixelData());
    const int32 bytesPerPixel = m_image->GetBytesPerPixel();


	const char* pixelRow = pixelData + y * getWidth() * bytesPerPixel;
	if (!row) 
	{
		row = zxing::ArrayRef<char>(getWidth());
	}

	for (int x = 0; x < getWidth(); x++) 
	{
		row[x] = convertPixel(pixelRow + (x * bytesPerPixel));
	}

	return row;
}


zxing::ArrayRef<char> ImageLuminanceSource::getMatrix() const
{
	const char* p = reinterpret_cast<const char*>(m_image->GetPixelData());
    const int32 bytesPerPixel = m_image->GetBytesPerPixel();

	zxing::ArrayRef<char> matrix(getWidth() * getHeight());

	char* m = &matrix[0];

	const int totalPixels = getWidth() * getHeight();

	// Directly set the value for grayscale images
	if (bytesPerPixel == 1 || bytesPerPixel == 2)
	{
		for (int i = 0; i < totalPixels; ++i)
		{
			m[i] = p[i * bytesPerPixel];
		}
	}
	else if (bytesPerPixel == 3 || bytesPerPixel == 4)
	{
		for (int i = 0; i < totalPixels; ++i)
		{
			m[i] = (char)((306 * (int)p[0] + 601 * (int)p[1] +
				117 * (int)p[2] + 0x200) >> 10);

			p += bytesPerPixel;
		}
	}

	return matrix;
}


char ImageLuminanceSource::convertPixel(const char* pixel) const
{
	const byte* uPixel = reinterpret_cast<const byte*>(pixel);
	const int32 bytesPerPixel = m_image->GetBytesPerPixel();
	if (bytesPerPixel == 1 || bytesPerPixel == 2)
	{
		// Gray or gray+alpha
		return pixel[0];
	} 
	else if (bytesPerPixel == 3 || bytesPerPixel == 4)
	{
		// Red, Green, Blue, (Alpha)

		// relative luminance: http://en.wikipedia.org/wiki/Relative_luminance
		// Y = 0.2126 R + 0.7152 G + 0.0722 B
		float R = (float)uPixel[0] / 255.f;
		float G = (float)uPixel[1] / 255.f;
		float B = (float)uPixel[2] / 255.f;
		float luminance = 0.2126f * R + 0.7152f * G + 0.0722f * B;

		char luminanceChar = (char)(luminance * 255.f);
		return luminanceChar;
	}
	else 
	{
		throw zxing::IllegalArgumentException("Unexpected image depth");
	}
}

XTOOLS_NAMESPACE_END
