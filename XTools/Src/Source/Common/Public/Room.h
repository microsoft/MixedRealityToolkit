//////////////////////////////////////////////////////////////////////////
// Room.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

typedef ::XTools::int64 RoomID;

/// Represents a physical space or room in which a connected device resides.  
/// In general, if one device can see the other then they would be considered
/// to be in the same "Room"
class Room : public AtomicRefCounted
{
public:
	static const RoomID kInvalidRoomID = -1L;

	/// Returns the user-friendly name of the room
	virtual XStringPtr GetName() const = 0;

	/// Returns the unique ID of the room
	virtual RoomID GetID() const = 0;

	/// Returns the total number of users in the room
	virtual int32 GetUserCount() = 0;

	/// Returns the user at the given index in the room
	virtual UserID GetUserID(int32 userIndex) = 0;

	/// Returns true if the server should keep the room open even if there are no users in it
	virtual bool GetKeepOpen() const = 0;

	/// Tell the server whether or not it should keep this room open even if there are no users in it
	virtual void SetKeepOpen(bool keepOpen) = 0;

	/// Return the number of unique anchor names associated with this room
	virtual int32 GetAnchorCount() const = 0;

	/// Returns the name of the anchor at the given index
	virtual const XStringPtr& GetAnchorName(int32 index) const = 0;
};

DECLARE_PTR(Room)

XTOOLS_NAMESPACE_END
