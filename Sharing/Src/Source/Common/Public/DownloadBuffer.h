//////////////////////////////////////////////////////////////////////////
// DownloadBuffer.h
//
// Simple class that wraps a buffer used to hold data as it is downloaded,
// then pass it off to the user ensuring ownership is transferred safely and
// memory is released when the buffer is no longer needed
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class DownloadBuffer : public AtomicRefCounted
{
public:
	// Returns the size of the buffer in bytes
	virtual uint32 GetSize() const = 0;

	// Returns a read-only pointer to the buffer
	virtual const byte* GetData() const = 0;

	// Releases ownership of the buffer to the caller.  Once this is called,
	// this object will no longer free the buffer's memory when it is destroyed
	virtual byte* ReleaseData() = 0;

	// Add the given data to the end of the current data array
	virtual void Append(void* data, uint32 size) = 0;

	// Reserve the given size and set the size used to zero
	virtual void Reset(uint32 newReservedSize) = 0;
};

DECLARE_PTR(DownloadBuffer)

XTOOLS_NAMESPACE_END
