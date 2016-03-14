//////////////////////////////////////////////////////////////////////////
// BufferQueue.cpp
//
// Queue of buffers of data. Avoids making frequent allocations and is thread safe
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BufferQueue.h"

XTOOLS_NAMESPACE_BEGIN

BufferQueue::BufferQueue(uint32 initialSize)
: m_startIndex(0)
, m_endIndex(0)
, m_allocSize(0)
{
	Reserve(initialSize);
}


void BufferQueue::Pop(Buffer& msg)
{
	const uint32 startingSize = GetUsedSize();

	// Read the size
	uint32 msgSize = 0;
	ReadData(reinterpret_cast<byte*>(&msgSize), sizeof(msgSize));

	msg.Reset(msgSize);
	byte* data = msg.GetData();

	// Read the message data
	ReadData(data, msgSize);

	// Record the message size
	msg.SetUsedSize(msgSize);

	const uint32 endSize = GetUsedSize();
	XTASSERT(startingSize > endSize);
	XTASSERT(startingSize - endSize == msgSize + sizeof(msgSize));
}


void BufferQueue::Push(const byte* data, uint32 size)
{
	if (XTVERIFY(size > 0))
	{
		const uint32 startingSize = GetUsedSize();

		uint32 reserveSize = m_allocSize;
		while (startingSize + size + sizeof(uint32) >= reserveSize)
		{
			reserveSize *= 2;
		}

		Reserve(reserveSize);

		// Push the size of the message first
		WriteData(reinterpret_cast<byte*>(&size), sizeof(size));

		// Then push the data itself
		WriteData(data, size);

		const uint32 endSize = GetUsedSize();
		XTASSERT(endSize > startingSize);
		XTASSERT(endSize - startingSize == size + sizeof(size));
	}
}


void BufferQueue::Push(const Buffer& msg)
{
	Push(msg.GetData(), msg.GetSize());
}


uint32 BufferQueue::GetUsedSize() const
{
	return (m_startIndex <= m_endIndex) ? m_endIndex - m_startIndex : m_endIndex + (m_allocSize - m_startIndex);
}


void BufferQueue::Reserve(uint32 newSize)
{
	if (newSize > m_allocSize)
	{
		byte* newBuffer = new byte[newSize];

		uint32 originalSize = GetUsedSize();

		// If startIndex and endIndex are equal, then there's no existing 
		// data in the buffer to save

		// If the queue has not wrapped around...
		if (m_startIndex < m_endIndex)
		{
			uint32 copySize = m_endIndex - m_startIndex;
			XTASSERT(copySize <= m_allocSize);

			// Do a straight copy of the data to the start of the new buffer
			memcpy(newBuffer, m_data.get() + m_startIndex, copySize);
		}
		else if (m_endIndex < m_startIndex)
		{
			// The data wraps to the start of the buffer; do two copies
			uint32 startToEndSize = m_allocSize - m_startIndex;
			memcpy(newBuffer, m_data.get() + m_startIndex, startToEndSize);
			memcpy(newBuffer + startToEndSize, m_data.get(), m_endIndex);
		}

		m_data = newBuffer;
		m_startIndex = 0;
		m_endIndex = originalSize;
		m_allocSize = newSize;

		XTASSERT(GetUsedSize() == originalSize);
	}
}


void BufferQueue::ReadData(byte* data, uint32 size)
{
	// Copy the data
	uint32 copyStart = m_startIndex;
	uint32 remainingSize = size;
	for (; remainingSize > 0;)
	{
		copyStart = copyStart % m_allocSize;
		XTASSERT(copyStart < m_allocSize);

		uint32 copyEnd = std::min(copyStart + remainingSize, m_allocSize);
		uint32 copySize = copyEnd - copyStart;

		XTASSERT(copyEnd <= m_allocSize);
		XTASSERT(copySize <= remainingSize);


		memcpy(data, m_data.get() + copyStart, copySize);

		remainingSize -= copySize;
		data += copySize;

		XTASSERT(copyStart + copySize <= m_allocSize);
		copyStart = copyStart + copySize;
		XTASSERT(copyStart <= m_allocSize);
	}

	m_startIndex = copyStart;
}


void BufferQueue::WriteData(const byte* data, uint32 size)
{
	// Copy the data
	uint32 copyStart = m_endIndex;
	uint32 remainingSize = size;
	for (; remainingSize > 0;)
	{
		copyStart = copyStart % m_allocSize;
		XTASSERT(copyStart < m_allocSize);

		uint32 copyEnd = std::min(copyStart + remainingSize, m_allocSize);
		uint32 copySize = copyEnd - copyStart;

		XTASSERT(copyEnd <= m_allocSize);
		XTASSERT(copySize <= remainingSize);

		memcpy(m_data.get() + copyStart, data, copySize);

		remainingSize -= copySize;
		data += copySize;

		XTASSERT(copyStart + copySize <= m_allocSize);
		copyStart = copyStart + copySize;
		XTASSERT(copyStart <= m_allocSize);
	}

	m_endIndex = copyStart;
}


XTOOLS_NAMESPACE_END