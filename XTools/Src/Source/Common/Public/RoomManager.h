//////////////////////////////////////////////////////////////////////////
// RoomManager.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class RoomManager : public AtomicRefCounted
{
public:
	/// Register an object to receive callbacks when an async operation is complete.  
	/// Multiple listeners can be registered.  The wrapper class will hold a reference to the listener
	/// to ensure it is not garbage collected until this class is destroyed or the listener is removed. 
	/// \param newListener The listener object that will receive callbacks
	virtual void AddListener(RoomManagerListener* newListener) = 0;

	/// Remove a previously registered listener.  The wrapper class will release its reference to the given listener.  
	/// \param oldListener The listener object that will no longer receive callbacks  
	virtual void RemoveListener(RoomManagerListener* oldListener) = 0;

	/// Returns the number of rooms that are available in the current session
	virtual int32 GetRoomCount() const = 0;

	/// Returns the room at the given index
	virtual RoomPtr GetRoom(int32 index) = 0;

	/// Returns the room that the local user is currently a part of
	virtual RoomPtr GetCurrentRoom() = 0;

	/// Creates a new room with the given name and ID and adds the local user to that room.
	/// The room is created immediately and the remote devices are notified asynchronously.  
	/// Returns the newly created room if successful, or null if a room with the same name or ID already exists 
	virtual RoomPtr CreateRoom(const XStringPtr& roomName, RoomID roomID, bool keepOpenWhenEmpty) = 0;

	/// Add the local user to the given room.  If the user is currently in another room, they will automatically 
	/// leave the old room before joining the new one.  
	/// Returns true on success
	virtual bool JoinRoom(const RoomPtr& room) = 0;

	/// Remove the local user from their current room.  
	/// Returns true on success
	virtual bool LeaveRoom() = 0;

	/// Begins the asynchronous download of an anchor in the given room from the session server.  
	/// Returns an AnchorDownloadRequest object that allows the user to track or cancel the download and retrieve the
	/// data once its finished downloading
	virtual bool DownloadAnchor(const RoomPtr& room, const XStringPtr& anchorName) = 0;

	/// Begin the asynchronous upload of an anchor.  A copy of the data is created internally,
	/// so it is safe to release once this function returns.  
	/// Returns false if an upload is already in progress.  
	virtual bool UploadAnchor(const RoomPtr& room, const XStringPtr& anchorName, const byte* data, int32 dataSize) = 0;
};

DECLARE_PTR(RoomManager)

XTOOLS_NAMESPACE_END
