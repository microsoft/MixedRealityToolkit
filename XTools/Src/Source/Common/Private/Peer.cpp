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

std::atomic<PeerID>	Peer::m_sCounter;

Peer::Peer()
: m_rakPeer(RakNet::RakPeerInterface::GetInstance())
, m_peerID(CreatePeerID())
{

}


Peer::Peer(PeerID id)
	: m_rakPeer(RakNet::RakPeerInterface::GetInstance())
	, m_peerID(id)
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


//static 
PeerID Peer::CreatePeerID()
{
	return ++m_sCounter;
}

XTOOLS_NAMESPACE_END
