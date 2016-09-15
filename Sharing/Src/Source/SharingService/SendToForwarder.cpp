//////////////////////////////////////////////////////////////////////////
// SendToForwarder.cpp
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SendToForwarder.h"
#include <Private/NetworkCommonPrivate.h>

XTOOLS_NAMESPACE_BEGIN

SendToForwarder::SendToForwarder()
{
}


void SendToForwarder::AddConnection(UserID userID, const NetworkConnectionPtr& primaryConnection, const NetworkConnectionPtr& secondaryConnection)
{
	primaryConnection->AddListener(MessageID::SendTo, this);
	secondaryConnection->AddListener(MessageID::SendTo, this);

	ConnectionInfo newConnection;
	newConnection.m_primaryConnection = primaryConnection;
	newConnection.m_secondaryConnection = secondaryConnection;

	m_connections[userID] = newConnection;
}


void SendToForwarder::RemoveConnection(UserID userID)
{
	auto item = m_connections.find(userID);
	if (XTVERIFY(item != m_connections.end()))
	{
		item->second.m_primaryConnection->RemoveListener(MessageID::SendTo, this);
		item->second.m_secondaryConnection->RemoveListener(MessageID::SendTo, this);
		m_connections.erase(item);
	}
}


void SendToForwarder::OnMessageReceived(const NetworkConnectionPtr& connection, NetworkInMessage& message)
{
	const byte* inMsg = message.GetData();
	const uint32 inMsgSize = message.GetSize();

	// Get the sendto header
	const SendToNetworkHeader* sendToHeader = reinterpret_cast<const SendToNetworkHeader*>(inMsg);

	// Extract the payload from the sendto message and put in a new outgoing message
	const byte* payload = inMsg + sizeof(SendToNetworkHeader);
	const uint32 payloadSize = inMsgSize - sizeof(SendToNetworkHeader);

	if (payloadSize > 0)
	{
		NetworkOutMessagePtr outMsg = connection->CreateMessage(*payload);
		outMsg->WriteArray(payload + 1, payloadSize - 1);

		// Send the payload to the correct remote peer
		auto userConnectionItr = m_connections.find(sendToHeader->m_userID);
		if (userConnectionItr != m_connections.end())
		{
			ClientRole role = sendToHeader->m_deviceRole;

			if (role == ClientRole::Primary ||
				role == ClientRole::Unspecified)
			{
				// Forward the message on with the same settings that it was sent here with
				userConnectionItr->second.m_primaryConnection ->Send(outMsg, sendToHeader->m_priority, sendToHeader->m_reliability, sendToHeader->m_channel, false);
			}

			if (role == ClientRole::Secondary ||
				role == ClientRole::Unspecified)
			{
				// Forward the message on with the same settings that it was sent here with
				userConnectionItr->second.m_secondaryConnection->Send(outMsg, sendToHeader->m_priority, sendToHeader->m_reliability, sendToHeader->m_channel, false);
			}
		}

		connection->ReturnMessage(outMsg);
	}
}

XTOOLS_NAMESPACE_END
