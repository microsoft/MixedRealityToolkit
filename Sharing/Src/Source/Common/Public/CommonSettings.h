//////////////////////////////////////////////////////////////////////////
// CommonSettings.h
//
// Settings shared across all the different parts of XTools
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

//#define SYNC_DEBUG

const uint16 kSessionServerMaxConnections = 500;

const uint16 kSessionServerPort					= 0x507A;		// SOTA, in hex 1337speak
const uint16 kAppPluginPort						= 0x507B;
const uint16 kDiscoveryClientPort				= 0x507C;
const uint16 kDiscoveryServerPort				= 0x507D;

// Use different ports for each role, so that users can run them all on the same machine
const uint16 kProfilingPortServer				= 0x507F;
const uint16 kProfilingPortPrimaryClient		= 0x5080;
const uint16 kProfilingPortSecondaryClient		= 0x5081;

// Time out a connection if we have not heard from it after this long (in milliseconds)
const uint32 kConnectionTimeoutMS = 15000;	

static const uint32 kDefaultMessagePoolSize = 50;


// The minimum required length of a session's name
static const uint32 kMinSessionNameLength = 1;

// The maximum allowed length of a session's name
static const uint32 kMaxSessionNameLength = 1024;

// The minimum required length of a user's name
static const uint32 kMinUserNameLength = 1;

// The maximum allowed length of a user's name
static const uint32 kMaxUserNameLength = 30;

typedef std::vector<std::string> StringList;

XTOOLS_NAMESPACE_END
