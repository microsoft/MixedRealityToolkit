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

XTOOLS_NAMESPACE_BEGIN

XTOOLS_REFLECTION_DEFINE(XSocketManager)
.BaseClass<Reflection::XTObject>()
.BaseClass<IUpdateable>();

XTOOLS_REFLECTION_DEFINE(XSocketManagerImpl)
.BaseClass<XSocketManager>();

// It is possible that processing the incoming messages progresses at a slower
// pace than the rate of messages coming in.  To prevent deadlock, we set a maximum number
// of messages that can be processed per call to Update().
static const uint32 kMaxMessagesPerUpdate = 200;

// How long (in milliseconds) to allow RakNet to finish flushing messages and stop a peer's
// thread before destroying it
static const uint64 kRakNetShutdownTime = 300;

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
	PeerPtr tempPeer = new Peer(0);

	int numAddresses = tempPeer->GetNumberOfAddresses();
	for (int i = 0; i < numAddresses; ++i)
	{
		addressList.push_back(IPAddressV4(tempPeer->GetLocalIP(i)));
	}

	return addressList;
}


struct XSocketManagerImpl::PeerConnection : public AtomicRefCounted
{
	PeerConnection(uint32 peerID)
	: m_peer(new Peer(peerID))
	, m_listener(NULL)
	{
	}

	std::map<RakNet::RakNetGUID, XSocketImpl*>	m_sockets;		// Maps RakNetGUIDs to their corresponding XSocket
	PeerPtr										m_peer;
	IncomingXSocketListener*					m_listener;		// Callback for when new connections are made
};



XSocketManagerImpl::XSocketManagerImpl()
	: m_nextSocketID(1)
	, m_nextPeerID(1)
{
	
}


XSocketPtr XSocketManagerImpl::OpenConnection(const std::string& remoteName, uint16 remotePort)
{
	PROFILE_SCOPE("OpenConnection");

	using namespace RakNet;

	PeerConnectionPtr peerConnection;
	{
		ScopedProfile peerCreationProfile("new PeerConnection");
		peerConnection = new PeerConnection(m_nextPeerID++);
	}
	SocketDescriptor sd;
	sd.socketFamily = AF_INET; // IPV4

	// Startup the peer interface.  Will fail if we already have a peer interface for this host
	StartupResult startupResult = peerConnection->m_peer->Startup(1, &sd, 1);
	if (startupResult != RAKNET_STARTED)
	{
		LogError("Failed to create a connection to %s:%u.  Error code: %u", remoteName.c_str(), remotePort, startupResult);
		return NULL;
	}

	// Set the connection to time out after the given amount of time
	peerConnection->m_peer->SetTimeoutTime(kConnectionTimeoutMS, UNASSIGNED_SYSTEM_ADDRESS);

	// Begin an attempt to connect to the remote host
	ConnectionAttemptResult connectResult;
	{
		ScopedProfile raknetConnectProfile("RakNet Connect");
		connectResult = peerConnection->m_peer->Connect(remoteName.c_str(), remotePort, 0, 0);
	}
	
	if (connectResult != CONNECTION_ATTEMPT_STARTED)
	{
		return NULL;
	}

	RakNet::SystemAddress addr(remoteName.c_str(), remotePort);

	// Create a connection object to represent this connection.  
	XSocketImplPtr connection = new XSocketImpl(this,m_nextSocketID++, peerConnection->m_peer, addr);
	connection->SetRegistrationReceipt(CreateRegistrationReceipt(XSocketManagerImplPtr(this), &XSocketManagerImpl::OnConnectionDestroyed, connection.get()));

	// Store a naked pointer to this Connection; it will remove itself with a call to OnConnectionDestroyed when its deleted
	peerConnection->m_sockets[UNASSIGNED_RAKNET_GUID] = connection.get();

	m_peers[peerConnection->m_peer.get()->GetPeerID()] = peerConnection;

	m_messageQueue.AddConnection(peerConnection->m_peer);

	return connection;
}


ReceiptPtr XSocketManagerImpl::AcceptConnections(uint16 port, uint16 maxConnections, IncomingXSocketListener* listener)
{
	PROFILE_FUNCTION();

	XTASSERT(listener != NULL);

	// Create the peer
	PeerID newPeerID = m_nextPeerID++;
	PeerConnectionPtr peerConnection = new PeerConnection(newPeerID);

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
	m_peers[newPeerID] = peerConnection;

	m_messageQueue.AddConnection(peerConnection->m_peer);

	ListenerInfo listenerInfo;
	listenerInfo.m_listener = listener;
	listenerInfo.m_peerID = newPeerID;
	return CreateRegistrationReceipt(XSocketManagerImplPtr(this), &XSocketManagerImpl::UnregisterConnectionListener, listenerInfo);
}


void XSocketManagerImpl::AddInterceptor(const MessageInterceptorPtr& interceptor)
{
	m_messageQueue.AddMessageInterceptor(interceptor);
}


void XSocketManagerImpl::RemoveInterceptor(const MessageInterceptorPtr& interceptor)
{
	m_messageQueue.RemoveMessageInterceptor(interceptor);
}


void XSocketManagerImpl::UnregisterConnectionListener(ListenerInfo listenerInfo)
{
	if (XTVERIFY(listenerInfo.m_listener != NULL))
	{
		auto iter = m_peers.find(listenerInfo.m_peerID);
		if (XTVERIFY(iter != m_peers.end()))
		{
			PeerConnectionPtr peerConnection = iter->second;

			if (XTVERIFY(peerConnection->m_listener == listenerInfo.m_listener))
			{
				// Set the max incoming connections to zero to stop accepting new connections
				peerConnection->m_peer->SetMaximumIncomingConnections(0);
				peerConnection->m_listener = NULL;

				// Destroy the peer if no one is using it anymore
				if (peerConnection->m_sockets.empty() && peerConnection->m_listener == NULL)
				{
					peerConnection->m_peer->Shutdown(kRakNetShutdownTime);
					m_closingPeers.push_back(ClosingPeer(std::chrono::high_resolution_clock::now(), peerConnection->m_peer));

					m_messageQueue.RemoveConnection(peerConnection->m_peer);
					m_peers.erase(iter);
				}
			}
		}
	}
}


void XSocketManagerImpl::OnConnectionDestroyed(XSocketImpl* closingConnection)
{
	// Find the peer for this connection
	PeerPtr peer = closingConnection->GetPeer();

	// Close the connection if its open
	{
		RakNet::ConnectionState currentState = peer->GetConnectionState(closingConnection->GetRakNetGUID());
		if (currentState == RakNet::ConnectionState::IS_CONNECTED ||
			currentState == RakNet::ConnectionState::IS_CONNECTING ||
			currentState == RakNet::ConnectionState::IS_PENDING)
		{
			peer->CloseConnection(closingConnection->GetRakNetGUID(), true);
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

			auto socketIter = connectionList.find(closingConnection->GetRakNetGUID());
			if (XTVERIFY(socketIter != connectionList.end()))
			{
				connectionList.erase(socketIter);

				// Destroy the peer if no one is using it anymore
				if (connectionList.empty() && currentPeerConnection->m_listener == NULL)
				{
					currentPeerConnection->m_peer->Shutdown(kRakNetShutdownTime);
					m_closingPeers.push_back(ClosingPeer(std::chrono::high_resolution_clock::now(), currentPeerConnection->m_peer));

					m_messageQueue.RemoveConnection(currentPeerConnection->m_peer);
					m_peers.erase(peerIter);
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
				auto peerIter = m_peers.find(incomingMessage->GetPeerID());
				if (peerIter != m_peers.end())
				{
					PeerConnectionPtr peerConnection = peerIter->second;

					// First check to see if this is a new incoming connection
					if (packetID == ID_NEW_INCOMING_CONNECTION)
					{
						// Create a connection object to represent this connection.  
						XSocketImplPtr connection = new XSocketImpl(this, m_nextSocketID++, peerConnection->m_peer, incomingMessage->GetSystemAddress(), incomingMessage->GetRakNetGUID());
						connection->SetRegistrationReceipt(CreateRegistrationReceipt(XSocketManagerImplPtr(this), &XSocketManagerImpl::OnConnectionDestroyed, connection.get()));

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
				}
				else
				{
					LogWarning("Got message from closed peer connection %i", incomingMessage->GetPeerID());
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


std::string XSocketManagerImpl::GetLocalMachineAddress()
{
	// Frustratingly, RakNet does not provide a way to get the address of the local machine without first creating a peer.  
	// One you have one or more peers, it shouldn't matter which one you use to find out your own address, as it should
	// be the same for all of them
	if (!m_peers.empty())
	{
		
		RakNet::SystemAddress address = m_peers.begin()->second->m_peer->GetInternalID();

		return address.ToString(false);
	}
	else
	{
		return std::string();
	}
}


XTOOLS_NAMESPACE_END