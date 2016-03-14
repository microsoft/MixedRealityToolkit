//////////////////////////////////////////////////////////////////////////
// BroadcastForwarder.cpp
//
// Listens for broadcast messages then forwards them to all the other connections
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BroadcastForwarder.h"
#include <Private/NetworkCommonPrivate.h>

XTOOLS_NAMESPACE_BEGIN

BroadcastForwarder::BroadcastForwarder()
{
}


void BroadcastForwarder::AddConnection(const NetworkConnectionPtr& connection)
{
	// Register for callbacks with the Broadcast id
	ConnectionInfo newConnection;
	newConnection.m_connection = connection;

	connection->AddListener(MessageID::Broadcast, this);

	m_connections.push_back(newConnection);
}


void BroadcastForwarder::RemoveConnection(const NetworkConnectionPtr& connection)
{
	bool found = false;

	// Find the remote client with the given connection and remove it
	for (size_t i = 0; i < m_connections.size(); ++i)
	{
		if (m_connections[i].m_connection == connection)
		{
			m_connections[i].m_connection->RemoveListener(MessageID::Broadcast, this);
			m_connections.erase(m_connections.begin() + i);
			found = true;
			break;
		}
	}

	XTASSERT(found);		// if not found, then invalid connection
}


void BroadcastForwarder::OnMessageReceived(const NetworkConnectionPtr& connection, NetworkInMessage& message)
{
	const byte* inMsg = message.GetData();
	const uint32 inMsgSize = message.GetSize();

	// Get the broadcast header
	const NetworkHeader* broadcastHeader = reinterpret_cast<const NetworkHeader*>(inMsg);

	// Extract the payload from the broadcast message and put in a new outgoing message
	const byte* payload = inMsg + sizeof(NetworkHeader);
	const uint32 payloadSize = inMsgSize - sizeof(NetworkHeader);

	if (payloadSize > 0)
	{
		NetworkOutMessagePtr outMsg = connection->CreateMessage(*payload);
		outMsg->WriteArray(payload + 1, payloadSize - 1);

		// Send the payload to all the other remote peers
		for (size_t i = 0; i < m_connections.size(); ++i)
		{
			NetworkConnectionPtr currentConnection = m_connections[i].m_connection;
			if (currentConnection != connection && currentConnection->IsConnected())
			{
				// Forward the message on with the same settings that it was sent here with
				currentConnection->Send(outMsg, broadcastHeader->m_priority, broadcastHeader->m_reliability, broadcastHeader->m_channel, false);
			}
		}

		connection->ReturnMessage(outMsg);
	}
}


XTOOLS_NAMESPACE_END