//////////////////////////////////////////////////////////////////////////
// RoomMessageID.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

// Enum of the message IDs used by the Room/Anchor sharing system
enum RoomMessageID : byte
{
	AnchorUploadRequest = 0,	// Sent from the client to the server, requesting that a new anchor be added
	AnchorUploadResponse,		// Response from the server about success or failure of an upload
	AnchorDownloadRequest,		// Request from the client asking to download an anchor
	AnchorDownloadResponse,		// Response from the server containing the anchor data that was requested
	AnchorsChangedNotification	// Sent from the server to all clients when an anchor for a room has changed
};

XTOOLS_NAMESPACE_END
