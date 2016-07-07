// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// DownloadBufferImpl.cpp
// Simple class that wraps a buffer used to hold data as it is downloaded,
// then pass it off to the user ensuring ownership is transferred safely and
// memory is released when the buffer is no longer needed
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DownloadBufferImpl.h"

XTOOLS_NAMESPACE_BEGIN

DownloadBufferImpl::DownloadBufferImpl()
: m_data(nullptr)
, m_usedSize(0)
, m_reservedSize(0)
{

}


DownloadBufferImpl::DownloadBufferImpl(uint32 reserveSize)
	: m_data(nullptr)
	, m_usedSize(0)
	, m_reservedSize(reserveSize)
{
	m_data = new byte[m_reservedSize];
}


DownloadBufferImpl::~DownloadBufferImpl()
{
	if (m_data)
	{
		delete[] m_data;
		m_data = nullptr;
	}
}


uint32 DownloadBufferImpl::GetSize() const
{
	return m_usedSize;
}


const byte* DownloadBufferImpl::GetData() const
{
	return m_data;
}


byte* DownloadBufferImpl::ReleaseData()
{
	byte* temp = m_data;
	m_data = nullptr;
	m_usedSize = 0;
	m_reservedSize = 0;

	return temp;
}


void DownloadBufferImpl::Append(void* data, uint32 size)
{
	// Increase the size of the buffer if necessary
	{
		uint32 targetReservedSize = m_reservedSize;
		while (targetReservedSize - m_usedSize < size)
		{
			targetReservedSize *= 2;
		}

		if (targetReservedSize != m_reservedSize)
		{
			byte* newBuffer = new byte[targetReservedSize];

			if (m_data)
			{
				memcpy(newBuffer, m_data, m_usedSize);
				delete[] m_data;
			}

			m_data = newBuffer;
			m_reservedSize = targetReservedSize;
		}
	}

	// Copy the new data into the buffer
	memcpy(m_data + m_usedSize, data, size);
	m_usedSize += size;
}


void DownloadBufferImpl::Reset(uint32 newSize)
{
	if (newSize != m_reservedSize)
	{
		if (m_data)
		{
			delete[] m_data;
		}

		m_data = new byte[newSize];
		m_reservedSize = newSize;
	}

	m_usedSize = 0;
}

XTOOLS_NAMESPACE_END
