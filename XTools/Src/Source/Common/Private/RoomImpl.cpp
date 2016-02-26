//////////////////////////////////////////////////////////////////////////
// RoomImpl.cpp
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RoomImpl.h"

XTOOLS_NAMESPACE_BEGIN

RoomImpl::RoomImpl(const RoomListenerListPtr& listeners)
	: m_listeners(listeners)
	, m_name(new XString("UnnamedRoom"))
	, m_id(kInvalidRoomID)
	, m_userListenerAdapter(new IntArrayAdapter())
{
	SetupMembers();
}


RoomImpl::RoomImpl(const RoomListenerListPtr& listeners, const XStringPtr& name, RoomID id)
	: m_listeners(listeners)
	, m_name(name)
	, m_id(id)
	, m_userListenerAdapter(new IntArrayAdapter())
{
	SetupMembers();
}


XStringPtr RoomImpl::GetName() const
{
	return m_name;
}


RoomID RoomImpl::GetID() const
{
	return m_id;
}


int32 RoomImpl::GetUserCount()
{
	return m_users.GetCount();
}


UserID RoomImpl::GetUserID(int32 userIndex)
{
	if (userIndex >= 0 && userIndex < m_users.GetCount())
	{
		return m_users[userIndex];
	}
	else
	{
		return User::kInvalidUserID;
	}
}


SyncArray& RoomImpl::GetUserArray()
{
	return m_users;
}


const SyncArray& RoomImpl::GetUserArray() const
{
	return m_users;
}


void RoomImpl::SetupMembers()
{
	AddMember(&m_name, "Name");
	AddMember(&m_id, "ID");
	AddMember(&m_users, "users");

	m_userListenerAdapter->SetValueInsertedCallback(CreateCallback2(this, &RoomImpl::OnUserAdded));
	m_userListenerAdapter->SetValueRemovedCallback(CreateCallback2(this, &RoomImpl::OnUserRemoved));

	m_users.AddListener(m_userListenerAdapter.get());
}


void RoomImpl::OnUserAdded(int32 , int32 userID)
{
	m_listeners->NotifyListeners(&RoomManagerListener::OnUserJoinedRoom, this, userID);
}


void RoomImpl::OnUserRemoved(int32 , int32 userID)
{
	m_listeners->NotifyListeners(&RoomManagerListener::OnUserLeftRoom, this, userID);
}

XTOOLS_NAMESPACE_END
