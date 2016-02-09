//////////////////////////////////////////////////////////////////////////
// DiscoveryServerImpl.cpp
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DiscoveryServerImpl.h"
#include "SystemDescription.h"
#include "PacketWrapper.h"

XTOOLS_NAMESPACE_BEGIN

//static 
DiscoveryServerPtr DiscoveryServer::Create(SystemRole role)
{
	return new DiscoveryServerImpl(role);
}


DiscoveryServerImpl::DiscoveryServerImpl(SystemRole role)
: m_peer(new Peer(0))
{
	// Get the name of this machine
	std::string name = Platform::GetLocalMachineNetworkName();

	// Create the description object of the local system
	SystemDescription desc;
	memcpy(desc.m_name, name.c_str(), name.length());
	desc.m_name[name.length()] = '\0';
	desc.m_role = role;

	// Set the system description to be returned along with ping responses
	m_peer->SetOfflinePingResponse(reinterpret_cast<char*>(&desc), sizeof(SystemDescription));

	// Initialize the listener socket
	RakNet::SocketDescriptor socketDescriptor(kDiscoveryServerPort, 0);
	RakNet::StartupResult startupResult = m_peer->Startup(2, &socketDescriptor, 1);
	if (startupResult != RakNet::RAKNET_STARTED)
	{
		//LogError("Failed to initialize DiscoveryServer.  Error code: %u", startupResult); // TODO: No DiscoveryServer support for BUILD 2016.
		return;
	}

	// Start listening for pings
	m_peer->SetMaximumIncomingConnections(2);
}


void DiscoveryServerImpl::Update()
{
	// We don't need to actually handle any packets, so just keep the incoming queue clear
	for (PacketWrapper packet(m_peer, m_peer->Receive()); packet.IsValid(); packet = m_peer->Receive())
	{
	}
}

XTOOLS_NAMESPACE_END
