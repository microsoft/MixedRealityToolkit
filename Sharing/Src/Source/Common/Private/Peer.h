//////////////////////////////////////////////////////////////////////////
// Peer.h
//
// Safe wrapper for RakPeerInterface
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

// ID assigned by the XSocketManager to easily identify peers
typedef uint32 PeerID;
const PeerID kInvalidPeerID = 0xFFFFFFFF;

class Peer : public AtomicRefCounted
{
public:
	Peer();
	explicit Peer(PeerID id);
	virtual ~Peer();

	RakNet::RakPeerInterface*		operator->() { return m_rakPeer; }
	const RakNet::RakPeerInterface* operator->() const { return m_rakPeer; }

	// Get the unique ID of this peer, assigned at creation
	PeerID							GetPeerID() const;

	RakNet::RakPeerInterface*		GetRakPeer() { return m_rakPeer; }
	const RakNet::RakPeerInterface*	GetRakPeer() const { return m_rakPeer; }

	static PeerID					CreatePeerID();

private:
	RakNet::RakPeerInterface*	m_rakPeer;
	PeerID						m_peerID;
	static std::atomic<PeerID>	m_sCounter;
};

DECLARE_PTR_PROXY(Peer)

XTOOLS_NAMESPACE_END
