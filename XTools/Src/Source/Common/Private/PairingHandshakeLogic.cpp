//////////////////////////////////////////////////////////////////////////
// PairingHandshakeLogic.cpp
//
// The logic and packet formats for the pairing handshake between the desktop 
// app and Baraboo
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Public/PairingHandshakeLogic.h>

XTOOLS_NAMESPACE_BEGIN

PairingHandshakeLogic::PairingHandshakeLogic(const UserPtr& user, bool isReceiver)
: m_user(user)
, m_isReceiver(isReceiver)
{
}


void PairingHandshakeLogic::CreateOutgoingMessage(const NetworkOutMessagePtr& msg) const
{
	if (m_isReceiver)
	{
		CreateMessageFromReceiver(msg);
	}
	else
	{
		CreateMessageFromConnector(msg);
	}
}


bool PairingHandshakeLogic::ValidateIncomingMessage(NetworkInMessage& msg) const
{
	if (m_isReceiver)
	{
		return ValidateMessageFromConnector(msg);
	}
	else
	{
		return ValidateMessageFromReceiver(msg);
	}
}


void PairingHandshakeLogic::CreateMessageFromConnector(const NetworkOutMessagePtr& msg) const
{
	// Schema version
	msg->Write(kXToolsSchemaVersion);
}


bool PairingHandshakeLogic::ValidateMessageFromConnector(NetworkInMessage& msg) const
{
	// Schema version
	uint32 remoteSchemaVersion = msg.ReadUInt32();
	if (remoteSchemaVersion != kXToolsSchemaVersion)
	{
		LogWarning("Handshake Failed: Invalid schema version");
		return false;
	}

	return true;
}


void PairingHandshakeLogic::CreateMessageFromReceiver(const NetworkOutMessagePtr& msg) const
{
	// Schema version
	msg->Write(kXToolsSchemaVersion);

	// UserID
	msg->Write(m_user->GetID());

	// The user's name
	msg->Write(m_user->GetName());

	// The current mute state
	msg->Write(m_user->GetMuteState() ? 1 : 0);
}


bool PairingHandshakeLogic::ValidateMessageFromReceiver(NetworkInMessage& msg) const
{
	// Schema version
	uint32 remoteSchemaVersion = msg.ReadUInt32();
	if (remoteSchemaVersion != kXToolsSchemaVersion)
	{
		LogWarning("Handshake Failed: Invalid schema version");
		return false;
	}

	// UserID
	UserID userID = msg.ReadInt32();
	if (userID == User::kInvalidUserID)
	{
		LogWarning("Handshake Failed: Invalid User ID");
		return false;
	}

	XStringPtr userName = msg.ReadString();
	if (userName->GetLength() == 0)
	{
		LogWarning("Handshake Failed: Invalid User Name");
		return false;
	}

	int32 muteStateValue = msg.ReadInt32();

	m_user->SetMuteState((muteStateValue > 0) ? true : false);
	m_user->SetID(userID);
	m_user->SetName(userName);

	return true;
}

XTOOLS_NAMESPACE_END
