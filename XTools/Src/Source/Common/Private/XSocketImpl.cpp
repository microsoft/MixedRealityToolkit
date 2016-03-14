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


XSocketImpl::XSocketImpl(XSocketManagerImpl* manager, SocketID id, const PeerPtr& peer, const RakNet::SystemAddress& address, RakNet::RakNetGUID guid)
	: m_manager(manager)
	, m_id(id)
	, m_peer(peer)
	, m_listener(NULL)
	, m_address(address)
	, m_raknetGuid(guid)
	, m_status(Connecting)
{
	// XSockets are only created when:
	// 1) The user asks to open a connection, or
	// 2) A new connection is received
	// RakNet does not properly handle pending connections, so set the status as connecting unless
	// the RakNet status is connected

	RakNet::ConnectionState peerState = m_peer->GetConnectionState(m_address);
	if (peerState == RakNet::IS_CONNECTED)
	{
		m_status = Connected;
	}
}


SocketID XSocketImpl::GetID() const
{
	return m_id;
}


void XSocketImpl::Send(const byte* message, uint32 messageSize, MessagePriority priority, MessageReliability reliability, MessageChannel channel)
{
	XTASSERT(m_status == Connected);

	uint32 messageID = m_peer->Send(reinterpret_cast<const char*>(message), messageSize, (PacketPriority)priority, (PacketReliability)reliability, (char)channel, m_address, false);
	if (messageID == 0)
	{
		XTASSERT(false);
		LogWarning("Failed to send message to %s", m_address.ToString(true));
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


std::string XSocketImpl::GetRemoteSystemName() const
{
	return m_address.ToString(false);
}


bool XSocketImpl::OnReceiveMessage(const MessageConstPtr& msg)
{
	bool bConsumedPacket = false;

	byte packetID = msg->GetMessageID();

	// If this is a user's packet, forward it to them
	if (packetID >= ID_USER_PACKET_ENUM)
	{
		if (m_listener)
		{
			m_listener->OnMessageReceived(this, msg->GetData(), msg->GetSize());
			bConsumedPacket = true;
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
			bConsumedPacket = true;
			break;

		case ID_DISCONNECTION_NOTIFICATION:
			OnLostConnection();
			bConsumedPacket = true;
			break;

		case ID_CONNECTION_LOST:
			OnLostConnection();
			bConsumedPacket = true;
			break;

		case ID_CONNECTION_ATTEMPT_FAILED:
		case ID_ALREADY_CONNECTED:
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			OnConnectionAttemptFailed(packetID);
			bConsumedPacket = true;
			break;

		case ID_IP_RECENTLY_CONNECTED:
			LogInfo("Connection attempt rejected because it was too soon after the last connection");
			break;
		}
	}
	
	return bConsumedPacket;
}


const PeerPtr& XSocketImpl::GetPeer() 
{ 
	return m_peer; 
}


PeerConstPtr XSocketImpl::GetPeer() const 
{ 
	return m_peer; 
}


RakNet::SystemAddress XSocketImpl::GetAddress() const 
{ 
	return m_address; 
}


void XSocketImpl::SetAddress(const RakNet::SystemAddress& newAddress)
{
	m_address = newAddress;
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


XSocketManagerImpl* XSocketImpl::GetSocketManager()
{
	return m_manager;
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
