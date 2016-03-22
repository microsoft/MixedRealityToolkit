//////////////////////////////////////////////////////////////////////////
// TunnelBridge.cpp
//
// Forward packets between two remote peers
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TunnelBridge.h"

//#define DEBUG_TUNNEL

XTOOLS_NAMESPACE_BEGIN

TunnelBridge::TunnelBridge(const NetworkConnectionPtr& serverConnection, const NetworkConnectionPtr& bConnection)
{
#if defined(DEBUG_TUNNEL)
	LogInfo("Tunnel constructed.  Server Connected: %s, Baraboo Connected: %s", (serverConnection->IsConnected()) ? "true" : "false", (bConnection->IsConnected()) ? "true" : "false");
#endif

	m_connections[TunnelIndex::ServerIndex] = serverConnection; 
	m_connections[TunnelIndex::SecondaryClientIndex] = bConnection;

	serverConnection->AddListenerAsync(MessageID::Tunnel, this);
	bConnection->AddListenerAsync(MessageID::Tunnel, this);

	SendConnectionMessages();
}


TunnelBridge::~TunnelBridge()
{
	// Clear out the listener connections first!  This ensure that this object will not receive
	// any more messages from the network thread once destruction is complete.  
	// This WILL block until any concurrently running OnMessageReceived() call finishes, so no need for
	// additional locks.  
	m_connections[TunnelIndex::ServerIndex]->RemoveListenerAsync(MessageID::Tunnel, this);
	m_connections[TunnelIndex::SecondaryClientIndex]->RemoveListenerAsync(MessageID::Tunnel, this);

	if (IsConnected())
	{
		SendTunnelControlMessage(m_connections[TunnelIndex::ServerIndex], RemotePeerDisconnected);
		SendTunnelControlMessage(m_connections[TunnelIndex::SecondaryClientIndex], RemotePeerDisconnected);
	}
}


bool TunnelBridge::IsConnected() const
{
	return (m_connections[TunnelIndex::ServerIndex]->IsConnected() && m_connections[TunnelIndex::SecondaryClientIndex]->IsConnected());
}


void TunnelBridge::OnConnected(const NetworkConnectionPtr& connection)
{
#if defined(DEBUG_TUNNEL)
	LogInfo("Tunnel connection from %s", (m_connections[TunnelIndex::ServerIndex].m_connection == &connection) ? "Server" : "Baraboo");
#else
	XT_UNREFERENCED_PARAM(connection);
#endif

	SendConnectionMessages();
}


void TunnelBridge::OnConnectFailed(const NetworkConnectionPtr&)
{
	
}


void TunnelBridge::OnDisconnected(const NetworkConnectionPtr& connection)
{
#if defined(DEBUG_TUNNEL)
	LogInfo("Tunnel Disconnected from %s", (m_connections[TunnelIndex::ServerIndex].m_connection == &connection) ?  "Server" : "Baraboo");
#else
	XT_UNREFERENCED_PARAM(connection);
#endif

	SendTunnelControlMessage(GetOtherConnection(connection), RemotePeerDisconnected);
}


// NOTE: Called on the network thread!!!!!
void TunnelBridge::OnMessageReceived(const NetworkConnectionPtr& connection, NetworkInMessage& message)
{
	NetworkConnectionPtr otherConnection = GetOtherConnection(connection);

	if (otherConnection && otherConnection->IsConnected())
	{
		const byte* inMsg = message.GetData();
		const uint32 inMsgSize = message.GetSize();

		// Get the tunnel header, but don't make a new packet; just forward on this one with the settings described in the header
		const NetworkHeader* tunnelHeader = reinterpret_cast<const NetworkHeader*>(inMsg);

		NetworkOutMessagePtr outMsg = otherConnection->CreateMessage(tunnelHeader->m_messageID);
		outMsg->WriteArray(inMsg + 1, inMsgSize - 1);

		// Send the message with the same settings that were used to get it here
		otherConnection->Send(outMsg, tunnelHeader->m_priority, tunnelHeader->m_reliability, tunnelHeader->m_channel);
	}
	else
	{
		LogWarning("Received a tunnel message but the receipient is not connected");
	}
}


void TunnelBridge::SendTunnelControlMessage(const NetworkConnectionPtr& connection, TunnelMsgType msgType) const
{
	if (connection && connection->IsConnected())
	{
		NetworkOutMessagePtr msg = connection->CreateMessage(MessageID::TunnelControl);

		msg->Write((byte)msgType);

		connection->Send(msg);
	}
}


const NetworkConnectionPtr& TunnelBridge::GetOtherConnection(const NetworkConnectionPtr& connection)
{
	return (m_connections[TunnelIndex::ServerIndex] == connection) ? m_connections[TunnelIndex::SecondaryClientIndex] : m_connections[TunnelIndex::ServerIndex];
}


void TunnelBridge::SendConnectionMessages()
{
	if (IsConnected())
	{
#if defined(DEBUG_TUNNEL)
		LogInfo("Tunnel Fully connected");
#endif

		SendTunnelControlMessage(m_connections[TunnelIndex::ServerIndex], RemotePeerConnected);
		SendTunnelControlMessage(m_connections[TunnelIndex::SecondaryClientIndex], RemotePeerConnected);
	}
}

XTOOLS_NAMESPACE_END
