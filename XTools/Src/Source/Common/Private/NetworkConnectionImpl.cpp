//////////////////////////////////////////////////////////////////////////
// NetworkConnectionImpl.cpp
//
// Implementation of the NetworkConnection interface
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NetworkConnectionImpl.h"
#include "NetworkInMessageImpl.h"
#include <random>

XTOOLS_NAMESPACE_BEGIN

static_assert(static_cast<int>(MessageID::Start) == static_cast<int>(ID_USER_PACKET_ENUM), "XTools internal MessageID::Start must match RakNet's ID_USER_PACKET_ENUM");


NetworkConnectionImpl::NetworkConnectionImpl(const NetworkMessagePoolPtr& messagePool)
	: m_messagePool(messagePool)
	, m_messageBuffer(new byte[kDefaultMessageBufferSize])
	, m_messageBufferSize(kDefaultMessageBufferSize)
{
	std::random_device randomGenerator;
	m_connectionGUID = ((uint64)randomGenerator() << 32) + (uint64)randomGenerator();
}


ConnectionGUID NetworkConnectionImpl::GetConnectionGUID() const
{
	return m_connectionGUID;
}


bool NetworkConnectionImpl::IsConnected() const
{
	return (m_socket != NULL && m_socket->GetStatus() == XSocket::Connected);
}


void NetworkConnectionImpl::Send(const NetworkOutMessagePtr& msg, MessagePriority priority, MessageReliability reliability, MessageChannel channel, bool releaseMessage)
{
	if (IsConnected())
	{
		m_socket->Send(msg->GetData(), msg->GetSize(), priority, reliability, channel);
	}
	else
	{
		LogError("Trying to send a message to a remote host that is not connected");
	}

	if (releaseMessage)
	{
		m_messagePool->ReturnMessage(msg);
	}
}


void NetworkConnectionImpl::SendTo(const UserPtr& user, ClientRole deviceRole, const NetworkOutMessagePtr& msg, MessagePriority priority, MessageReliability reliability, MessageChannel channel, bool releaseMessage)
{
	if (IsConnected())
	{
		// Append a sendto header to the outgoing message

		const uint32 msgSize = msg->GetSize();

		const uint32 sendPacketSize = msgSize + sizeof(SendToNetworkHeader);
		if (m_messageBufferSize < sendPacketSize)
		{
			m_messageBuffer = new byte[sendPacketSize];
		}

		// Write a header onto the front of the buffer
		SendToNetworkHeader* header = reinterpret_cast<SendToNetworkHeader*>(m_messageBuffer.get());
		header->m_messageID = MessageID::SendTo;
		header->m_priority = priority;
		header->m_reliability = reliability;
		header->m_channel = channel;
		header->m_userID = user->GetID();
		header->m_deviceRole = deviceRole;

		// Write the message onto the rest of the buffer
		byte* payload = m_messageBuffer.get() + sizeof(SendToNetworkHeader);
		memcpy(payload, msg->GetData(), msgSize);

		// Send the constructed packet
		m_socket->Send(m_messageBuffer.get(), sendPacketSize, priority, reliability, channel);
	}
	else
	{
		LogError("Trying to send a message to a remote host that is not connected");
	}

	if (releaseMessage)
	{
		m_messagePool->ReturnMessage(msg);
	}
}


// Instruct the recipient to sent this messages on to all other connected peers
void NetworkConnectionImpl::Broadcast(const NetworkOutMessagePtr& msg, MessagePriority priority, MessageReliability reliability, MessageChannel channel, bool releaseMessage)
{
	if (IsConnected())
	{
		// Append a broadcast header to the outgoing message

		const uint32 msgSize = msg->GetSize();

		const uint32 sendPacketSize = msgSize + sizeof(NetworkHeader);
		if (m_messageBufferSize < sendPacketSize)
		{
			m_messageBuffer = new byte[sendPacketSize];
		}

		// Write a header onto the front of the buffer
		NetworkHeader* header = reinterpret_cast<NetworkHeader*>(m_messageBuffer.get());
		header->m_messageID = MessageID::Broadcast;
		header->m_priority = priority;
		header->m_reliability = reliability;
		header->m_channel = channel;

		// Write the message onto the rest of the buffer
		byte* payload = m_messageBuffer.get() + sizeof(NetworkHeader);
		memcpy(payload, msg->GetData(), msgSize);

		// Send the constructed packet
		m_socket->Send(m_messageBuffer.get(), sendPacketSize, priority, reliability, channel);
	}
	else
	{
		LogError("Trying to send a message to a remote host that is not connected");
	}

	if (releaseMessage)
	{
		m_messagePool->ReturnMessage(msg);
	}
}


void NetworkConnectionImpl::AddListener(byte messageType, NetworkConnectionListener* newListener)
{
	// If the message ID being registered for is outside the valid range, then set it to be
	// StatusOnly messages: the listener will still get connect/disconnect notifications, but will not receive any messages
	if (messageType < MessageID::Start)
	{
		messageType = MessageID::StatusOnly;
	}

	ListenerListPtr list = m_listeners[messageType];
	if (!list)
	{
		list = ListenerList::Create();
		m_listeners[messageType] = list;
	}

	list->AddListener(newListener);
}


void NetworkConnectionImpl::RemoveListener(byte messageType, NetworkConnectionListener* oldListener)
{
	// If the message ID being registered for is outside the valid range, then set it to be
	// StatusOnly messages: the listener will still get connect/disconnect notifications, but will not receive any messages
	if (messageType < MessageID::Start)
	{
		messageType = MessageID::StatusOnly;
	}

	ListenerListPtr list = m_listeners[messageType];
	if (list)
	{
		list->RemoveListener(oldListener);
	}
}


void NetworkConnectionImpl::AddListenerAsync(byte messageType, NetworkConnectionListener* newListener)
{
	// If the message ID being registered for is outside the valid range, then set it to be
	// StatusOnly messages: the listener will still get connect/disconnect notifications, but will not receive any messages
	if (messageType < MessageID::Start)
	{
		messageType = MessageID::StatusOnly;
	}

	{
		ScopedLock lock(m_asyncListMutex);

		ListenerListPtr list = m_asyncListeners[messageType];
		if (!list)
		{
			list = ListenerList::Create();
			m_asyncListeners[messageType] = list;
		}

		list->AddListener(newListener);
	}
}


void NetworkConnectionImpl::RemoveListenerAsync(byte messageType, NetworkConnectionListener* oldListener)
{
	// If the message ID being registered for is outside the valid range, then set it to be
	// StatusOnly messages: the listener will still get connect/disconnect notifications, but will not receive any messages
	if (messageType < MessageID::Start)
	{
		messageType = MessageID::StatusOnly;
	}

	{
		ScopedLock lock(m_asyncListMutex);
		ListenerListPtr list = m_asyncListeners[messageType];
		if (list)
		{
			list->RemoveListener(oldListener);
		}
	}
}


NetworkOutMessagePtr NetworkConnectionImpl::CreateMessage(byte messageType)
{
	NetworkOutMessagePtr newMessage = m_messagePool->AcquireMessage();
	newMessage->Write(messageType);

	return newMessage;
}


void NetworkConnectionImpl::ReturnMessage(const NetworkOutMessagePtr& msg)
{
	XTASSERT(msg);
	m_messagePool->ReturnMessage(msg);
}


void NetworkConnectionImpl::Disconnect()
{
	if (m_socket != NULL && 
		m_socket->GetStatus() != XSocket::Disconnected && 
		m_socket->GetStatus() != XSocket::Disconnecting)
	{
		LogInfo("Intentionally closing connection");
		OnDisconnected(m_socket);
	}
}


XSocketPtr NetworkConnectionImpl::GetConnection() const
{
	return m_socket;
}


XStringPtr NetworkConnectionImpl::GetRemoteAddress() const
{
	if (m_socket)
	{
		return new XString(m_socket->GetRemoteSystemName());
	}
	else
	{
		return nullptr;
	}
}


const XSocketPtr& NetworkConnectionImpl::GetSocket() const
{
	return m_socket;
}


void NetworkConnectionImpl::SetSocket(const XSocketPtr& connection)
{
    if (connection)
    {
		if (m_socket)
		{
			LogInfo("NetworkConnection: Replacing an existing socket with a new one");
		}

        m_socket = connection;
        m_listenerReceipt = m_socket->RegisterListener(this);

        // If this connection is already connected, then trigger an OnConnected callback
        if (m_socket->GetStatus() == XSocket::Connected)
        {
            OnConnected(m_socket);
        }
    }
    else
    {
		if (m_socket)
		{
			if (m_socket->GetStatus() == XSocket::Connected)
			{
				LogInfo("NetworkConnection: Clearing open socket");
				OnDisconnected(m_socket);
			}
			else
			{
				m_listenerReceipt = NULL;
				m_socket = NULL;
			}
		}
    }
}


const NetworkMessagePoolPtr& NetworkConnectionImpl::GetMessagePool()
{
	return m_messagePool;
}


void NetworkConnectionImpl::OnConnected(const XSocketPtr& connection)
{
	XTASSERT(connection == m_socket);
	XT_UNREFERENCED_PARAM(connection);	// Necessary for when XTASSET is not defined

	// Prevent this object from getting destroyed while iterating through callbacks
	NetworkConnectionPtr thisPtr(this);

	for (auto it = m_listeners.begin(); it != m_listeners.end(); ++it)
	{
		it->second->NotifyListeners(&NetworkConnectionListener::OnConnected, thisPtr);
	}

	{
		ScopedLock lock(m_asyncListMutex);
		for (auto it = m_asyncListeners.begin(); it != m_asyncListeners.end(); ++it)
		{
			it->second->NotifyListeners(&NetworkConnectionListener::OnConnected, thisPtr);
		}
	}
}


void NetworkConnectionImpl::OnConnectionFailed(const XSocketPtr& connection, FailureReason)
{
	XTASSERT(connection == m_socket);
	XT_UNREFERENCED_PARAM(connection);	// Necessary for when XTASSET is not defined

	// Prevent this object from getting destroyed while iterating through callbacks
	NetworkConnectionPtr thisPtr(this);

	m_socket = NULL;
	m_listenerReceipt = NULL;

	for (auto it = m_listeners.begin(); it != m_listeners.end(); ++it)
	{
		it->second->NotifyListeners(&NetworkConnectionListener::OnConnectFailed, thisPtr);
	}

	{
		ScopedLock lock(m_asyncListMutex);
		for (auto it = m_asyncListeners.begin(); it != m_asyncListeners.end(); ++it)
		{
			it->second->NotifyListeners(&NetworkConnectionListener::OnConnectFailed, thisPtr);
		}
	}
}


void NetworkConnectionImpl::OnDisconnected(const XSocketPtr& connection)
{
	XTASSERT(connection == m_socket);
	XT_UNREFERENCED_PARAM(connection);	// Necessary for when XTASSET is not defined

	// Prevent this object from getting destroyed while iterating through callbacks
	NetworkConnectionPtr thisPtr(this);

	m_socket = NULL;
	m_listenerReceipt = NULL;

	for (auto it = m_listeners.begin(); it != m_listeners.end(); ++it)
	{
		it->second->NotifyListeners(&NetworkConnectionListener::OnDisconnected, thisPtr);
	}

	{
		ScopedLock lock(m_asyncListMutex);
		for (auto it = m_asyncListeners.begin(); it != m_asyncListeners.end(); ++it)
		{
			it->second->NotifyListeners(&NetworkConnectionListener::OnDisconnected, thisPtr);
		}
	}
}


void NetworkConnectionImpl::OnMessageReceived(const XSocketPtr& connection, const byte* message, uint32 messageLength)
{
	XTASSERT(connection == m_socket);
	XT_UNREFERENCED_PARAM(connection);	// Necessary for when XTASSET is not defined

	auto callbackIter = m_listeners.find(message[0]);
	if (callbackIter != m_listeners.end())
	{
		// Make sure we pass a different message instance to each listener so that they can independently
		// consume content from the message.  Otherwise, two listeners for the same message type will each
		// advance the state of the NetworkInMessageImpl.
		for (int32 i = callbackIter->second->GetListenerCount() - 1; i >= 0; i--)
		{
		// Wrap the message in a NetworkMessage on the stack and call the callback.
		NetworkInMessageImpl msg(message, messageLength);

		// Read the message ID off the front before passing it off to the callback
		msg.ReadByte();

			callbackIter->second->NotifyListener(i, &NetworkConnectionListener::OnMessageReceived, this, msg);
		}
	}
}

void NetworkConnectionImpl::OnMessageReceivedAsync(const XSocketPtr& connection, const byte* message, uint32 messageLength)
{
	ScopedLock lock(m_asyncListMutex);

	XTASSERT(connection == m_socket);
	XT_UNREFERENCED_PARAM(connection);	// Necessary for when XTASSET is not defined

	auto callbackIter = m_asyncListeners.find(message[0]);
	if (callbackIter != m_asyncListeners.end())
	{
		// Make sure we pass a different message instance to each listener so that they can independently
		// consume content from the message.  Otherwise, two listeners for the same message type will each
		// advance the state of the NetworkInMessageImpl.
		for (int32 i = callbackIter->second->GetListenerCount() - 1; i >= 0; i--)
		{
			// Wrap the message in a NetworkMessage on the stack and call the callback.
			NetworkInMessageImpl msg(message, messageLength);

			// Read the message ID off the front before passing it off to the callback
			msg.ReadByte();

			callbackIter->second->NotifyListener(i, &NetworkConnectionListener::OnMessageReceived, this, msg);
		}
	}
}

XTOOLS_NAMESPACE_END
