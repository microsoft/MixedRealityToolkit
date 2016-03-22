//////////////////////////////////////////////////////////////////////////
// TagImage.h
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

/// An image of a rectangular tag where the pixel values can only be on or off
class TagImage : public AtomicRefCounted
{
public:
	/// The number of pixels across the tag image
	virtual int32 GetWidth() const = 0;

	/// The number of pixels down the tag image
	virtual int32 GetHeight() const = 0;

	/// Fill the given array with the pixel values of the tag, based on the number of bytes per pixel you set. 
	/// The number of bytes per pixel (BPP) can be 1, 2, 3 or 4.  The code assumes that:
	/// - 1 bpp is a greyscale image
	/// - 2 bpp is a greyscale + alpha image, alpha is ignored
	/// - 3 bpp is an RGB image
	/// - 4 bpp is an RGBA image, alpha is ignored
	/// \param data Pointer to the pixel buffer to fill
	/// \param bufferSize The size in bytes of the array that 'data' points to
	/// \param bytesPerPixel The number of bytes of data that represent the color of each pixel.  
	virtual void CopyImageData(byte* data, int32 bufferSize, int32 bytesPerPixel) const = 0;
};

DECLARE_PTR(TagImage)

XTOOLS_NAMESPACE_END