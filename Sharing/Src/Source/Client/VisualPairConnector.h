//////////////////////////////////////////////////////////////////////////
// VisualPairConnector.h
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

DECLARE_PTR_PRE(VisualPairConnector)

/// Takes an image from the user and searches for a visual pairing image
/// within the given image, decodes that information embedded in the image,
/// then uses that information to initiate a pairing with that remote client.  
class VisualPairConnector : public PairMaker
{
public:
	/// Create a new \ref VisualPairConnector
	static ref_ptr<VisualPairConnector> Create();

	/// Provide the connector with an image taken from the device's camera.  
	///
	/// Calling this function will kick off an asynchronous task to find any pairing tags in the image
	/// and decode the pairing information within it.  Image data is assumed to be in row major order.
	/// The number of bytes per pixel (BPP) can be 1, 2, 3 or 4.  The code assumes that:
	/// - 1 bpp is a greyscale image
	/// - 2 bpp is a greyscale + alpha image, alpha is ignored
	/// - 3 bpp is an RGB image
	/// - 4 bpp is an RGBA image, alpha is ignored
	/// \param image Pointer to a user-allocated array containing the image data
	/// \param width The number of pixels per row of the image
	/// \param height The number of pixels per column of the image
	/// \param bytesPerPixel The number of bytes of data that represent the color of each pixel.  
	/// \returns true if the async process is kicked off, false if another image is already being processed.  
	virtual bool ProcessImage(byte* image, int32 width, int32 height, int32 bytesPerPixel) = 0;

	/// Check if an image is being processed in the background.  
	/// \returns true if an image is currently being processed from a previous call to ProcessImage().  
	virtual bool IsProcessingImage() const = 0;
};

DECLARE_PTR_POST(VisualPairConnector)

XTOOLS_NAMESPACE_END
