//////////////////////////////////////////////////////////////////////////
// TagImageImpl.h
//
// 
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class TagImageImpl : public TagImage
{
public:
	TagImageImpl(int32 width, int32 height);

	// TagImage Functions:
	virtual int32 GetWidth() const XTOVERRIDE;
	virtual int32 GetHeight() const XTOVERRIDE;

	// Fill the given array with the pixel values of the tag, based on the number of bytes per pixel you set
	virtual void CopyImageData(byte* pixelBuffer, int32 bufferSize, int32 bytesPerPixel) const XTOVERRIDE;

	// Local Functions:
	byte GetPixel(int32 x, int32 y) const;
	void SetPixel(int32 x, int32 y, byte value);

private:
	scoped_array<byte> m_data;
	int32 m_width;
	int32 m_height;
};

DECLARE_PTR(TagImageImpl)

XTOOLS_NAMESPACE_END