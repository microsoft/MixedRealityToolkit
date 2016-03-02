//////////////////////////////////////////////////////////////////////////
// XSocketManagerImpl.h
//
// Implementation of the XSocketManager interface.  Allows us to limit the 
// exposure of the RakNet APIs to code outside the common library
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>
#include <string>
#include <map>
#include <chrono>
#include "MessageQueue.h"

XTOOLS_NAMESPACE_BEGIN

class XSocketImpl;

class XSocketManagerImpl : public XSocketManager
{
	XTOOLS_REFLECTION_DECLARE(XSocketManagerImpl)

public:
	XSocketManagerImpl();

	//////////////////////////////////////////////////////////////////////////
	// ConnectionMgr Functions:

	// Opens a connection to the given remote host.  The 'remoteName' can be either and IP address or a machine name.  
	// Mutex locked
	virtual XSocketPtr OpenConnection(const std::string& remoteName, uint16 remotePort) XTOVERRIDE;

	// Opens a port on the local machine to listen for incoming connections.  When a remote client connects,
	// a XSocket instance is created for it and passed to the given listener.  Only 'maxConnections' clients
	// can connect at the same time.  
	// Mutex locked
	virtual ReceiptPtr AcceptConnections(uint16 port, uint16 maxConnections, IncomingXSocketListener* listener) XTOVERRIDE;

	virtual std::string GetLocalMachineAddress() XTOVERRIDE;

	// Processes any network messages that have arrived since the last call to Update().  
	// When called, it will process any connection or disconnection notifications, then
	// forward on any packets to the appropriate Connection object
	virtual void Update() XTOVERRIDE;

	// Add an interceptor, which will intercept an incoming message on the network thread and handle it before it is sent
	// to the main thread
	void AddInterceptor(const MessageInterceptorPtr& interceptor);

	void RemoveInterceptor(const MessageInterceptorPtr& interceptor);

private:
	struct PeerConnection;
	DECLARE_PTR(PeerConnection)

	struct ListenerInfo
	{
		IncomingXSocketListener*	m_listener;
		PeerID						m_peerID;
	};

	struct ClosingPeer
	{
		ClosingPeer() {}
		ClosingPeer(std::chrono::high_resolution_clock::time_point shutdownTime, const PeerPtr& peer)
			: m_shutdownStartTime(shutdownTime)
			, m_peer(peer) {}

		std::chrono::high_resolution_clock::time_point	m_shutdownStartTime;
		PeerPtr                                         m_peer;
	};

	// Callback for when a Connection object is not longer referenced.  Called from ConnectionImpl.
	// Mutex locked
	void OnConnectionDestroyed(XSocketImpl* closingConnection);

	void UnregisterConnectionListener(ListenerInfo listenerDesc);

	// Maps peerIDs to peer connections
	std::map<uint32, PeerConnectionPtr>	m_peers;

	// List of peers that are currently shutting down and we're waiting for them to finish sending
	// their last messages before deleting them.  
	std::list<ClosingPeer>		m_closingPeers;

	SocketID						m_nextSocketID;
	uint32							m_nextPeerID;

	MessageQueue					m_messageQueue;
};

DECLARE_PTR(XSocketManagerImpl)

XTOOLS_NAMESPACE_END