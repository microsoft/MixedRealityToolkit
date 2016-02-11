//////////////////////////////////////////////////////////////////////////
// TunnelConnection.h
//
// Implementation of the NetworkConnection interface that  
// tunnels all incoming and outgoing traffic through a proxy.  
// Wraps another NetworkConnection that it uses for communication
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>

XTOOLS_NAMESPACE_BEGIN

class TunnelConnection : public NetworkConnection, public NetworkConnectionListener
{
public:
	TunnelConnection(const NetworkConnectionPtr& connection);
	virtual ~TunnelConnection();

	// Get unique ID from underlying Connection object and pass back to user, or 0 if no current connection
	virtual ConnectionGUID GetConnectionGUID() const XTOVERRIDE;

	/// NetworkConnection Functions:
	virtual bool IsConnected() const XTOVERRIDE;

	// Send the given message with specific settings.  Set releaseMessage to true to release the NetworkMessage
	// and return it to the message pool for later reuse.  
	virtual void Send(const NetworkOutMessagePtr& msg, MessagePriority priority, MessageReliability reliability, MessageChannel channel, bool releaseMessage) XTOVERRIDE;

	virtual void SendTo(const UserPtr& user, ClientRole deviceRole, const NetworkOutMessagePtr& msg, MessagePriority priority, MessageReliability reliability, MessageChannel channel, bool releaseMessage) XTOVERRIDE;

	// Instruct the recipient to sent this messages on to all other connected peers
	virtual void Broadcast(const NetworkOutMessagePtr& msg, MessagePriority priority, MessageReliability reliability, MessageChannel channel, bool releaseMessage) XTOVERRIDE;

	virtual void AddListener(byte messageType, NetworkConnectionListener* newListener) XTOVERRIDE;

	virtual void RemoveListener(byte messageType, NetworkConnectionListener* oldListener) XTOVERRIDE;

	// Same as RegisterCallback, but the callbacks will be called on the network thread as soon as they arrive,
	// rather than being queued and processed later on the main thread
	virtual bool RegisterAsyncCallback(byte messageType, NetworkConnectionListener* cb) XTOVERRIDE;

	// Manually unregister for async callbacks
	virtual void UnregisterAsyncCallback(byte messageType) XTOVERRIDE;

	virtual NetworkOutMessagePtr CreateMessage(byte messageType) XTOVERRIDE;

	virtual void ReturnMessage(const NetworkOutMessagePtr& msg) XTOVERRIDE;

	virtual void Disconnect() XTOVERRIDE;

	virtual XStringPtr GetRemoteAddress() const XTOVERRIDE;

	virtual const XSocketPtr& GetSocket() const XTOVERRIDE;
	virtual void SetSocket(const XSocketPtr& connection) XTOVERRIDE;

private:
	/// NetworkConnectionListener Functions:

	virtual void OnConnected(const NetworkConnectionPtr& connection) XTOVERRIDE;
	virtual void OnConnectFailed(const NetworkConnectionPtr& connection) XTOVERRIDE;
	virtual void OnDisconnected(const NetworkConnectionPtr& connection) XTOVERRIDE;
	virtual void OnMessageReceived(const NetworkConnectionPtr& connection, NetworkInMessage& message) XTOVERRIDE;

	// Notify the listeners that the tunnel is now connected
	void NotifyConnected();

	// Notify the listeners that the tunnel is now connected
	void NotifyDisconnected();

	typedef ListenerList<NetworkConnectionListener> ListenerList;
	DECLARE_PTR(ListenerList);

	std::map<byte, ListenerListPtr>		m_listeners;
	NetworkConnectionPtr		        m_netConnection;
	ConnectionGUID						m_connectionGUID;
	scoped_array<byte>					m_messageBuffer;
	uint32								m_messageBufferSize;

	// Is the other end of the tunnel connected to the bridge as well
	bool								m_remoteSystemConnected;
};

DECLARE_PTR(TunnelConnection)

XTOOLS_NAMESPACE_END