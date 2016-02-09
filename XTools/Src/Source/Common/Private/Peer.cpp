//////////////////////////////////////////////////////////////////////////
// Peer.cpp
//
// 
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Peer.h"

XTOOLS_NAMESPACE_BEGIN

Peer::Peer(PeerID peerID)
: m_rakPeer(RakNet::RakPeerInterface::GetInstance())
, m_peerID(peerID)
{

}


Peer::~Peer()
{
	RakNet::RakPeerInterface::DestroyInstance(m_rakPeer);
	m_rakPeer = NULL;
}


PeerID Peer::GetPeerID() const
{
	return m_peerID;
}

XTOOLS_NAMESPACE_END
