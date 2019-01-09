//////////////////////////////////////////////////////////////////////////
// DiscoveryClientImpl.cpp
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DiscoveryClientImpl.h"
#include "PacketWrapper.h"
#include "SystemDescription.h"

XTOOLS_NAMESPACE_BEGIN

static const uint32 kMaxPingResponseTime = 800;


//static 
DiscoveryClientPtr DiscoveryClient::Create()
{
	return new DiscoveryClientImpl();
}


DiscoveryClientImpl::DiscoveryClientImpl()
	: m_listenerList(ListenerList::Create())
	, m_peer(new Peer(0))
	, m_bClearedOutStaleClients(false)
#if RAKPEER_USER_THREADED==1
	, m_updateBitStream(MAXIMUM_MTU_SIZE
		#if LIBCAT_SECURITY==1
			+ cat::AuthenticatedEncryption::OVERHEAD_BYTES
		#endif
	)
#endif
{
	RakNet::SocketDescriptor socketDescriptor(kDiscoveryClientPort, 0);
	socketDescriptor.socketFamily = AF_INET; // IPV4
	
	RakNet::StartupResult startupResult = m_peer->Startup(1, &socketDescriptor, 1);
	if (startupResult != RakNet::RAKNET_STARTED)
	{
		LogError("Failed to initialize DiscoveryClient.  Error code: %u", startupResult);
	}
}


void DiscoveryClientImpl::Ping()
{
	m_peer->Ping("255.255.255.255", kDiscoveryServerPort, false);
	m_lastPingTime = std::chrono::high_resolution_clock::now();
	m_bClearedOutStaleClients = false;

	// Mark all the remote systems as unresponsive, so we can clear them out if they don't 
	// respond to the new ping
	for (auto it = m_remoteSystems.begin(); it != m_remoteSystems.end(); ++it)
	{
		it->second.m_bReceivedResponse = false;
	}
}


uint32 DiscoveryClientImpl::GetDiscoveredCount() const
{
	return static_cast<uint32>(m_remoteSystems.size());
}


DiscoveredSystemPtr DiscoveryClientImpl::GetDiscoveredSystem(uint32 index) const
{
	if (index < GetDiscoveredCount())
	{
		auto it = m_remoteSystems.cbegin();
		for (uint32 i = 0; i < index; ++it, ++i) {}

		return it->second.m_remoteSystem;
	}
	else
	{
		return nullptr;
	}
}


void DiscoveryClientImpl::Update()
{
#if RAKPEER_USER_THREADED==1
	m_updateBitStream.Reset();
	m_peer->RunUpdateCycle(m_updateBitStream);
#endif
	// Process any incoming responses to our ping
	for (PacketWrapper packet(m_peer, m_peer->Receive()); packet.IsValid(); packet = m_peer->Receive())
	{
		if (packet->data[0] == ID_UNCONNECTED_PONG)
		{
			RakNet::TimeMS time;
			RakNet::BitStream bsIn(packet->data, packet->length, false);
			bsIn.IgnoreBytes(1);
			bsIn.Read(time);

			uint32 dataLength = packet->length - sizeof(unsigned char) - sizeof(RakNet::TimeMS);
			if (XTVERIFY(dataLength == sizeof(SystemDescription)))
			{
				SystemDescription* desc = reinterpret_cast<SystemDescription*>(packet->data + sizeof(unsigned char) + sizeof(RakNet::TimeMS));

				std::string descString(desc->m_name);
				descString += desc->m_role;

				// Check to see if we already know about this remote system
				auto iter = m_remoteSystems.find(descString);
				if (iter != m_remoteSystems.end())
				{
					// We already knew about this remote system.  Update the time we last heard from it
					iter->second.m_bReceivedResponse = true;
					iter->second.m_pingTime = time;
				}
				else
				{
					// New discovery!  Add it
					DiscoveredSystemPtr newSystem = new DiscoveredSystem(desc->m_name, packet->systemAddress.ToString(false), desc->m_role);
					
					RemoteClient newRemoteClient;
					newRemoteClient.m_pingTime = time;
					newRemoteClient.m_remoteSystem = newSystem;
					newRemoteClient.m_bReceivedResponse = true;

					m_remoteSystems[descString] = newRemoteClient;

					// Notify
					m_listenerList->NotifyListeners(&DiscoveryClientListener::OnRemoteSystemDiscovered, newSystem);
				}
			}
		}
	}

	// Remove any clients that have not responded
	if (!m_bClearedOutStaleClients)
	{
		std::chrono::high_resolution_clock::time_point currentTime = std::chrono::high_resolution_clock::now();

		int64 silentTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - m_lastPingTime).count();

		// If the timeout for receiving responses since the last ping has expired, then clear out any cached systems that 
		// did not respond
		if (silentTime > kMaxPingResponseTime)
		{
			auto it = m_remoteSystems.begin();
			while (it != m_remoteSystems.end()) 
			{
				if (!it->second.m_bReceivedResponse)
				{
					DiscoveredSystemPtr staleClient = it->second.m_remoteSystem;
					
					m_listenerList->NotifyListeners(&DiscoveryClientListener::OnRemoteSystemLost, staleClient);
				
					auto toErase = it;
					++it;
					m_remoteSystems.erase(toErase);
				}
				else 
				{
					++it;
				}
			}

			m_bClearedOutStaleClients = true;
		}
	}
}


void DiscoveryClientImpl::AddListener(DiscoveryClientListener* newListener)
{
	m_listenerList->AddListener(newListener);
}


void DiscoveryClientImpl::RemoveListener(DiscoveryClientListener* oldListener)
{
	m_listenerList->RemoveListener(oldListener);
}

XTOOLS_NAMESPACE_END
