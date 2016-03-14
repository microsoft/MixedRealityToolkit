//////////////////////////////////////////////////////////////////////////
// AnchorDownloadRequestImpl.cpp
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AnchorDownloadRequestImpl.h"

XTOOLS_NAMESPACE_BEGIN

AnchorDownloadRequestImpl::AnchorDownloadRequestImpl(const XStringPtr& anchorName, const RoomPtr& room)
	: m_name(anchorName)
	, m_room(room)
	, m_status(Status::Downloading)
{

}


XStringPtr AnchorDownloadRequestImpl::GetAnchorName() const
{
	return m_name;
}


RoomPtr AnchorDownloadRequestImpl::GetRoom() const
{
	return m_room;
}


int32 AnchorDownloadRequestImpl::GetDataSize() const
{
	return (m_data != nullptr) ? m_data->GetSize() : 0;
}

 
bool AnchorDownloadRequestImpl::GetData(byte* data, int32 dataSize) const
{
	if (m_status == Status::Cancelled)
	{
		LogError("Tried to retrieve anchor data from a request that was cancelled");
		return false;
	}

	if (m_data == nullptr || m_data->GetSize() == 0 || m_status == Status::Downloading)
	{
		LogError("Tried to retrieve anchor data before it has finished downloading");
		return false;
	}

	if (data == nullptr)
	{
		LogError("NULL data buffer passed to AnchorDownloadRequest::GetData()");
		return false;
	}

	// NOTE: promote to 64-bit int to allow for safe comparison of signed-vs-unsigned
	if ((int64)dataSize < (int64)m_data->GetSize())
	{
		LogError("Data buffer is not big enough.  Got %i, expected %%i", dataSize, m_data->GetSize());
		return false;
	}

	memcpy(data, m_data->GetData(), m_data->GetSize());

	return true;
}


void AnchorDownloadRequestImpl::SetData(const BufferPtr& data)
{
	m_data = data;
	m_status = Status::Downloaded;
}

XTOOLS_NAMESPACE_END
