//////////////////////////////////////////////////////////////////////////
// RoomImpl.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "IntArrayAdapter.h"
#include "AnchorList.h"

XTOOLS_NAMESPACE_BEGIN

class RoomImpl : public Room, public SyncObject
{
public:
	typedef ListenerList<RoomManagerListener> RoomListenerList;
	DECLARE_PTR(RoomListenerList);

	RoomImpl(const RoomListenerListPtr& listeners);
	RoomImpl(const RoomListenerListPtr& listeners, const XStringPtr& name, RoomID id, bool bKeepOpen);

	/// Returns the user-friendly name of the room
	virtual XStringPtr			GetName() const XTOVERRIDE;

	/// Returns the unique ID of the room
	virtual RoomID				GetID() const XTOVERRIDE;

	/// Returns the total number of users in the room
	virtual int32				GetUserCount() XTOVERRIDE;

	/// Returns the user at the given index in the room
	virtual UserID				GetUserID(int32 userIndex) XTOVERRIDE;

	/// Returns true if the server should keep the room open even if there are no users in it
	virtual bool				GetKeepOpen() const XTOVERRIDE;

	/// Tell the server whether or not it should keep this room open even if there are no users in it
	virtual void				SetKeepOpen(bool keepOpen) XTOVERRIDE;

	/// Return the number of unique anchor names associated with this room
	virtual int32				GetAnchorCount() const XTOVERRIDE;

	/// Returns the name of the anchor at the given index
	virtual const XStringPtr&	GetAnchorName(int32 index) const XTOVERRIDE;

	SyncArray&					GetUserArray();
	const SyncArray&			GetUserArray() const;

	// Remove all the anchors.  Called when disconnected from the server
	void						ClearAnchors();

private:
	void						SetupMembers();

	void						OnUserAdded(int32 index, int32 userID);
	void						OnUserRemoved(int32 index, int32 userID);

	RoomListenerListPtr	m_listeners;
	SyncString			m_name;
	SyncLong			m_id;
	SyncBool			m_keepOpen;
	SyncArray			m_users;	// Array of all the UserIDs of the users in this room
	AnchorList			m_anchors;
	IntArrayAdapterPtr	m_userListenerAdapter;
};

DECLARE_PTR(RoomImpl)

XTOOLS_NAMESPACE_END
