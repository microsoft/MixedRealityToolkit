//////////////////////////////////////////////////////////////////////////
// ServerRoom.cpp
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ServerRoom.h"

XTOOLS_NAMESPACE_BEGIN

ServerRoom::ServerRoom(RoomEmptyListener* listener)
	: SyncObject("Room")
	, m_name(new XString("UnnamedRoom"))
	, m_id(Room::kInvalidRoomID)
	, m_keepOpen(false)
	, m_roomEmptylistener(listener)
	, m_userListenerAdapter(new IntArrayAdapter())
{
	AddMember(&m_name, "Name");
	AddMember(&m_id, "ID");
	AddMember(&m_keepOpen, "keepOpen");
	AddMember(&m_users, "users");
	AddMember(&m_anchors, "Anchors");

	m_userListenerAdapter->SetValueRemovedCallback(CreateCallback2(this, &ServerRoom::OnUserRemoved));

	m_users.AddListener(m_userListenerAdapter.get());
}


XStringPtr ServerRoom::GetName() const
{
	return m_name;
}


RoomID ServerRoom::GetID() const
{
	return m_id;
}


bool ServerRoom::GetKeepOpen() const
{
	return m_keepOpen;
}


void ServerRoom::SetKeepOpen(bool keepOpen)
{
	m_keepOpen = keepOpen;
}


bool ServerRoom::RemoveUser(UserID userID)
{
	for (int32 i = 0; i < m_users.GetCount(); ++i)
	{
		if (m_users[i] == userID)
		{
			m_users.Remove(i);
			return true;
		}
	}

	return false;
}


bool ServerRoom::IsEmpty() const
{
	return (m_users.GetCount() == 0);
}


void ServerRoom::SetAnchor(const std::string& name, const BufferPtr& data)
{
	m_anchors.SetAnchor(name, data);
}


void ServerRoom::RemoveAnchor(const std::string& name)
{
	m_anchors.RemoveAnchor(name);
}


BufferPtr ServerRoom::GetAnchorData(const std::string& name)
{
	return m_anchors.GetAnchorData(name);
}


void ServerRoom::OnUserRemoved(int32, int32 )
{
	// If there are no long any users left in the room, notify the listener
	if (m_users.GetCount() == 0 && m_roomEmptylistener)
	{
		m_roomEmptylistener->OnRoomEmpty(this);
	}
}

XTOOLS_NAMESPACE_END
