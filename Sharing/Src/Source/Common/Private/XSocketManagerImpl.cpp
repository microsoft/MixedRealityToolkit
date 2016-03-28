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
static const uint64 kRakNetShutdownTime = 300;

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
	PeerConnection() : m_peer(new Peer()) {}
	explicit PeerConnection(PeerID peerID) : m_peer(new Peer(peerID)) {}

	// Get the unique ID of this peer, assigned at creation
	PeerID GetPeerID() const { return m_peer.get()->GetPeerID(); }

	std::map<RakNet::RakNetGUID, XSocketImpl*>	m_sockets;		// Maps RakNetGUIDs to their corresponding XSocket
	PeerPtr										m_peer;
};



//////////////////////////////////////////////////////////////////////////
// XSocketManagerImpl implementation

XSocketManagerImpl::XSocketManagerImpl()
	: m_stopping(0)
	, m_commandQueue(kCommandQueueSize)
	, m_messageQueue(kMessageQueueSize)
	, m_networkThreadEvent(false, true)
{
	// Start a thread to run the update loop. 
	m_networkThread = new MemberFuncThread(&XSocketManagerImpl::ThreadFunc, this);
}


XSocketManagerImpl::~XSocketManagerImpl()
{
	// trigger the thread exit and wait for it...
	m_stopping = 1;
	m_networkThread->WaitForThreadExit();

	XTASSERT(m_peers.empty());
	XTASSERT(m_closingPeers.empty());
}


XSocketPtr XSocketManagerImpl::OpenConnection(const std::string& remoteName, uint16 remotePort)
{
	PROFILE_SCOPE("XSocketManager::OpenConnection");

	XTASSERT(remotePort != 0);

	XSocketImplPtr newSocket = new XSocketImpl(remoteName, remotePort);
	newSocket->SetRegistrationReceipt(CreateRegistrationReceipt(XSocketManagerImplPtr(this), &XSocketManagerImpl::CloseConnection, newSocket.get()));

	CommandPtr openCommand = new Command(newSocket);
	SendCommandToNetworkThread(openCommand);

	return newSocket;
}


ReceiptPtr XSocketManagerImpl::AcceptConnections(uint16 port, uint16 maxConnections, IncomingXSocketListener* listener)
{
	PROFILE_SCOPE("XSocketManager::AcceptConnections");

	XTASSERT(listener != NULL);

	PeerID peerID = Peer::CreatePeerID();

	m_incomingConnectionListeners[peerID] = listener;

	CommandPtr acceptCommand = new Command(peerID, port, maxConnections);
	SendCommandToNetworkThread(acceptCommand);

	return CreateRegistrationReceipt(XSocketManagerImplPtr(this), &XSocketManagerImpl::UnregisterConnectionListener, peerID);
}


void XSocketManagerImpl::SendCommandToNetworkThread(const CommandPtr& command)
{
	// Ensures that that command is enqueued before this function returns
	while (!m_commandQueue.TryPush(command)) {}

	// Signal the network thread to wake up and process the command
	m_networkThreadEvent.Set();
}


void XSocketManagerImpl::CloseConnection(XSocketImpl* closingSocket)
{
	if (closingSocket)
	{
		// Find the peer for this connection.  This will be NULL if the network thread failed to successfully
		// create the connection in the first place
		PeerPtr peer = closingSocket->GetPeer();
		if (peer)
		{
			// Close the connection if its open
			{
				RakNet::ConnectionState currentState = peer->GetConnectionState(closingSocket->GetRakNetGUID());
				if (currentState == RakNet::ConnectionState::IS_CONNECTED ||
					currentState == RakNet::ConnectionState::IS_CONNECTING ||
					currentState == RakNet::ConnectionState::IS_PENDING)
				{
					peer->CloseConnection(closingSocket->GetRakNetGUID(), true);
				}
			}

			ScopedLock lock(m_peerMutex);

			auto peerIter = m_peers.find(peer.get()->GetPeerID());
			if (XTVERIFY(peerIter != m_peers.end()))
			{
				PeerConnectionPtr currentPeerConnection = peerIter->second;

				// Make sure that this peer connection is the right one for the given peer
				if (XTVERIFY(currentPeerConnection->m_peer == peer))
				{
					// Find the socket that is closing
					auto& connectionList = currentPeerConnection->m_sockets;

					auto socketIter = connectionList.find(closingSocket->GetRakNetGUID());
					if (XTVERIFY(socketIter != connectionList.end()))
					{
						connectionList.erase(socketIter);

						// Destroy the peer if no one is using it anymore
						if (connectionList.empty() && m_incomingConnectionListeners.find(currentPeerConnection->GetPeerID()) == m_incomingConnectionListeners.end())
						{
							currentPeerConnection->m_peer->Shutdown(kRakNetShutdownTime);
							m_closingPeers.push_back(ClosingPeer(std::chrono::high_resolution_clock::now(), currentPeerConnection->m_peer));

							m_peers.erase(peerIter);
						}
					}
				}
			}
		}
	}
}


void XSocketManagerImpl::UnregisterConnectionListener(PeerID peerID)
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

		m_incomingConnectionListeners.erase(m_incomingConnectionListeners.find(peerID));

		// Destroy the peer if no one is using it anymore
		if (peerConnection->m_sockets.empty())
		{
			peerConnection->m_peer->Shutdown(kRakNetShutdownTime);
			m_closingPeers.push_back(ClosingPeer(std::chrono::high_resolution_clock::now(), peerConnection->m_peer));

			m_peers.erase(iter);
		}
	}
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
			// Get the socket that this message came from
			XSocketImplPtr socket = msg->GetSocket();
			if (socket)
			{
				const byte packetID = msg->GetMessageID();

				// First check to see if this is a new incoming connection
				if (packetID == ID_NEW_INCOMING_CONNECTION)
				{
					// Notify the listener that a new connection has been made
					IncomingXSocketListener* listener = nullptr;

					auto listenerItr = m_incomingConnectionListeners.find(msg->GetPeerID());
					if (listenerItr != m_incomingConnectionListeners.end())
					{
						listener = listenerItr->second;
					}

					if (listener)
					{
						listener->OnNewConnection(socket);
					}
					else
					{
						LogError("Accepted connection request, but have no listener");
					}
				}
				// Otherwise, find the Connection this packet is meant for and pass it off 
				else 
				{
					socket->OnReceiveMessage(msg);
				}

				++messagesProcessed;

				// If we've hit the limit on the maximum number of messages to process per peer per update, then go on to the next peer
				if (messagesProcessed >= kMaxMessagesPerUpdate)
				{
					break;
				}
			}
			else
			{
				LogError("Received message from %s with no associated socket", msg->GetSystemAddress().ToString(false));
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


void XSocketManagerImpl::ThreadFunc()
{
	while (m_stopping == 0)
	{
		ProcessCommands();
		ProcessMessages();
		m_networkThreadEvent.WaitTimeout(10);
	}

	ScopedLock lock(m_peerMutex);
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
				msg->SetPeerID(peerConnection->GetPeerID());
				msg->SetData(packet->data, packet->length);

				// If an outgoing connection was accepted by a remote peer...
				if (msg->GetMessageID() == ID_CONNECTION_REQUEST_ACCEPTED)
				{
					// This should only happen on outgoing connections where there is only one connection per peer.
					if (XTVERIFY(peerConnection->m_sockets.size() == 1))
					{
						XSocketImplPtr socket = peerConnection->m_sockets.begin()->second;

						// After the connection is made, the address is often updated to be more accurate, so capture that
						socket->SetAddress(msg->GetSystemAddress());

						// We did not have a valid RakNetGUID for this connection until now, so reset the mapping 
						// for this socket to its guid
						socket->SetRakNetGUID(packet->guid);
						peerConnection->m_sockets.clear();
						peerConnection->m_sockets[packet->guid] = socket.get();

						msg->SetSocket(socket);
					}
				}
				// If we have accepted a new connection from a remote peer...
				else if (msg->GetMessageID() == ID_NEW_INCOMING_CONNECTION)
				{
					// Create a socket object to represent this connection.  Note that the main thread will trigger the callback
					// to listeners to let them know about the new connection.  
					XSocketImplPtr newSocket = new XSocketImpl(std::string(), 0);
					newSocket->SetAddress(msg->GetSystemAddress());
					newSocket->SetRakNetGUID(packet->guid);
					newSocket->SetPeer(peerConnection->m_peer);
					newSocket->SetRegistrationReceipt(CreateRegistrationReceipt(XSocketManagerImplPtr(this), &XSocketManagerImpl::CloseConnection, newSocket.get()));

					msg->SetSocket(newSocket);

					peerConnection->m_sockets[packet->guid] = newSocket.get();
				}
				else
				{
					// Check to see if we have a socket for this packet, and if so call back to its async message received function
					auto socketIter = peerConnection->m_sockets.find(packet->guid);
					if (socketIter != peerConnection->m_sockets.end())
					{
						XSocketImplPtr socket = socketIter->second;

						msg->SetSocket(socketIter->second);

						socket->OnReceiveMessageAsync(msg);
					}
				}
				
				// Finally stuff the message in the message queue for delivery to the main thread
				SendMessageToMainThread(msg);

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

	XSocketImplPtr newSocket = openCommand->GetSocket();

	PeerConnectionPtr peerConnection;
	{
		ScopedProfile peerCreationProfile("new PeerConnection");
		peerConnection = new PeerConnection();
	}
	SocketDescriptor sd;
	sd.socketFamily = AF_INET; // IPV4

							   // Startup the peer interface.  Will fail if we already have a peer interface for this host
	StartupResult startupResult = peerConnection->m_peer->Startup(1, &sd, 1);
	if (startupResult != RAKNET_STARTED)
	{
		LogError("Failed to create connection object for %s.  Error code: %u", newSocket->GetRemoteSystemName().c_str(), startupResult);
		SendConnectionFailedMessage(newSocket);
		return;
	}

	// Set the connection to time out after the given amount of time
	peerConnection->m_peer->SetTimeoutTime(kConnectionTimeoutMS, UNASSIGNED_SYSTEM_ADDRESS);

	// Begin an attempt to connect to the remote host
	ConnectionAttemptResult connectResult;
	{
		ScopedProfile raknetConnectProfile("RakNet Connect");

		connectResult = peerConnection->m_peer->Connect(newSocket->GetRemoteSystemName().c_str(), newSocket->GetRemoteSystemPort(), 0, 0);
	}

	if (connectResult != CONNECTION_ATTEMPT_STARTED)
	{
		LogError("Failed to begin connection attempt for %s.  Error code: %u", newSocket->GetRemoteSystemName().c_str(), connectResult);
		SendConnectionFailedMessage(newSocket);
		return;
	}

	// Set the peer on the socket now that a successful connection attempt has been made.  This will allow the
	// socket to send network messages
	newSocket->SetPeer(peerConnection->m_peer);

	// Store a naked pointer to this socket; it will remove itself with a call to CloseConnection when its deleted
	peerConnection->m_sockets[UNASSIGNED_RAKNET_GUID] = newSocket.get();

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
		peerConnection->m_peer->Startup(acceptCommand->GetMaxConnections(), &socketDescriptor, 1);
	}
	catch (...) {}
	
	if (startupResult != RakNet::RAKNET_STARTED)
	{
		LogError("Failed to create incoming connection on port %u.  Error code: %u", acceptCommand->GetPort(), startupResult);
		return;
	}

	// Set the connection to time out after the given amount of time
	peerConnection->m_peer->SetTimeoutTime(kConnectionTimeoutMS, RakNet::UNASSIGNED_SYSTEM_ADDRESS);

	// Calling this function starts the peer listening for incoming connections
	peerConnection->m_peer->SetMaximumIncomingConnections(acceptCommand->GetMaxConnections());

	// Add the new peer to the list
	m_peerMutex.Lock();
	m_peers[peerConnection->GetPeerID()] = peerConnection;
	m_peerMutex.Unlock();
}


void XSocketManagerImpl::SendConnectionFailedMessage(const XSocketImplPtr& socket)
{
	// Fake a network connection failed message and send that to the main thread

	MessagePtr msg = new Message();
	msg->SetSocket(socket);

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