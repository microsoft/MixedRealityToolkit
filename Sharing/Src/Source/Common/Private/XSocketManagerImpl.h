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
#include "NetworkThreadCommands.h"
#include "Utils/TypedLFQueue.h"

XTOOLS_NAMESPACE_BEGIN

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

	// Open a port to listen for discovery connections and respond with a description of this machine
	virtual ReceiptPtr AcceptDiscoveryPings(uint16 port, SystemRole role) XTOVERRIDE;

	virtual std::string GetLocalAddressForRemoteClient(const XSocketPtr& socket) const XTOVERRIDE;

	// Get a reference to an event that will get signalled when a message has arrived.  
	virtual Event& GetMessageArrivedEvent() XTOVERRIDE;

	// Processes any network messages that have arrived since the last call to Update().  
	// When called, it will process any connection or disconnection notifications, then
	// forward on any packets to the appropriate Connection object
	virtual void Update() XTOVERRIDE;

private:

	struct PeerConnection;
	DECLARE_PTR(PeerConnection)

	struct ClosingPeer;

	void SendCommandToNetworkThread(const CommandPtr& command);

	// Callback for when a socket is not longer referenced.
	// Mutex locked
	void CloseConnection(XSocketImpl* closingSocket);

	void UnregisterConnectionListener(PeerID peerID);

	void ThreadFunc();

	// Process commands sent from the main thread to the network thread via the commandQueue
	void ProcessCommands();

	void UpdatePeers();

	// Process incoming packets from the network
	void ProcessMessages();

	void ProcessOpenCommand(const CommandPtr& openCommand);
	void ProcessAcceptCommand(const CommandPtr& acceptCommand);
	void ProcessDiscoveryResponseCommand(const CommandPtr& discoveryCommand);
	void ProcessRemovePeerReferenceCommand(const CommandPtr& command);

	void SendConnectionFailedMessage(const std::string& address, uint16 port, PeerID peerID);
	bool SendMessageToMainThread(const MessagePtr& msg);

	std::list<XSocketImpl*> m_connectingSockets;

	// Maps peers to callbacks that should be notified when a new incoming connection is made
	std::map<PeerID, IncomingXSocketListener*> m_incomingConnectionListeners;

	// List of the active sockets, mapped by their RakNetGUID
	std::map<RakNet::RakNetGUID, XSocketImpl*> m_sockets;


	// Maps peerIDs to peer connections.  Only used on the network thread
	std::map<PeerID, PeerConnectionPtr>	m_peers;
	Mutex								m_peerMutex;

	// List of peers that are currently shutting down and we're waiting for them to finish sending
	// their last messages before deleting them.  
	std::list<ClosingPeer>		m_closingPeers;

	TypedLFQueue<CommandPtr>	m_commandQueue;
	TypedLFQueue<MessagePtr>	m_messageQueue;

	RakNet::RakNetSmartPtr<RakNet::SignaledEvent>		m_networkThreadEvent;

	MemberFuncThreadPtr			m_networkThread;
	volatile int				m_stopping;

	// If the main lock-free queue is full, store the messages here to be sent later
	std::queue<MessagePtr>		m_backupQueue;

	Event						m_messagesAvailableEvent;
};

DECLARE_PTR(XSocketManagerImpl)

XTOOLS_NAMESPACE_END