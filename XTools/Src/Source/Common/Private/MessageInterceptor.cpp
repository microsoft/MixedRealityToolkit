//////////////////////////////////////////////////////////////////////////
// MessageInterceptor.cpp
//
// Shim to allow NetworkConnections to intercept messages of specific 
// types on the network thread
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MessageInterceptor.h"
#include "NetworkInMessageImpl.h"

XTOOLS_NAMESPACE_BEGIN

MessageInterceptor::MessageInterceptor(const PeerPtr& peer)
: m_peer(peer)
{
	XTASSERT(m_peer);
}


bool MessageInterceptor::OnReceive(RakNet::Packet *packet)
{ 
	return HandlePacket(packet->guid, packet->data, packet->length);
}


const PeerPtr& MessageInterceptor::GetPeer()
{
	return m_peer;
}


XTOOLS_NAMESPACE_END
