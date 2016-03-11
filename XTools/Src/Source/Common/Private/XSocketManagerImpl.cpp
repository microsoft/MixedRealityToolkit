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
	PeerConnection()
		: m_peer(new Peer())
		, m_listener(NULL) {}


	// Get the unique ID of this peer, assigned at creation
	PeerID GetPeerID() const { return (*m_peer).GetPeerID(); }

	std::map<RakNet::RakNetGUID, XSocketImpl*>	m_sockets;		// Maps RakNetGUIDs to their corresponding XSocket
	PeerPtr										m_peer;
	IncomingXSocketListener*					m_listener;		// Callback for when new connections are made
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
}


XSocketPtr XSocketManagerImpl::OpenConnection(const std::string& remoteName, uint16 remotePort)
{
	PROFILE_SCOPE("OpenConnection");

	RakNet::SystemAddress addr(remoteName.c_str(), remotePort);

	XSocketImplPtr newSocket = new XSocketImpl(remoteName, remotePort);
	newSocket->SetRegistrationReceipt(CreateRegistrationReceipt(XSocketManagerImplPtr(this), &XSocketManagerImpl::CloseConnection, newSocket->GetID()));

	m_sockets[newSocket->GetID()] = newSocket.get();

	CommandPtr openCommand = new OpenCommand(newSocket);
	SendCommandToNetworkThread(openCommand);

	return newSocket;
}


ReceiptPtr XSocketManagerImpl::AcceptConnections(uint16 port, uint16 maxConnections, IncomingXSocketListener* listener)
{
	XTASSERT(listener != NULL);

	PeerID peerID = Peer::CreatePeerID();

	CommandPtr acceptCommand = new AcceptCommand(peerID, port, maxConnections, listener);
	SendCommandToNetworkThread(acceptCommand);

	return CreateRegistrationReceipt(XSocketManagerImplPtr(this), &XSocketManagerImpl::UnregisterConnectionListener, listener, peerID);
}


void XSocketManagerImpl::SendCommandToNetworkThread(const CommandPtr& command)
{
	// Ensures that that command is enqueued before this function returns
	while (!m_commandQueue.TryPush(command)) {}

	// Signal the network thread to wake up and process the command
	m_networkThreadEvent.Set();
}


void XSocketManagerImpl::CloseConnection(SocketID socketID)
{
	// Lock the peer list.  This can block the network thread
	ScopedLock lock(m_peerMutex);

	XSocketImpl* closingSocket = nullptr;

	// Remove the socket from the socket list
	auto itr = m_sockets.find(socketID);
	if (itr != m_sockets.end())
	{
		closingSocket = itr->second;
		m_sockets.erase(itr);
	}

	
	if (closingSocket)
	{
		// Find the peer for this connection
		PeerPtr peer = closingSocket->GetPeer();

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

		auto peerIter = m_peers.find((*peer).GetPeerID());
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
					if (connectionList.empty() && currentPeerConnection->m_listener == NULL)
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


void XSocketManagerImpl::UnregisterConnectionListener(IncomingXSocketListener*	listener, PeerID peerID)
{
	if (XTVERIFY(listener != NULL))
	{
		// Lock the peer list.  This can block the network thread
		ScopedLock lock(m_peerMutex);

		auto iter = m_peers.find(peerID);
		if (XTVERIFY(iter != m_peers.end()))
		{
			PeerConnectionPtr peerConnection = iter->second;

			if (XTVERIFY(peerConnection->m_listener == listener))
			{
				// Set the max incoming connections to zero to stop accepting new connections
				peerConnection->m_peer->SetMaximumIncomingConnections(0);
				peerConnection->m_listener = NULL;

				// Destroy the peer if no one is using it anymore
				if (peerConnection->m_sockets.empty() && peerConnection->m_listener == NULL)
				{
					peerConnection->m_peer->Shutdown(kRakNetShutdownTime);
					m_closingPeers.push_back(ClosingPeer(std::chrono::high_resolution_clock::now(), peerConnection->m_peer));

					m_peers.erase(iter);
				}
			}
		}
	}
}


void XSocketManagerImpl::Update()
{
	PROFILE_FUNCTION();

	uint32 messagesProcessed = 0;

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

	MessagePtr incomingMessage;
	while (m_messageQueue.TryGetMessage(incomingMessage))
	{
		if (XTVERIFY(incomingMessage->IsValid()))
		{
			const byte packetID = incomingMessage->GetMessageID();

			if (XTVERIFY(packetID < 255))
			{
				// First check to see if this is a new incoming connection
				if (packetID == ID_NEW_INCOMING_CONNECTION)
				{
					// Create a connection object to represent this connection.  
					XSocketImplPtr connection = new XSocketImpl(connection->GetID(), incomingMessage->GetSystemAddress());
					connection->SetRegistrationReceipt(CreateRegistrationReceipt(XSocketManagerImplPtr(this), &XSocketManagerImpl::CloseConnection, connection->GetID()));

					peerConnection->m_sockets[incomingMessage->GetRakNetGUID()] = connection.get();

					// Notify the listener that a new connection has been made
					IncomingXSocketListener* listener = peerConnection->m_listener;
					if (listener)
					{
						listener->OnNewConnection(connection);
					}
					else
					{
						LogError("Accepted connection request, but have no listener");
					}
				}
				// Otherwise, find the Connection this packet is meant for and pass it off 
				else if (!peerConnection->m_sockets.empty())
				{
					// Once the connection is accepted, the full system address may have been fully resolved.  
					// Update the binding
					if (packetID == ID_CONNECTION_REQUEST_ACCEPTED)
					{
						// This should only happen on outgoing connections where there is only one connection per peer.
						if (XTVERIFY(peerConnection->m_sockets.size() == 1))
						{
							XSocketImplPtr connection = peerConnection->m_sockets.begin()->second;
							XTASSERT(connection->GetAddress().GetPort() == incomingMessage->GetSystemAddress().GetPort());

							connection->SetAddress(incomingMessage->GetSystemAddress());
							connection->SetRakNetGUID(incomingMessage->GetRakNetGUID());
							peerConnection->m_sockets.clear();
							peerConnection->m_sockets[incomingMessage->GetRakNetGUID()] = connection.get();
						}
					}

					// Check to see if we have a socket for this packet.  Its possible for this to be false
					// if the connection was terminated but the peer still has pending received packets in the queue
					auto socketIter = peerConnection->m_sockets.find(incomingMessage->GetRakNetGUID());
					if (socketIter != peerConnection->m_sockets.end())
					{
						// Hold a reference to the connection to prevent it from getting destroyed before this callback completes
						XSocketImplPtr connection = socketIter->second;
						connection->OnReceiveMessage(incomingMessage);
					}
					else
					{
						LogError("Received message from %s with no associated socket", incomingMessage->GetSystemAddress().ToString(false));
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
}


void XSocketManagerImpl::ProcessCommands()
{
	CommandPtr newCommand;

	while (m_commandQueue.TryPop(newCommand))
	{
		const int typeID = newCommand->GetTypeInfo().GetID();

		if (OpenCommand::MyTypeInfo().GetID() == typeID)
		{
			OpenCommandPtr openCommand = reflection_cast<OpenCommand>(newCommand);
			ProcessOpenCommand(openCommand);
		}
		else if (AcceptCommand::MyTypeInfo().GetID() == typeID)
		{
			AcceptCommandPtr acceptCommand = reflection_cast<AcceptCommand>(newCommand);
			ProcessAcceptCommand(acceptCommand);
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
				MessagePtr msg = new Message(packet->length);
				msg->m_address = packet->systemAddress;
				msg->m_rakNetGuid = packet->guid;
				msg->m_peerID = peerConnection->GetPeerID();
				msg->m_payload.Append(packet->data, packet->length);

				// Check to see if we have a socket for this packet, and if so call back to its async message received function
				auto socketIter = peerConnection->m_sockets.find(packet->guid);
				if (socketIter != peerConnection->m_sockets.end())
				{
					// Hold a reference to the connection to prevent it from getting destroyed before this callback completes
					XSocketImplPtr connection = socketIter->second;
					connection->OnReceiveMessageAsync(msg);
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


void XSocketManagerImpl::ProcessOpenCommand(const OpenCommandPtr& openCommand)
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
		LogError("Failed to create a connection.  Error code: %u", startupResult);
		SendConnectionFailedMessage(newSocket->GetID());
		return;
	}

	// Set the connection to time out after the given amount of time
	peerConnection->m_peer->SetTimeoutTime(kConnectionTimeoutMS, UNASSIGNED_SYSTEM_ADDRESS);

	// Begin an attempt to connect to the remote host
	ConnectionAttemptResult connectResult;
	{
		ScopedProfile raknetConnectProfile("RakNet Connect");

		connectResult = peerConnection->m_peer->Connect(newSocket->GetAddress().ToString(false), newSocket->GetAddress().GetPort(), 0, 0);
	}

	if (connectResult != CONNECTION_ATTEMPT_STARTED)
	{
		SendConnectionFailedMessage(newSocket->GetID());
		return;
	}

	// Set the peer on the socket now that a successful connection attempt has been made.  This will allow the
	// socket to send network messages
	openCommand->GetSocket()->SetPeer(peerConnection->m_peer);

	// Store a naked pointer to this socket; it will remove itself with a call to CloseConnection when its deleted
	peerConnection->m_sockets[UNASSIGNED_RAKNET_GUID] = newSocket.get();

	m_peerMutex.Lock();
	m_peers[peerConnection->GetPeerID()] = peerConnection;
	m_peerMutex.Unlock();
}


void XSocketManagerImpl::ProcessAcceptCommand(const AcceptCommandPtr& acceptCommand)
{
	PROFILE_SCOPE("ProcessAcceptCommand");

	// Create the peer
	PeerConnectionPtr peerConnection = new PeerConnection(acceptCommand->GetPeerID());

	// Listen on the given port on the machine's default IP address for both IPv4 and IPv6
	RakNet::SocketDescriptor socketDescriptor;
	socketDescriptor.port = port;
	socketDescriptor.socketFamily = AF_INET; // IPV4

											 // Startup the peer interface with the given socket settings.  Will fail if we already have a peer interface for this host
	RakNet::StartupResult startupResult = peerConnection->m_peer->Startup(maxConnections, &socketDescriptor, 1);
	if (startupResult != RakNet::RAKNET_STARTED)
	{
		LogError("Failed to create incoming connection on port %u.  Error code: %u", port, startupResult);
		return NULL;
	}

	// Set the connection to time out after the given amount of time
	peerConnection->m_peer->SetTimeoutTime(kConnectionTimeoutMS, RakNet::UNASSIGNED_SYSTEM_ADDRESS);

	// Add to the list of all peers
	peerConnection->m_listener = listener;

	// Calling this function starts the peer listening for incoming connections
	peerConnection->m_peer->SetMaximumIncomingConnections(maxConnections);

	// Add the new peer to the list
	m_peerMutex.Lock();
	m_peers[peerConnection->GetPeerID()] = peerConnection;
	m_peerMutex.Unlock();

	
}


void XSocketManagerImpl::SendConnectionFailedMessage(SocketID socketID)
{
	// Fake a network connection failed message and send that to the main thread

	MessagePtr msg = new Message(socketID);
	byte msgID = ID_CONNECTION_ATTEMPT_FAILED;
	msg->AppendData(&msgID, sizeof(msgID));
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