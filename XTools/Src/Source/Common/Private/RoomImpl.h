//////////////////////////////////////////////////////////////////////////
// RoomImpl.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "IntArrayAdapter.h"

XTOOLS_NAMESPACE_BEGIN

class RoomImpl : public Room, public SyncObject
{
public:
	typedef ListenerList<RoomManagerListener> RoomListenerList;
	DECLARE_PTR(RoomListenerList);

	RoomImpl(const RoomListenerListPtr& listeners);
	RoomImpl(const RoomListenerListPtr& listeners, const XStringPtr& name, RoomID id);

	/// Returns the user-friendly name of the room
	virtual XStringPtr	GetName() const XTOVERRIDE;

	/// Returns the unique ID of the room
	virtual RoomID		GetID() const XTOVERRIDE;

	/// Returns the total number of users in the room
	virtual int32		GetUserCount() XTOVERRIDE;

	/// Returns the user at the given index in the room
	virtual UserID		GetUserID(int32 userIndex) XTOVERRIDE;

	SyncArray&			GetUserArray();
	const SyncArray&	GetUserArray() const;

private:
	void				SetupMembers();

	void OnUserAdded(int32 index, int32 userID);
	void OnUserRemoved(int32 index, int32 userID);

	RoomListenerListPtr	m_listeners;
	SyncString			m_name;
	SyncLong			m_id;
	SyncArray			m_users;	// Array of all the UserIDs of the users in this room
	IntArrayAdapterPtr	m_userListenerAdapter;
};

DECLARE_PTR(RoomImpl)

XTOOLS_NAMESPACE_END
