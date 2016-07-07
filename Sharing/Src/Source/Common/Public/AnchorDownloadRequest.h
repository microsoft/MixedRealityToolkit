// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// AnchorDownloadRequest.h
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

/// Acts as the interface for a download of anchor data from the server.  
class AnchorDownloadRequest : public AtomicRefCounted
{
public:
	/// Returns the name of the anchor that was requested
	virtual XStringPtr GetAnchorName() const = 0;

	/// Returns the Room that the requested anchor is in
	virtual RoomPtr GetRoom() const = 0;

	/// Returns the size of the downloaded data.  Returns 0 until the download completes
	virtual int32 GetDataSize() const = 0;

	/// Copies the downloaded data into the given buffer.  
	/// Returns true on success, false if the data has not completed download or the size of the given buffer is no
	/// big enough to hold the data.  
	virtual bool GetData(byte* data, int32 dataSize) const = 0;
};

DECLARE_PTR(AnchorDownloadRequest)

XTOOLS_NAMESPACE_END
