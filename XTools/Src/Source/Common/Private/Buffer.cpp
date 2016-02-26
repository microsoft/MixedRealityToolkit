//////////////////////////////////////////////////////////////////////////
// Buffer.cpp
//
// Simple class to hold a buffer of data and its size, and help avoid frequent reallocations
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Buffer.h"

XTOOLS_NAMESPACE_BEGIN

Buffer::Buffer(uint32 initialSize)
	: m_usedSize(0)
	, m_allocSize(0)
{
	Reset(initialSize);
}


Buffer::Buffer(const void* data, uint32 size)
	: m_usedSize(0)
	, m_allocSize(0)
{
	Set(reinterpret_cast<const byte*>(data), size);
}


Buffer::Buffer(const Buffer& rhs)
	: m_usedSize(0)
	, m_allocSize(0)
{
	CopyFrom(rhs);
}

Buffer& Buffer::operator=(const Buffer& rhs)
{
	CopyFrom(rhs);
	return *this;
}


const byte* Buffer::GetData() const
{
	return m_buffer.get();
}


uint32 Buffer::GetSize() const
{
	return m_usedSize;
}


void Buffer::Set(const byte* data, uint32 size)
{
	Reset(size);
	memcpy(m_buffer.get(), data, size);
	m_usedSize = size;
}


void Buffer::Append(const byte* data, uint32 size)
{
	XTASSERT(data != NULL);
	XTASSERT(size > 0);
	XTASSERT(m_usedSize + size <= m_allocSize);
	XTASSERT(m_buffer != NULL);
	memcpy(m_buffer.get() + m_usedSize, data, size);
	m_usedSize += size;
}


uint32 Buffer::GetAllocSize() const
{
	return m_allocSize;
}


void Buffer::Reset(uint32 newSize)
{
	if (newSize > m_allocSize)
	{
		m_buffer = new byte[newSize];
		m_allocSize = newSize;
	}

	m_usedSize = 0;
}


void Buffer::Resize(uint32 newSize)
{
	if (newSize > m_allocSize)
	{
		m_buffer = new byte[newSize];
		m_allocSize = newSize;
	}

	m_usedSize = newSize;
}


void Buffer::Clear()
{
	m_usedSize = 0;
}


void Buffer::TrimFront(uint32 size)
{
	if (size < m_usedSize)
	{
		uint32 startIndex = 0;
		for (uint32 i = size; size < m_usedSize; ++size)
		{
			m_buffer[startIndex++] = m_buffer[i];
		}

		m_usedSize -= size;
	}
	else
	{
		m_usedSize = 0;
	}
}


void Buffer::TrimBack(uint32 size)
{
	if (size < m_usedSize)
	{
		m_usedSize -= size;
	}
	else
	{
		m_usedSize = 0;
	}
}


bool Buffer::Equals(const Buffer& rhs) const
{
	if (m_usedSize != rhs.m_usedSize)
	{
		return false;
	}

	for (uint32 i = 0; i < m_usedSize; ++i)
	{
		if (m_buffer[i] != rhs.m_buffer[i])
		{
			return false;
		}
	}

	return true;
}


byte* Buffer::GetData()
{
	return m_buffer.get();
}


void Buffer::SetUsedSize(uint32 size)
{
	m_usedSize = size;
}


void Buffer::CopyFrom(const Buffer& rhs)
{
	if (rhs.m_allocSize == 0)
	{
		m_buffer = NULL;
		m_usedSize = 0;
		m_allocSize = 0;
	}
	else
	{
		m_buffer = new byte[rhs.m_allocSize];
		m_usedSize = rhs.m_usedSize;
		m_allocSize = rhs.m_allocSize;
		memcpy(m_buffer.get(), rhs.m_buffer.get(), rhs.m_usedSize);
	}
}

XTOOLS_NAMESPACE_END
