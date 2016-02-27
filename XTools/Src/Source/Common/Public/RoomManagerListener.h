//////////////////////////////////////////////////////////////////////////
// RoomManagerListener.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

// Disable unused parameter warnings.  Pure virtual functions don't play nice with SWIG directors, 
// but we still want to see the names of the parameters to make reading the code clearer
#pragma warning( push )
#pragma warning( disable : 4100 ) 

XT_LISTENER_DECLARE(RoomManagerListener)

/// Base class for objects that want to receive callbacks from the RoomManager
/// about the completion of asynchronous operations
class RoomManagerListener XTABSTRACT : public Listener
{
public:
	virtual ~RoomManagerListener() {}

	/// Notification that a new room has been created.  Includes the creation of rooms by the local device.  
	/// \param newRoom The Room that was created
	virtual void OnRoomAdded(const RoomPtr& newRoom) {}

	/// Notification that a room has been closed.  This happens when the room has no more users.  
	/// \param room The Room that was closed
	virtual void OnRoomClosed(const RoomPtr& room) {}

	/// Notification that a User has entered a Room.  Includes the local user
	virtual void OnUserJoinedRoom(const RoomPtr& room, UserID user) {}

	/// Notification that a User has left a Room.  Includes the local user
	virtual void OnUserLeftRoom(const RoomPtr& room, UserID user) {}

	/// Notification that the anchors for a particular room have changed.  Note that this does
	/// not mean the new anchors have been downloaded, just that new anchor data is available to download.
	virtual void OnAnchorsChanged(const RoomPtr& room) {}

	/// Notification that an anchor download request has completed
	virtual void OnAnchorsDownloaded(bool successful, const AnchorDownloadRequestPtr& request, const XStringPtr& failureReason) {}

	/// Notification that the upload from this device to the server of new anchor data has finished.  
	virtual void OnAnchorUploadComplete(bool successful, const XStringPtr& failureReason) {}
};

#pragma warning( pop )

XTOOLS_NAMESPACE_END
