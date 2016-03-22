//////////////////////////////////////////////////////////////////////////
// LFQueue.cpp
//
// 
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LFQueue.h"

XTOOLS_NAMESPACE_BEGIN

LFQueue::LFQueue(uint32 cacheSize)
: m_data(new byte[cacheSize])
, m_startIndex(0)
, m_endIndex(0)
, m_allocSize(cacheSize)
{
	XTASSERT(m_startIndex.is_lock_free());
	XTASSERT(m_endIndex.is_lock_free());
}


uint32 LFQueue::GetSize() const
{
	uint32 endLocal = m_endIndex.load();
	uint32 startLocal = m_startIndex.load();

	return GetSize(startLocal, endLocal);
}


uint32 LFQueue::GetAllocatedSize() const
{
	return m_allocSize;
}


bool LFQueue::TryPop(Buffer& msg)
{
	const uint32 endLocal = m_endIndex.load();
	const uint32 startLocal = m_startIndex.load();

	// If the queue is empty, return false
	if (endLocal == startLocal)
	{
		return false;
	}

	const uint32 startingSize = GetSize(startLocal, endLocal);

	// First byte is always the length of the buffer
	uint32 bufferSize = 0;
	uint32 payloadStart = ReadData(reinterpret_cast<byte*>(&bufferSize), sizeof(bufferSize), startLocal);

	// Check that the indicated buffer size isn't something bigger than the entire stored data
	XTASSERT(bufferSize <= startingSize);

	msg.Reset(bufferSize);
	byte* dataOut = msg.GetData();

	uint32 newStartIndex = ReadData(dataOut, bufferSize, payloadStart);

	// Record the message size
	msg.SetUsedSize(bufferSize);

	// Atomically set the new start index
	m_startIndex.store(newStartIndex);

	const uint32 endSize = GetSize(newStartIndex, endLocal);
	XT_UNREFERENCED_PARAM(endSize);
	XTASSERT(startingSize > endSize);
	XTASSERT(startingSize - endSize == bufferSize + sizeof(bufferSize));

	return true;
}


bool LFQueue::TryPush(const void* data, uint32 size)
{
	if (XTVERIFY(size > 0))
	{
		// Write the data to the end, then update the end index

		const uint32 endLocal = m_endIndex.load();
		const uint32 startLocal = m_startIndex.load();

		const uint32 startingSize = GetSize(startLocal, endLocal);

		if (startingSize + size + sizeof(uint32) < m_allocSize)
		{
			// Push the size of the message first
			uint32 payloadStart = WriteData(reinterpret_cast<byte*>(&size), sizeof(size), endLocal);

			// Then push the data itself
			uint32 newEnd = WriteData(reinterpret_cast<const byte*>(data), size, payloadStart);

			const uint32 endSize = GetSize(startLocal, newEnd);
			XT_UNREFERENCED_PARAM(endSize);
			XTASSERT(endSize > startingSize);
			XTASSERT(endSize - startingSize == size + sizeof(size));

			m_endIndex.store(newEnd);

			return true;
		}
	}

	return false;
}


bool LFQueue::TryPush(const Buffer& msg)
{
	return TryPush(msg.GetData(), msg.GetSize());
}


uint32 LFQueue::ReadData(byte* data, uint32 size, uint32 startIndex)
{
	// Copy the data
	uint32 copyStart = startIndex;
	uint32 remainingSize = size;
	while (remainingSize > 0)
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

	return copyStart;
}


uint32 LFQueue::WriteData(const byte* data, uint32 size, uint32 endIndex)
{
	// Copy the data
	uint32 copyStart = endIndex;
	uint32 remainingSize = size;
	while (remainingSize > 0)
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

	return copyStart;
}


uint32 LFQueue::GetSize(uint32 startIndex, uint32 endIndex) const
{
	return (startIndex <= endIndex) ? endIndex - startIndex : endIndex + (m_allocSize - startIndex);
}

XTOOLS_NAMESPACE_END
