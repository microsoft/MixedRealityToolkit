//////////////////////////////////////////////////////////////////////////
// XSocketManagerImpl.cpp
//
// 
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XSocketManagerImpl.h"
#include "XSocketImpl.h"
#include "Peer.h"
#include "PacketWrapper.h"
#include "SystemDescription.h"

XTOOLS_NAMESPACE_BEGIN

XTOOLS_REFLECTION_DEFINE(XSocketManager)
.BaseClass<Reflection::XTObject>()
.BaseClass<IUpdateable>();

XTOOLS_REFLECTION_DEFINE(XSocketManagerImpl)
.BaseClass<XSocketManager>();

//////////////////////////////////////////////////////////////////////////
// Implement static XSocketManager methods

// static 
XSocketManagerPtr XSocketManager::Create()
{
	return new XSocketManagerImpl();
}

//static 
IPAddressList XSocketManager::GetLocalMachineAddresses()
{
	IPAddressList addressList;

	// Make a temp peer so we can get this info
	PeerPtr tempPeer = new Peer();

	int numAddresses = tempPeer->GetNumberOfAddresses();
	for (int i = 0; i < numAddresses; ++i)
	{
		addressList.push_back(IPAddressV4(tempPeer->GetLocalIP(i)));
	}

	return addressList;
}


//////////////////////////////////////////////////////////////////////////
// XSocketManagerImpl tweakables

// It is possible that processing the incoming messages progresses at a slower
// pace than the rate of messages coming in.  To prevent deadlock, we set a maximum number
// of messages that can be processed per call to Update().
static const uint32 kMaxMessagesPerUpdate = 200;

// How long (in milliseconds) to allow RakNet to finish flushing messages and stop a peer's
// thread before destroying it
static const uint64 kRakNetShutdownTime = 30;

// Allocate space for the message buffer
static const uint32 kMessageQueueSize = 3 * (1024 * 1024);

// Allocate space for the command buffer
static const uint32 kCommandQueueSize = 1024;

// The maximum number of message to process from a single peer connection per update.  
static const uint32 kMaxIncomingMessages = 50;



//////////////////////////////////////////////////////////////////////////
// XSocketManagerImpl internal types

struct XSocketManagerImpl::PeerConnection : public AtomicRefCounted
{
	explicit PeerConnection(PeerID peerID)
		: m_peer(new Peer(peerID))
		, m_updateBitStream(MAXIMUM_MTU_SIZE)
		, m_referenceCount(0)
	{}

	// Get the unique ID of this peer, assigned at creation
	PeerID GetPeerID() const { return m_peer.get()->GetPeerID(); }

	std::map<RakNet::RakNetGUID, Callback<const MessageConstPtr&> > m_asyncCallbacks;
	PeerPtr										m_peer;
	RakNet::BitStream							m_updateBitStream;
	int32										m_referenceCount;
};

struct XSocketManagerImpl::ClosingPeer
{
	ClosingPeer() {}
	ClosingPeer(std::chrono::high_resolution_clock::time_point shutdownTime, const PeerConnectionPtr& peer)
		: m_shutdownStartTime(shutdownTime)
		, m_peerConnection(peer) {}

	std::chrono::high_resolution_clock::time_point	m_shutdownStartTime;
	PeerConnectionPtr                               m_peerConnection;
};



//////////////////////////////////////////////////////////////////////////
// XSocketManagerImpl implementation

XSocketManagerImpl::XSocketManagerImpl()
	: m_stopping(0)
	, m_commandQueue(kCommandQueueSize)
	, m_messageQueue(kMessageQueueSize)
	, m_networkThreadEvent(new RakNet::SignaledEvent())
	, m_messagesAvailableEvent(false, false)
{
	m_networkThreadEvent->InitEvent();

	// Start a thread to run the update loop. 
	m_networkThread = new MemberFuncThread(&XSocketManagerImpl::ThreadFunc, this);
}


XSocketManagerImpl::~XSocketManagerImpl()
{
	// trigger the thread exit and wait for it...
	m_stopping = 1;
	m_networkThreadEvent->SetEvent();
	m_networkThread->WaitForThreadExit();

	XTASSERT(m_peers.empty());
	XTASSERT(m_closingPeers.empty());


	m_networkThreadEvent->CloseEvent();
}


XSocketPtr XSocketManagerImpl::OpenConnection(const std::string& remoteName, uint16 remotePort)
{
	PROFILE_SCOPE("XSocketManager::OpenConnection");

	XTASSERT(remotePort != 0);

	PeerID peerID = Peer::CreatePeerID();

	XSocketImplPtr newSocket = new XSocketImpl(peerID, remoteName, remotePort);
	newSocket->SetRegistrationReceipt(CreateRegistrationReceipt(XSocketManagerImplPtr(this), &XSocketManagerImpl::CloseConnection, newSocket.get()));

	// Add the new socket to the list of sockets pending connection
	m_connectingSockets.push_back(newSocket.get());

	CommandPtr openCommand = CreateOpenCommand(newSocket);
	SendCommandToNetworkThread(openCommand);

	return newSocket;
}


ReceiptPtr XSocketManagerImpl::AcceptConnections(uint16 port, uint16 maxConnections, IncomingXSocketListener* listener)
{
	PROFILE_SCOPE("XSocketManager::AcceptConnections");

	PeerID peerID = Peer::CreatePeerID();

	if (listener)
	{
		m_incomingConnectionListeners[peerID] = listener;
	}

	CommandPtr acceptCommand = CreateAcceptCommand(peerID, port, maxConnections);
	SendCommandToNetworkThread(acceptCommand);

	return CreateRegistrationReceipt(XSocketManagerImplPtr(this), &XSocketManagerImpl::UnregisterConnectionListener, peerID);
}


ReceiptPtr XSocketManagerImpl::AcceptDiscoveryPings(uint16 port, SystemRole role)
{
	PROFILE_SCOPE("XSocketManager::AcceptDiscoveryPings");

	PeerID peerID = Peer::CreatePeerID();

	CommandPtr acceptCommand = CreateDiscoveryResponseCommand(peerID, port, role);
	SendCommandToNetworkThread(acceptCommand);

	return CreateRegistrationReceipt(XSocketManagerImplPtr(this), &XSocketManagerImpl::UnregisterConnectionListener, peerID);
}


void XSocketManagerImpl::SendCommandToNetworkThread(const CommandPtr& command)
{
	// Ensures that that command is enqueued before this function returns
	while (!m_commandQueue.TryPush(command)) {}

	// Signal the network thread to wake up and process the command
	m_networkThreadEvent->SetEvent();
}


void XSocketManagerImpl::CloseConnection(XSocketImpl* closingSocket)
{
	if (XTVERIFY(closingSocket))
	{
		// Remove the socket from the data structures used by the main thread

		// If the socket has a valid RakNetGUID then is has been connected and is in the m_sockets map
		if (closingSocket->GetRakNetGUID() != RakNet::UNASSIGNED_RAKNET_GUID)
		{
			auto socketItr = m_sockets.find(closingSocket->GetRakNetGUID());
			if (XTVERIFY(socketItr != m_sockets.end()))
			{
				m_sockets.erase(socketItr);
			}
		}
		// Otherwise it is in the m_connectingSockets list
		else
		{
			for (auto socketItr = m_connectingSockets.begin(); socketItr != m_connectingSockets.end(); ++socketItr)
			{
				if (*socketItr == closingSocket)
				{
					m_connectingSockets.erase(socketItr);
					break;
				}
			}
		}

		
		{
			ScopedLock lock(m_peerMutex);

			auto peerIter = m_peers.find(closingSocket->GetPeerID());
			if (peerIter != m_peers.end())
			{
				PeerConnectionPtr currentPeerConnection = peerIter->second;

				// Close the connection that this socket represented, if any
				PeerPtr peer = peerIter->second->m_peer;
				if (peer)
				{
					RakNet::ConnectionState currentState = peer->GetConnectionState(closingSocket->GetRakNetGUID());
					if (currentState == RakNet::ConnectionState::IS_CONNECTED ||
						currentState == RakNet::ConnectionState::IS_CONNECTING ||
						currentState == RakNet::ConnectionState::IS_PENDING)
					{
						peer->CloseConnection(closingSocket->GetRakNetGUID(), true);
					}
				}

				auto callbackItr = currentPeerConnection->m_asyncCallbacks.find(closingSocket->GetRakNetGUID());
				if (callbackItr != currentPeerConnection->m_asyncCallbacks.end())
				{
					currentPeerConnection->m_asyncCallbacks.erase(callbackItr);
				}
			}
		}

		// Send a command to the network thread to remove a reference from the peer.  This is necessary in case the command to create the peer
		// is in the queue and has not been processed yet.  Otherwise the peer would never be closed
		CommandPtr cmd = CreateRemovePeerReferenceCommand(closingSocket->GetPeerID());
		SendCommandToNetworkThread(cmd);
	}
}


void XSocketManagerImpl::UnregisterConnectionListener(PeerID peerID)
{
	{
		// Lock the peer list.  This can block the network thread
		ScopedLock lock(m_peerMutex);

		// NOTE: if the peer failed during creation on the network thread then it will not be in this list
		auto iter = m_peers.find(peerID);
		if (iter != m_peers.end())
		{
			PeerConnectionPtr peerConnection = iter->second;

			// Set the max incoming connections to zero to stop accepting new connections
			peerConnection->m_peer->SetMaximumIncomingConnections(0);

			auto incomingListenerItr = m_incomingConnectionListeners.find(peerID);
			if (incomingListenerItr != m_incomingConnectionListeners.end())
			{
				m_incomingConnectionListeners.erase(incomingListenerItr);
			}
		}
	}

	// Send a command to the network thread to remove a reference from the peer.  This is necessary in case the command to create the peer
	// is in the queue and has not been processed yet.  Otherwise the peer would never be closed
	CommandPtr cmd = CreateRemovePeerReferenceCommand(peerID);
	SendCommandToNetworkThread(cmd);
}


void XSocketManagerImpl::Update()
{
	PROFILE_FUNCTION();

	uint32 messagesProcessed = 0;

	MessagePtr msg;
	while (m_messageQueue.TryPop(msg))
	{
		if (XTVERIFY(msg->IsValid()))
		{
			const byte packetID = msg->GetMessageID();

			// First check to see if this is a new incoming connection
			if (packetID == ID_NEW_INCOMING_CONNECTION)
			{
				// Create a socket object to represent this connection.  Note that the main thread will trigger the callback
				// to listeners to let them know about the new connection.  
				XSocketImplPtr newSocket = new XSocketImpl(msg->GetPeerID(), std::string(), 0);
				newSocket->SetAddress(msg->GetSystemAddress());
				newSocket->SetRakNetGUID(msg->GetRakNetGUID());
				newSocket->SetRegistrationReceipt(CreateRegistrationReceipt(XSocketManagerImplPtr(this), &XSocketManagerImpl::CloseConnection, newSocket.get()));

				{
					ScopedLock lock(m_peerMutex);
					auto iter = m_peers.find(msg->GetPeerID());
					if (iter != m_peers.end())
					{
						PeerConnectionPtr peerConnection = iter->second;

						newSocket->SetPeer(peerConnection->m_peer);

						peerConnection->m_asyncCallbacks[msg->GetRakNetGUID()] = CreateCallback(newSocket.get(), &XSocketImpl::OnReceiveMessageAsync);

						++peerConnection->m_referenceCount;
					}
				}

				m_sockets[msg->GetRakNetGUID()] = newSocket.get();

				// Notify the listener that a new connection has been made
				IncomingXSocketListener* listener = nullptr;

				auto listenerItr = m_incomingConnectionListeners.find(msg->GetPeerID());
				if (listenerItr != m_incomingConnectionListeners.end())
				{
					listener = listenerItr->second;
				}

				if (listener)
				{
					listener->OnNewConnection(newSocket);
				}
				else
				{
					LogError("Accepted connection request, but have no listener");
				}
			}
			// Otherwise, find the Connection this packet is meant for and pass it off 
			else 
			{
				XSocketImplPtr socketForMessage = nullptr;

				auto socketItr = m_sockets.find(msg->GetRakNetGUID());
				if (socketItr != m_sockets.end())
				{
					socketForMessage = socketItr->second;
				}
				else
				{
					// Find the socket in the list of pending sockets 
					for (auto itr = m_connectingSockets.begin(); itr != m_connectingSockets.end(); ++itr)
					{
						XSocketImpl* socket = (*itr);
						if (socket->GetPeerID() == msg->GetPeerID())
						{
							// Set the socket's connection data

							// After the connection is made, the address is often updated to be more accurate, so capture that
							socket->SetAddress(msg->GetSystemAddress());

							// We did not have a valid RakNetGUID for this connection until now, so reset the mapping 
							// for this socket to its guid
							socket->SetRakNetGUID(msg->GetRakNetGUID());

							{
								ScopedLock lock(m_peerMutex);
								auto iter = m_peers.find(msg->GetPeerID());
								if (iter != m_peers.end())
								{
									PeerConnectionPtr peerConnection = iter->second;

									socket->SetPeer(peerConnection->m_peer.get());

									peerConnection->m_asyncCallbacks[msg->GetRakNetGUID()] = CreateCallback(socket, &XSocketImpl::OnReceiveMessageAsync);
								}
							}

							// Set the found socket as the socket to receive this message
							socketForMessage = socket;

							// Add the socket to the main socket map if it has a valid GUID.
							// If it doesn't that means it was never able to establish a successful connection
							if (msg->GetRakNetGUID() != RakNet::UNASSIGNED_RAKNET_GUID)
							{
								m_sockets[msg->GetRakNetGUID()] = socket;
							}
							
							// Remove the socket from the list of connecting sockets, this one is now connected
							m_connectingSockets.erase(itr);

							break;
						}
					}
				}

				if (socketForMessage != nullptr)
				{
					socketForMessage->OnReceiveMessage(msg);
				}
			}

			++messagesProcessed;

			// If we've hit the limit on the maximum number of messages to process per peer per update, then go on to the next peer
			if (messagesProcessed >= kMaxMessagesPerUpdate)
			{
				break;
			}
		}
	}
}


std::string XSocketManagerImpl::GetLocalAddressForRemoteClient(const XSocketPtr& socket) const
{
	XSocketImpl* socketImpl = reflection_cast<XSocketImpl>(socket);
	if (socketImpl)
	{
		
		RakNet::SystemAddress address = socketImpl->GetPeer()->GetExternalID(socketImpl->GetAddress());
		return address.ToString(false);
	}
	else
	{
		return std::string();
	}
}


Event& XSocketManagerImpl::GetMessageArrivedEvent()
{
	return m_messagesAvailableEvent;
}


void XSocketManagerImpl::ThreadFunc()
{
	while (m_stopping == 0)
	{
		ProcessCommands();
		UpdatePeers();
		ProcessMessages();
		m_networkThreadEvent->WaitOnEvent(10);
	}

	ScopedLock lock(m_peerMutex);
	m_peers.clear();
	m_closingPeers.clear();
}


void XSocketManagerImpl::ProcessCommands()
{
	CommandPtr newCommand;

	while (m_commandQueue.TryPop(newCommand))
	{
		if (newCommand->GetType() == Command::Open)
		{
			ProcessOpenCommand(newCommand);
		}
		else if (newCommand->GetType() == Command::Accept)
		{
			ProcessAcceptCommand(newCommand);
		}
		else if (newCommand->GetType() == Command::DiscoveryResponse)
		{
			ProcessDiscoveryResponseCommand(newCommand);
		}
		else if (newCommand->GetType() == Command::RemovePeerReference)
		{
			ProcessRemovePeerReferenceCommand(newCommand);
		}
	}
}


void XSocketManagerImpl::UpdatePeers()
{
	ScopedLock lock(m_peerMutex);
	for (auto peerItr = m_peers.begin(); peerItr != m_peers.end(); ++peerItr)
	{
		PeerConnectionPtr peerConnection = peerItr->second;
		PeerPtr peer = peerConnection->m_peer;

		peer->RunUpdateCycle(peerConnection->m_updateBitStream);
	}

	for (auto peerItr = m_closingPeers.begin(); peerItr != m_closingPeers.end(); ++peerItr)
	{
		PeerConnectionPtr peerConnection = peerItr->m_peerConnection;
		PeerPtr peer = peerConnection->m_peer;

		peer->RunUpdateCycle(peerConnection->m_updateBitStream);
	}
}


void XSocketManagerImpl::ProcessMessages()
{
	// Send the messages that did not fit in the queue last update
	while (!m_backupQueue.empty())
	{
		MessagePtr msg = m_backupQueue.front();
		if (m_messageQueue.TryPush(msg))
		{
			// Message put on the lfqueue, clear it from the backup buffer
			m_backupQueue.pop();
		}
		else
		{
			// Buffer is still full.  Exit the function and try again later after the main thread has had
			// more time to consume the packets.  
			return;
		}
	}

	ScopedLock lock(m_peerMutex);

	// Check to see if the time is up for any peers that have been shutting down.
	// Note that the oldest peers will be at the front, so loop until the list is empty
	// or you hit one whose time is not yet up.
	auto currentTime = std::chrono::high_resolution_clock::now();
	while (!m_closingPeers.empty())
	{
		uint64 timeDelta = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_closingPeers.front().m_shutdownStartTime).count();

		if (timeDelta < kRakNetShutdownTime)
		{
			break;
		}

		m_closingPeers.erase(m_closingPeers.begin());
	}

	// Loop through all the peers
	for (auto peerItr = m_peers.begin(); peerItr != m_peers.end(); ++peerItr)
	{
		PeerConnectionPtr peerConnection = peerItr->second;
		PeerPtr peer = peerItr->second->m_peer;

		uint32 peerMessagesProcessed = 0;

		// Loop through all the packets that have arrived on this peer since the last update.
		for (PacketWrapper packet(peer, peer->Receive()); packet.IsValid() && peerMessagesProcessed < kMaxIncomingMessages; packet = peer->Receive())
		{
			// Don't push empty packets.  This shouldn't happen anyway
			if (packet->length > 0)
			{
				// Create a message from this packet
				MessagePtr msg = new Message();
				msg->SetSystemAddress(packet->systemAddress);
				msg->SetRakNetGUID(packet->guid);
				msg->SetPeerID(peerConnection->GetPeerID());
				msg->SetData(packet->data, packet->length);

				// Check to see if we have an async callback for this packet, and if so call it
				auto callbackItr = peerConnection->m_asyncCallbacks.find(packet->guid);
				if (callbackItr != peerConnection->m_asyncCallbacks.end())
				{
					if (callbackItr->second.IsBound())
					{
						callbackItr->second.Call(msg);
					}
				}
				
				// Finally stuff the message in the message queue for delivery to the main thread
				SendMessageToMainThread(msg);

				// Set the event to notify any sleeping threads that a message is available to process
				m_messagesAvailableEvent.Set();

				// Increment the number of messages from this peer that have been processed, so that we can
				// prevent one peer from sending so much traffic that other peers never get their messages read
				++peerMessagesProcessed;
			}
		}
	}
}


void XSocketManagerImpl::ProcessOpenCommand(const CommandPtr& openCommand)
{
	PROFILE_SCOPE("ProcessOpenCommand");

	using namespace RakNet;

	PeerConnectionPtr peerConnection;
	{
		ScopedProfile peerCreationProfile("new PeerConnection");
		peerConnection = new PeerConnection(openCommand->GetPeerID());
	}
	SocketDescriptor sd;
	sd.socketFamily = AF_INET; // IPV4

	// Startup the peer interface.  Will fail if we already have a peer interface for this host
	StartupResult startupResult = peerConnection->m_peer->Startup(1, &sd, 1, m_networkThreadEvent);
	if (startupResult != RAKNET_STARTED)
	{
		LogError("Failed to create connection object for %s.  Error code: %u", openCommand->GetAddress().c_str(), startupResult);
		SendConnectionFailedMessage(openCommand->GetAddress(), openCommand->GetPort(), openCommand->GetPeerID());
		return;
	}

	// Set the connection to time out after the given amount of time
	peerConnection->m_peer->SetTimeoutTime(kConnectionTimeoutMS, UNASSIGNED_SYSTEM_ADDRESS);

	// Begin an attempt to connect to the remote host
	ConnectionAttemptResult connectResult;
	{
		ScopedProfile raknetConnectProfile("RakNet Connect");

		connectResult = peerConnection->m_peer->Connect(openCommand->GetAddress().c_str(), openCommand->GetPort(), 0, 0);
	}

	if (connectResult != CONNECTION_ATTEMPT_STARTED)
	{
		LogError("Failed to begin connection attempt for %s.  Error code: %u", openCommand->GetAddress().c_str(), connectResult);
		SendConnectionFailedMessage(openCommand->GetAddress(), openCommand->GetPort(), openCommand->GetPeerID());
		return;
	}

	// Increment the number of references that are using this peer
	++peerConnection->m_referenceCount;

	m_peerMutex.Lock();
	m_peers[peerConnection->GetPeerID()] = peerConnection;
	m_peerMutex.Unlock();
}


void XSocketManagerImpl::ProcessAcceptCommand(const CommandPtr& acceptCommand)
{
	PROFILE_SCOPE("ProcessAcceptCommand");

	// Create the peer
	PeerConnectionPtr peerConnection = new PeerConnection(acceptCommand->GetPeerID());

	// Listen on the given port on the machine's default IP address for both IPv4 and IPv6
	RakNet::SocketDescriptor socketDescriptor;
	socketDescriptor.port = acceptCommand->GetPort();
	socketDescriptor.socketFamily = AF_INET; // IPV4

	// Startup the peer interface with the given socket settings.  Will fail if we already have a peer interface for this host
	RakNet::StartupResult startupResult = RakNet::STARTUP_OTHER_FAILURE; 
	
	try 
	{
		startupResult = peerConnection->m_peer->Startup(acceptCommand->GetMaxConnections(), &socketDescriptor, 1, m_networkThreadEvent);
	}
	catch (...)  { }
	
	if (startupResult != RakNet::RAKNET_STARTED)
	{
		LogError("Failed to create incoming connection on port %u.  Error code: %u", acceptCommand->GetPort(), startupResult);
		return;
	}

	// Set the connection to time out after the given amount of time
	peerConnection->m_peer->SetTimeoutTime(kConnectionTimeoutMS, RakNet::UNASSIGNED_SYSTEM_ADDRESS);

	// Calling this function starts the peer listening for incoming connections
	peerConnection->m_peer->SetMaximumIncomingConnections(acceptCommand->GetMaxConnections());

	// Increment the number of references that are using this peer
	++peerConnection->m_referenceCount;

	// Add the new peer to the list
	m_peerMutex.Lock();
	m_peers[peerConnection->GetPeerID()] = peerConnection;
	m_peerMutex.Unlock();
}


void XSocketManagerImpl::ProcessDiscoveryResponseCommand(const CommandPtr& discoveryCommand)
{
	PROFILE_SCOPE("ProcessDiscoveryResponseCommand");

	// Create the peer
	PeerConnectionPtr peerConnection = new PeerConnection(discoveryCommand->GetPeerID());

	// Get the name of this machine
	std::string name = Platform::GetLocalMachineNetworkName();

	// Create the description object of the local system
	SystemDescription desc;
	memcpy(desc.m_name, name.c_str(), std::min(name.length(), sizeof(desc.m_name)));
	desc.m_name[name.length()] = '\0';
	desc.m_role = discoveryCommand->GetRole();

	// Set the system description to be returned along with ping responses
	peerConnection->m_peer->SetOfflinePingResponse(reinterpret_cast<char*>(&desc), sizeof(SystemDescription));

	// Listen on the given port on the machine's default IP address for both IPv4 and IPv6
	RakNet::SocketDescriptor socketDescriptor;
	socketDescriptor.port = discoveryCommand->GetPort();
	socketDescriptor.socketFamily = AF_INET; // IPV4

	// Startup the peer interface with the given socket settings.  Will fail if we already have a peer interface for this host
	RakNet::StartupResult startupResult = RakNet::STARTUP_OTHER_FAILURE;

	try
	{
		startupResult = peerConnection->m_peer->Startup(discoveryCommand->GetMaxConnections(), &socketDescriptor, 1);
	}
	catch (...) {}

	if (startupResult != RakNet::RAKNET_STARTED)
	{
		LogError("Failed to create incoming connection on port %u.  Error code: %u", discoveryCommand->GetPort(), startupResult);
		return;
	}

	// Calling this function starts the peer listening for incoming connections
	peerConnection->m_peer->SetMaximumIncomingConnections(discoveryCommand->GetMaxConnections());

	// Increment the number of references that are using this peer
	++peerConnection->m_referenceCount;

	// Add the new peer to the list
	m_peerMutex.Lock();
	m_peers[peerConnection->GetPeerID()] = peerConnection;
	m_peerMutex.Unlock();
}


void XSocketManagerImpl::ProcessRemovePeerReferenceCommand(const CommandPtr& command)
{
	ScopedLock lock(m_peerMutex);

	auto peerItr = m_peers.find(command->GetPeerID());
	if (peerItr != m_peers.end())
	{
		PeerConnectionPtr peerConnection = peerItr->second;
		--peerConnection->m_referenceCount;
		
		// If there are no more references, start to clean up the peer
		if (peerConnection->m_referenceCount <= 0)
		{
			m_closingPeers.push_back(ClosingPeer(std::chrono::high_resolution_clock::now(), peerConnection));

			m_peers.erase(peerItr);

			m_networkThreadEvent->SetEvent();
		}
	}
}


void XSocketManagerImpl::SendConnectionFailedMessage(const std::string& address, uint16 port, PeerID peerID)
{
	// Fake a network connection failed message and send that to the main thread
	MessagePtr msg = new Message();
	RakNet::SystemAddress raknetAddress(address.c_str(), port);
	msg->SetSystemAddress(raknetAddress);
	msg->SetPeerID(peerID);

	byte msgID = ID_CONNECTION_ATTEMPT_FAILED;
	msg->SetData(&msgID, sizeof(msgID));

	SendMessageToMainThread(msg);
}


bool XSocketManagerImpl::SendMessageToMainThread(const MessagePtr& msg)
{
	if (!m_backupQueue.empty() || !m_messageQueue.TryPush(msg))
	{
		m_backupQueue.push(msg);
		return false;
	}

	return true;
}

XTOOLS_NAMESPACE_END