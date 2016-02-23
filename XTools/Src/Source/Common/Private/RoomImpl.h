//////////////////////////////////////////////////////////////////////////
// RoomImpl.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class RoomImpl : public Room, public SyncObject
{
public:
	RoomImpl();
	RoomImpl(const XStringPtr& name, RoomID id);

	/// Returns the user-friendly name of the room
	virtual XStringPtr GetName() const XTOVERRIDE;

	/// Returns the unique ID of the room
	virtual RoomID GetID() const XTOVERRIDE;

private:
	void SetupMemberBinding();

	SyncString	m_name;
	SyncLong	m_id;
};

DECLARE_PTR(RoomImpl)

XTOOLS_NAMESPACE_END
