//////////////////////////////////////////////////////////////////////////
// DownloadBufferImpl.h
//
// Simple class that wraps a buffer used to hold data as it is downloaded,
// then pass it off to the user ensuring ownership is transferred safely and
// memory is released when the buffer is no longer needed
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <Public/DownloadBuffer.h>

XTOOLS_NAMESPACE_BEGIN

class DownloadBufferImpl : public DownloadBuffer
{
public:
	DownloadBufferImpl();
	explicit DownloadBufferImpl(uint32 reserveSize);
	virtual ~DownloadBufferImpl();

	// Returns the size of the buffer in bytes
	virtual uint32 GetSize() const XTOVERRIDE;

	// Returns a read-only pointer to the buffer
	virtual const byte* GetData() const XTOVERRIDE;

	// Releases ownership of the buffer to the caller.  Once this is called,
	// this object will no longer free the buffer's memory when it is destroyed
	virtual byte* ReleaseData() XTOVERRIDE;

	// Add the given data to the end of the current data array
	virtual void Append(void* data, uint32 size) XTOVERRIDE;

	// Reserve the given size and set the size used to zero
	virtual void Reset(uint32 newReservedSize) XTOVERRIDE;

private:
	byte* m_data;
	uint32 m_usedSize;
	uint32 m_reservedSize;
};

XTOOLS_NAMESPACE_END
