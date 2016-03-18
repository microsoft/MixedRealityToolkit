//////////////////////////////////////////////////////////////////////////
// ServerRoom.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <Private/Buffer.h>
#include <Private/IntArrayAdapter.h>
#include "ServerAnchorList.h"

XTOOLS_NAMESPACE_BEGIN

class RoomEmptyListener;

class ServerRoom : public AtomicRefCounted, public SyncObject
{
public:
	ServerRoom(RoomEmptyListener* listener);

	XStringPtr	GetName() const;
	RoomID		GetID() const;

	bool		GetKeepOpen() const;
	void		SetKeepOpen(bool keepOpen);

	// Removes the user from the room.  Returns true if the user was in the room
	bool		RemoveUser(UserID user);

	// Returns true if there are no users in the room
	bool		IsEmpty() const;

	void		SetAnchor(const std::string& name, const BufferPtr& data);

	void		RemoveAnchor(const std::string& name);

	BufferPtr	GetAnchorData(const std::string& name);

private:
	void		OnUserRemoved(int32 index, int32 userID);

	SyncString			m_name;
	SyncLong			m_id;
	SyncBool			m_keepOpen;
	SyncArray			m_users;	// Array of all the UserIDs of the users in this room
	ServerAnchorList	m_anchors;
	IntArrayAdapterPtr	m_userListenerAdapter;
	RoomEmptyListener*	m_roomEmptylistener;
};

DECLARE_PTR(ServerRoom)


class RoomEmptyListener
{
public:
	virtual void OnRoomEmpty(const ServerRoomPtr& room) = 0;
};

XTOOLS_NAMESPACE_END
