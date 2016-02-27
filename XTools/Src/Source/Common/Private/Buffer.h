//////////////////////////////////////////////////////////////////////////
// Buffer.h
//
// Simple class to hold a buffer of data and its size, and help avoid frequent reallocations
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class Buffer
{
public:
	Buffer(uint32 initialSize);
	Buffer(const void* data, uint32 size);
	Buffer(const Buffer& rhs);

	Buffer& operator=(const Buffer& rhs);

	byte* GetData();
	const byte* GetData() const;

	// Returns the amount of the buffer used, not the size allocated
	uint32 GetSize() const;

	// Sets the data that the buffer holds.  Will only allocate new memory if the 
	// current buffer is not big enough
	void Set(const byte* data, uint32 size);

	// Adds the given buffer to the end of the current buffer.  Does not allocate,
	// and will assert if attempting to append more data than the buffer can hold
	void Append(const byte* data, uint32 size);

	// Reserve the given size and set the size used to zero
	void Reset(uint32 newSize);

	// Reserve the given size and mark it as the used size
	void Resize(uint32 newSize);

	// Set the used size to zero, but do not resize the reserved buffer
	void Clear();

	// Remove the given number of bytes from the front of the buffer.  Does not realloc
	void TrimFront(uint32 size);

	// Remove the given number of bytes from the back of the buffer.  Does not realloc
	void TrimBack(uint32 size);

	// Note: not a cheap operation, so not implemented using overloaded operator==()
	bool Equals(const Buffer& rhs) const;

	friend class BufferQueue;
	friend class LFQueue;
private:
	uint32 GetAllocSize() const;
	
	void SetUsedSize(uint32 size);

	void CopyFrom(const Buffer& rhs);

	scoped_array<byte>	m_buffer;
	uint32				m_usedSize;
	uint32				m_allocSize;
};

typedef std::shared_ptr<Buffer> BufferPtr;

XTOOLS_NAMESPACE_END
