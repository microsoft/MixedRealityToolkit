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
#include "TypedLFQueue.h"
#include "NetworkThreadCommands.h"

XTOOLS_NAMESPACE_BEGIN

typedef uint32 PeerID;
const PeerID kInvalidPeerID = 0xFFFFFFFF;

class XSocketImpl;

class XSocketManagerImpl : public XSocketManager
{
	XTOOLS_REFLECTION_DECLARE(XSocketManagerImpl)

public:
	XSocketManagerImpl();
	virtual ~XSocketManagerImpl();

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

	virtual std::string GetLocalAddressForRemoteClient(const XSocketPtr& socket) const XTOVERRIDE;

	// Processes any network messages that have arrived since the last call to Update().  
	// When called, it will process any connection or disconnection notifications, then
	// forward on any packets to the appropriate Connection object
	virtual void Update() XTOVERRIDE;

private:

	struct PeerConnection;
	DECLARE_PTR(PeerConnection)

	struct ClosingPeer
	{
		ClosingPeer() {}
		ClosingPeer(std::chrono::high_resolution_clock::time_point shutdownTime, const PeerPtr& peer)
			: m_shutdownStartTime(shutdownTime)
			, m_peer(peer) {}

		std::chrono::high_resolution_clock::time_point	m_shutdownStartTime;
		PeerPtr                                         m_peer;
	};

	void SendCommandToNetworkThread(const CommandPtr& command);

	// Callback for when a socket is not longer referenced.
	// Mutex locked
	void CloseConnection(SocketID socketID);

	void UnregisterConnectionListener(IncomingXSocketListener*	listener, PeerID peerID);

	void ThreadFunc();

	// Process commands sent from the main thread to the network thread via the commandQueue
	void ProcessCommands();

	// Process incoming packets from the network
	void ProcessMessages();

	void ProcessOpenCommand(const OpenCommandPtr& openCommand);
	void ProcessAcceptCommand(const AcceptCommandPtr& acceptCommand);
	void SendConnectionFailedMessage(SocketID socketID);
	bool SendMessageToMainThread(const MessagePtr& msg);


	

	// Maps Socket
	std::map<SocketID, XSocketImpl*> m_sockets;

	// Maps peers to callbacks that should be notified when a new incoming connection is made
	std::map<PeerID, IncomingXSocketListener*> m_incomingConnectionListeners;

	// Maps peerIDs to peer connections.  Only used on the network thread
	std::map<PeerID, PeerConnectionPtr>	m_peers;
	Mutex								m_peerMutex;

	// List of peers that are currently shutting down and we're waiting for them to finish sending
	// their last messages before deleting them.  
	std::list<ClosingPeer>		m_closingPeers;

	TypedLFQueue<CommandPtr>	m_commandQueue;
	TypedLFQueue<MessagePtr>	m_messageQueue;

	Event						m_networkThreadEvent;

	MemberFuncThreadPtr			m_networkThread;
	volatile int				m_stopping;

	Mutex						m_mutex;

	// If the main lock-free queue is full, store the messages here to be sent later
	std::queue<MessagePtr>		m_backupQueue;
};

DECLARE_PTR(XSocketManagerImpl)

XTOOLS_NAMESPACE_END