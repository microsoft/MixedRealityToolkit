//////////////////////////////////////////////////////////////////////////
// SessionListHandshakeLogic.cpp
//
// The logic and packet formats for the handshake between the desktop 
// app and the Session list server
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Public/SessionListHandshakeLogic.h>

XTOOLS_NAMESPACE_BEGIN

SessionListHandshakeLogic::SessionListHandshakeLogic(bool isServer)
: m_isServer(isServer)
{
}


void SessionListHandshakeLogic::CreateOutgoingMessage(const NetworkOutMessagePtr& msg) const
{
	if (m_isServer)
	{
		CreateMessageFromServer(msg);
	}
	else
	{
		CreateMessageFromDesktop(msg);
	}
}


bool SessionListHandshakeLogic::ValidateIncomingMessage(NetworkInMessage& msg) const
{
	if (m_isServer)
	{
		return ValidateMessageFromDesktop(msg);
	}
	else
	{
		return ValidateMessageFromServer(msg);
	}
}


void SessionListHandshakeLogic::CreateMessageFromServer(const NetworkOutMessagePtr& msg) const
{
	// Schema version
	msg->Write(kXToolsSchemaVersion);
}


bool SessionListHandshakeLogic::ValidateMessageFromServer(NetworkInMessage& msg) const
{
	// Schema version
	uint32 remoteSchemaVersion = msg.ReadUInt32();
	if (remoteSchemaVersion != kXToolsSchemaVersion)
	{
		LogWarning(	
			"\n\n***************************************************************\n"
			"List Server Handshake Failed: Invalid schema version. \n"
			"Expected: %i, got %i \n"
			"Please sync to latest XTools\n"
			"***************************************************************\n\n",
			kXToolsSchemaVersion, 
			remoteSchemaVersion);

		return false;
	}

	return true;
}


void SessionListHandshakeLogic::CreateMessageFromDesktop(const NetworkOutMessagePtr& msg) const
{
	// Schema version
	msg->Write(kXToolsSchemaVersion);
}


bool SessionListHandshakeLogic::ValidateMessageFromDesktop(NetworkInMessage& msg) const
{
	// Schema version
	uint32 remoteSchemaVersion = msg.ReadUInt32();
	if (remoteSchemaVersion != kXToolsSchemaVersion)
	{
		LogWarning(	
			"\n\n***************************************************************\n"
			"List Server Handshake Failed: Invalid schema version. \n"
			"Expected: %i, got %i \n"
			"Please sync to latest XTools\n"
			"***************************************************************\n\n",
			kXToolsSchemaVersion, 
			remoteSchemaVersion);

		return false;
	}

	return true;
}

XTOOLS_NAMESPACE_END

