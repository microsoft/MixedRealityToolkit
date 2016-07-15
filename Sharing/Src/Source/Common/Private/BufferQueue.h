// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// BufferQueue.h
// Queue of buffers of data. Avoids making frequent allocations and is thread safe
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "Buffer.h"

XTOOLS_NAMESPACE_BEGIN

class BufferQueue
{
public:
	BufferQueue(uint32 initialSize);

	uint32 GetUsedSize() const;

	void Pop(Buffer& msg);

	void Push(const byte* data, uint32 size);
	void Push(const Buffer& msg);

private:
	void Reserve(uint32 newSize);

	void ReadData(byte* data, uint32 size);
	void WriteData(const byte* data, uint32 size);

	scoped_array<byte>	m_data;
	uint32				m_startIndex;
	uint32				m_endIndex;
	uint32				m_allocSize;
};

XTOOLS_NAMESPACE_END
