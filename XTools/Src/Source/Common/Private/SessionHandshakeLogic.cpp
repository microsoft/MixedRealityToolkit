//////////////////////////////////////////////////////////////////////////
// SessionHandshakeLogic.cpp
//
// The logic and packet formats for the handshake between the desktop 
// app and a Session
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Public/SessionHandshakeLogic.h>

XTOOLS_NAMESPACE_BEGIN

SessionHandshakeLogic::SessionHandshakeLogic(bool isServer)
: m_isServer(isServer)
{
}


void SessionHandshakeLogic::CreateOutgoingMessage(const NetworkOutMessagePtr& msg) const
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


bool SessionHandshakeLogic::ValidateIncomingMessage(NetworkInMessage& msg) const
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


void SessionHandshakeLogic::CreateMessageFromServer(const NetworkOutMessagePtr& msg) const
{
	// Schema version
	msg->Write(kXToolsSchemaVersion);
}


bool SessionHandshakeLogic::ValidateMessageFromServer(NetworkInMessage& msg) const
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


void SessionHandshakeLogic::CreateMessageFromDesktop(const NetworkOutMessagePtr& msg) const
{
	// Schema version
	msg->Write(kXToolsSchemaVersion);
}


bool SessionHandshakeLogic::ValidateMessageFromDesktop(NetworkInMessage& msg) const
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
