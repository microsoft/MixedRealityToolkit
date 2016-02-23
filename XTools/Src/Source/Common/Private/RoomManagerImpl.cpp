//////////////////////////////////////////////////////////////////////////
// RoomManagerImpl.cpp
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RoomManagerImpl.h"

XTOOLS_NAMESPACE_BEGIN

RoomManagerImpl::RoomManagerImpl(const ClientContextConstPtr& context)
	: m_context(context)
{

}


void RoomManagerImpl::AddListener(RoomManagerListener* newListener)
{
	m_listenerList->AddListener(newListener);
}

 
void RoomManagerImpl::RemoveListener(RoomManagerListener* oldListener)
{
	m_listenerList->RemoveListener(oldListener);
}


int32 RoomManagerImpl::GetRoomCount() const
{
	return 0;
}


RoomPtr RoomManagerImpl::GetRoom(int32 index)
{
	XT_UNREFERENCED_PARAM(index);
	return nullptr;
}


RoomPtr RoomManagerImpl::GetCurrentRoom()
{
	return nullptr;
}


RoomPtr RoomManagerImpl::CreateRoom(const XStringPtr& roomName, RoomID roomID)
{
	XT_UNREFERENCED_PARAM(roomName);
	XT_UNREFERENCED_PARAM(roomID);
	return nullptr;
}


bool RoomManagerImpl::JoinRoom(const RoomPtr& room)
{
	XT_UNREFERENCED_PARAM(room);
	return false;
}


bool RoomManagerImpl::LeaveRoom(const RoomPtr& room)
{
	XT_UNREFERENCED_PARAM(room);
	return false;
}


int32 RoomManagerImpl::GetUserCount(const RoomPtr& room)
{
	XT_UNREFERENCED_PARAM(room);
	return 0;
}


UserPtr RoomManagerImpl::GetUser(const RoomPtr& room, int32 userIndex)
{
	XT_UNREFERENCED_PARAM(room);
	XT_UNREFERENCED_PARAM(userIndex);
	return nullptr;
}


int32 RoomManagerImpl::GetAnchorCount(const RoomPtr& room)
{
	XT_UNREFERENCED_PARAM(room);
	return 0;
}


XStringPtr RoomManagerImpl::GetAnchorName(const RoomPtr& room, int32 anchorIndex)
{
	XT_UNREFERENCED_PARAM(room);
	XT_UNREFERENCED_PARAM(anchorIndex);
	return nullptr;
}


AnchorDownloadRequestPtr RoomManagerImpl::DownloadAnchor(const RoomPtr& room, const XStringPtr& anchorName)
{
	XT_UNREFERENCED_PARAM(room);
	XT_UNREFERENCED_PARAM(anchorName);
	return nullptr;
}

 
bool RoomManagerImpl::UploadAnchor(const RoomPtr& room, const XStringPtr& anchorName, const byte* data, int32 dataSize)
{
	XT_UNREFERENCED_PARAM(room);
	XT_UNREFERENCED_PARAM(anchorName);
	XT_UNREFERENCED_PARAM(data);
	XT_UNREFERENCED_PARAM(dataSize);
	return false;
}


void RoomManagerImpl::OnElementAdded(const ElementPtr& element)
{
	XT_UNREFERENCED_PARAM(element);
}


void RoomManagerImpl::OnElementDeleted(const ElementPtr& element)
{
	XT_UNREFERENCED_PARAM(element);
}

XTOOLS_NAMESPACE_END
