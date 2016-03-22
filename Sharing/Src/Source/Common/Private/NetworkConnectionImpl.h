//////////////////////////////////////////////////////////////////////////
// NetworkConnectionImpl.h
//
// Implementation of the NetworkConnection interface
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <Public/NetworkConnection.h>
#include "NetworkMessagePool.h"
#include <map>

XTOOLS_NAMESPACE_BEGIN

class NetworkConnectionImpl : public NetworkConnection, public XSocketListener
{
public:
	NetworkConnectionImpl(const NetworkMessagePoolPtr& messagePool);

	virtual ConnectionGUID			GetConnectionGUID() const XTOVERRIDE;

	virtual bool					IsConnected() const XTOVERRIDE;

	virtual void					Send(const NetworkOutMessagePtr& msg, MessagePriority priority, MessageReliability reliability, MessageChannel channel, bool releaseMessage) XTOVERRIDE;

	virtual void					SendTo(const UserPtr& user, ClientRole deviceRole, const NetworkOutMessagePtr& msg, MessagePriority priority, MessageReliability reliability, MessageChannel channel, bool releaseMessage) XTOVERRIDE;

	virtual void					Broadcast(const NetworkOutMessagePtr& msg, MessagePriority priority, MessageReliability reliability, MessageChannel channel, bool releaseMessage) XTOVERRIDE;

	virtual void					AddListener(byte messageType, NetworkConnectionListener* newListener) XTOVERRIDE;

	virtual void					RemoveListener(byte messageType, NetworkConnectionListener* oldListener) XTOVERRIDE;

	virtual void					AddListenerAsync(byte messageType, NetworkConnectionListener* newListener) XTOVERRIDE;

	virtual void					RemoveListenerAsync(byte messageType, NetworkConnectionListener* oldListener) XTOVERRIDE;
 
	virtual NetworkOutMessagePtr	CreateMessage(byte messageType) XTOVERRIDE;
 
	virtual void					ReturnMessage(const NetworkOutMessagePtr& msg) XTOVERRIDE;

	virtual void					Disconnect() XTOVERRIDE;

	virtual const XSocketPtr&		GetSocket() const XTOVERRIDE;

	virtual void					SetSocket(const XSocketPtr& socket) XTOVERRIDE;

	virtual XStringPtr				GetRemoteAddress() const XTOVERRIDE;

	/// Local Functions:

	// Get the connection this class wraps, if any
	XSocketPtr						GetConnection() const;

	const NetworkMessagePoolPtr&	GetMessagePool();

protected:
	/// XSocketListener Functions:
	virtual void					OnConnected(const XSocketPtr& connection) XTOVERRIDE;
	virtual void					OnConnectionFailed(const XSocketPtr& connection, FailureReason reason) XTOVERRIDE;
	virtual void					OnDisconnected(const XSocketPtr& connection) XTOVERRIDE;
	virtual void					OnMessageReceived(const XSocketPtr& connection, const byte* message, uint32 messageLength) XTOVERRIDE;
	virtual void					OnMessageReceivedAsync(const XSocketPtr& socket, const byte* message, uint32 messageLength) XTOVERRIDE;

	typedef ListenerList<NetworkConnectionListener> ListenerList;
	DECLARE_PTR(ListenerList);

	std::map<byte, ListenerListPtr>	m_listeners;
	std::map<byte, ListenerListPtr>	m_asyncListeners;

	XSocketPtr						m_socket;
	ReceiptPtr						m_listenerReceipt;
	NetworkMessagePoolPtr			m_messagePool;
	ConnectionGUID					m_connectionGUID;
	scoped_array<byte>				m_messageBuffer;
	uint32							m_messageBufferSize;

	Mutex							m_asyncListMutex;
};

DECLARE_PTR(NetworkConnectionImpl)

XTOOLS_NAMESPACE_END