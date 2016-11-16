//////////////////////////////////////////////////////////////////////////
// XToolsVersion.h
//
// The version numbers for the current build
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

// The ID for the current schema used by the XTools network system.  If a connection is made between systems
// where this value does not match the connection is terminated to avoid corruption

// Please track what changed about the schema with each new revision
// eg: static const uint32 kXToolsSchemaVersion = 1234;	// Changed the format of the audio packet

// static const uint32 kXToolsSchemaVersion = 1;	// Initial revision
// static const uint32 kXToolsSchemaVersion = 2;	// Mute manager population during handshake.
// static const uint32 kXToolsSchemaVersion = 3;	// Added support for sync delete operations
// static const uint32 kXToolsSchemaVersion = 4;	// Added handshake between the desktop and the session list server and the session servers
// static const uint32 kXToolsSchemaVersion = 5;	// New tunneling and broadcast packet formats, to support channel settings across multiple hops
// static const uint32 kXToolsSchemaVersion = 6;	// Updated AudioSample packet format for HRTF, plus added 48000Hz rate to definitions
// static const uint32 kXToolsSchemaVersion = 7;	// Added UserUpdate messages for mute state and any other user changes we want.
// static const uint32 kXToolsSchemaVersion = 8;	// Refactored MuteManager into UserPresenceManager.
// static const uint32 kXToolsSchemaVersion = 9;	// Changes to the packet format of Sync data
// static const uint32 kXToolsSchemaVersion = 10;	// added average volume to audio packet
// static const uint32 kXToolsSchemaVersion = 11;	// Changed audio bit rate to 48k
//static const uint32 kXToolsSchemaVersion = 12;	// Changed pairing logic
//static const uint32 kXToolsSchemaVersion = 13;	// Added support for SendTo
//static const uint32 kXToolsSchemaVersion = 14;	// Added Room/Anchor support
//static const uint32 kXToolsSchemaVersion = 15;	// Added Boolean sync element type
//static const uint32 kXToolsSchemaVersion = 16;	// Added Float Array and String Array sync types
static const uint32 kXToolsSchemaVersion = 17;		// Added support for Types in sync system ObjectElements

XTOOLS_NAMESPACE_END
