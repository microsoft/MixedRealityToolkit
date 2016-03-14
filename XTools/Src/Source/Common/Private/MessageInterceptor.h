//////////////////////////////////////////////////////////////////////////
// MessageInterceptor.h
//
// Shim to allow NetworkConnections to intercept messages of specific 
// types on the network thread
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class MessageInterceptor : public AtomicRefCounted
{
public:
	MessageInterceptor(const PeerPtr& peer);

	/// \return True if this interceptor will handle the packet
	bool OnReceive(RakNet::Packet *packet);

	const PeerPtr& GetPeer();

protected:
	// User code inheriting from this class should implement this class to check the packets to 
	// see if they want to handle it.  Return true if your handling it yourself, false if the message
	// should continue to be routed to the appropriate listener
	virtual bool HandlePacket(RakNet::RakNetGUID guid, const byte* packetData, uint32 packetSize) = 0;

private:
	PeerPtr						m_peer;
	Mutex						m_mutex;
};

DECLARE_PTR(MessageInterceptor)

XTOOLS_NAMESPACE_END