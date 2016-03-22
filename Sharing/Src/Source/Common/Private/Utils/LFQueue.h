//////////////////////////////////////////////////////////////////////////
// LFQueue.h
//
// A single producer/single consumer lock-free queue for buffers of data
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <atomic>

XTOOLS_NAMESPACE_BEGIN

class LFQueue
{
public:
	// The amount of overhead in the queue per buffer pushed
	static const uint32 kElementOverhead = 4;

	explicit LFQueue(uint32 cacheSize);
	virtual ~LFQueue() {}

	uint32 GetSize() const;
	uint32 GetAllocatedSize() const;

	// Attempt to pop a buffer off the queue, returns false if there
	// was nothing to pop
	bool TryPop(Buffer& msg);

	// Try to push a new buffer onto the queue.  Returns false if the queue does not
	// have room to fit the buffer, either because its too full or just not big enough
	bool TryPush(const void* data, uint32 size);
	bool TryPush(const Buffer& msg);

private:
	uint32 ReadData(byte* data, uint32 size, uint32 startIndex);
	uint32 WriteData(const byte* data, uint32 size, uint32 endIndex);

	uint32 GetSize(uint32 startIndex, uint32 endIndex) const;

	scoped_array<byte>		m_data;
	std::atomic<uint32>		m_startIndex;
	std::atomic<uint32>		m_endIndex;
	uint32					m_allocSize;
};

XTOOLS_NAMESPACE_END