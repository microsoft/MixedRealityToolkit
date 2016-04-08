//////////////////////////////////////////////////////////////////////////
// XSocketImpl.cpp
//
// Implementation of the XSocket interface.  Allows us to limit the 
// exposure of the RakNet APIs to code outside the common library
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XSocketImpl.h"
#include "XSocketManagerImpl.h"

XTOOLS_NAMESPACE_BEGIN

// Ensure that our wrapper-exposed enums have values that match their counterparts in RakNet
static_assert(static_cast<int>(MessagePriority::Immediate)	== static_cast<int>(IMMEDIATE_PRIORITY), "XTools Priority enum does not match RakNet");
static_assert(static_cast<int>(MessagePriority::High)		== static_cast<int>(HIGH_PRIORITY), "XTools Priority enum does not match RakNet");
static_assert(static_cast<int>(MessagePriority::Medium)		== MEDIUM_PRIORITY, "XTools Priority enum does not match RakNet");
static_assert(static_cast<int>(MessagePriority::Low)		== LOW_PRIORITY, "XTools Priority enum does not match RakNet");

static_assert(static_cast<int>(MessageReliability::Unreliable)			== static_cast<int>(PacketReliability::UNRELIABLE), "XTools MessageReliability enum does not match RakNet");
static_assert(static_cast<int>(MessageReliability::UnreliableSequenced)	== static_cast<int>(PacketReliability::UNRELIABLE_SEQUENCED), "XTools MessageReliability enum does not match RakNet");
static_assert(static_cast<int>(MessageReliability::Reliable)			== static_cast<int>(PacketReliability::RELIABLE), "XTools MessageReliability enum does not match RakNet");
static_assert(static_cast<int>(MessageReliability::ReliableOrdered)		== static_cast<int>(PacketReliability::RELIABLE_ORDERED), "XTools MessageReliability enum does not match RakNet");
static_assert(static_cast<int>(MessageReliability::ReliableSequenced)	== static_cast<int>(PacketReliability::RELIABLE_SEQUENCED), "XTools MessageReliability enum does not match RakNet");

XTOOLS_REFLECTION_DEFINE(XSocket)
.BaseClass<Reflection::XTObject>();

XTOOLS_REFLECTION_DEFINE(XSocketImpl)
.BaseClass<XSocket>();

std::atomic<SocketID> XSocketImpl::m_sCounter(0);


XSocketImpl::XSocketImpl(PeerID peerID, const std::string& remoteName, uint16 remotePort)
	: m_id(++m_sCounter)
	, m_peerID(peerID)
	, m_listener(NULL)
	, m_address(remoteName)
	, m_port(remotePort)
	, m_raknetAddress(RakNet::UNASSIGNED_SYSTEM_ADDRESS)
	, m_raknetGuid(RakNet::UNASSIGNED_RAKNET_GUID)
	, m_status(Connecting)	// Default status to connecting
{

}


SocketID XSocketImpl::GetID() const
{
	return m_id;
}


void XSocketImpl::Send(const byte* message, uint32 messageSize, MessagePriority priority, MessageReliability reliability, MessageChannel channel)
{
	XTASSERT(m_status == Connected);

	if (m_peer)
	{
		uint32 messageID = m_peer->Send(reinterpret_cast<const char*>(message), messageSize, (PacketPriority)priority, (PacketReliability)reliability, (char)channel, m_raknetAddress, false);
		if (messageID == 0)
		{
			XTASSERT(false);
			LogWarning("Failed to send message to %s", m_address.c_str());
		}
	}
}


ReceiptPtr XSocketImpl::RegisterListener(XSocketListener* listener)
{
	XTASSERT(m_listener == NULL);
	m_listener = listener;

	return CreateRegistrationReceipt(XSocketImplPtr(this), &XSocketImpl::UnregisterListener, listener);
}


void XSocketImpl::UnregisterListener(XSocketListener* )
{
	XTASSERT(m_listener != NULL);
	m_listener = NULL;
}


XSocketImpl::Status XSocketImpl::GetStatus() const
{
	return m_status;
}


bool XSocketImpl::IsConnected() const
{
	return (m_status == Connected);
}


const std::string& XSocketImpl::GetRemoteSystemName() const
{
	return m_address;
}


uint16 XSocketImpl::GetRemoteSystemPort() const
{
	return m_port;
}


void XSocketImpl::OnReceiveMessage(const MessageConstPtr& msg)
{
	byte packetID = msg->GetMessageID();

	// If this is a user's packet, forward it to them
	if (packetID >= ID_USER_PACKET_ENUM)
	{
		if (m_listener)
		{
			m_listener->OnMessageReceived(this, msg->GetData(), msg->GetSize());
		}
		else
		{
			LogError("XSocket received message with no listener");
		}
	}
	else
	{
		// Check to see if this is a packet we should handle (connected, disconnected, etc)
		switch (packetID)
		{
		case ID_CONNECTION_REQUEST_ACCEPTED:
			OnConnected();
			break;

		case ID_DISCONNECTION_NOTIFICATION:
			OnLostConnection();
			break;

		case ID_CONNECTION_LOST:
			OnLostConnection();
			break;

		case ID_CONNECTION_ATTEMPT_FAILED:
		case ID_ALREADY_CONNECTED:
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			OnConnectionAttemptFailed(packetID);
			break;

		case ID_IP_RECENTLY_CONNECTED:
			LogInfo("Connection attempt rejected because it was too soon after the last connection");
			break;
		}
	}
}


void XSocketImpl::OnReceiveMessageAsync(const MessageConstPtr& msg)
{
	byte packetID = msg->GetMessageID();

	// If this is a user's packet, forward it to them
	if (packetID >= ID_USER_PACKET_ENUM)
	{
		if (m_listener)
		{
			m_listener->OnMessageReceivedAsync(this, msg->GetData(), msg->GetSize());
		}
	}
}


const PeerPtr& XSocketImpl::GetPeer() 
{ 
	return m_peer; 
}


PeerConstPtr XSocketImpl::GetPeer() const 
{ 
	return m_peer; 
}


void XSocketImpl::SetPeer(const PeerPtr& peer)
{
	m_peer = peer;

	RakNet::ConnectionState peerState = m_peer->GetConnectionState(m_raknetAddress);
	if (peerState == RakNet::IS_CONNECTED)
	{
		m_status = Connected;
	}
}


PeerID XSocketImpl::GetPeerID() const
{
	return m_peerID;
}


RakNet::SystemAddress XSocketImpl::GetAddress() const
{
	return m_raknetAddress;
}


void XSocketImpl::SetAddress(const RakNet::SystemAddress& newAddress)
{
	m_raknetAddress = newAddress;
	m_address = m_raknetAddress.ToString(false);
	m_port = m_raknetAddress.GetPort();
}


const RakNet::RakNetGUID& XSocketImpl::GetRakNetGUID() const
{
	return m_raknetGuid;
}


void XSocketImpl::SetRakNetGUID(const RakNet::RakNetGUID& guid)
{
	m_raknetGuid = guid;
}


void XSocketImpl::SetRegistrationReceipt(const ReceiptPtr& receipt)
{
	m_receipt = receipt;
}


void XSocketImpl::OnConnected()
{
	m_status = Connected;
	if (m_listener)
	{
		m_listener->OnConnected(this);
	}
}


void XSocketImpl::OnLostConnection()
{
	m_status = Disconnected;
	if (m_listener)
	{
		m_listener->OnDisconnected(this);
	}
}


void XSocketImpl::OnConnectionAttemptFailed(byte failureID)
{
	XSocketListener::FailureReason reason = XSocketListener::Failure_Unknown;
	switch (failureID)
	{
	case ID_CONNECTION_ATTEMPT_FAILED:
		reason = XSocketListener::Failure_CannotConnect;
		break;

	case ID_ALREADY_CONNECTED:
		reason = XSocketListener::Failure_AlreadyConnected;
		break;

	case ID_NO_FREE_INCOMING_CONNECTIONS:
		reason = XSocketListener::Failure_MaxConnectionsReached;
		break;
	}

	m_status = Disconnected;
	if (m_listener)
	{
		m_listener->OnConnectionFailed(this, reason);
	}
}

XTOOLS_NAMESPACE_END
