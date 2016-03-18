//////////////////////////////////////////////////////////////////////////
// NetworkThreadCommands.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

//////////////////////////////////////////////////////////////////////////
// Abstract base class for commands sent to the network thread
class Command : public AtomicRefCounted
{
public:
	enum Type
	{
		Open = 0,
		Accept
	};

	explicit Command(const XSocketImplPtr& socket) 
		: m_socket(socket)
		, m_peerID(kInvalidPeerID)
		, m_port(0)
		, m_maxConnections(0)
		, m_type(Open) {}

	Command(PeerID peerID, uint16 port, uint16 maxConnections)
		: m_peerID(peerID)
		, m_port(port)
		, m_maxConnections(maxConnections) 
		, m_type(Accept) {}

	const XSocketImplPtr& GetSocket() const { return m_socket; }
	PeerID GetPeerID() const { return m_peerID; }
	uint16 GetPort() const { return m_port; }
	uint16 GetMaxConnections() const { return m_maxConnections; }
	Type GetType() const { return m_type; }

private:
	XSocketImplPtr	m_socket;
	PeerID m_peerID;
	uint16 m_port;
	uint16 m_maxConnections;
	Type m_type;
};
DECLARE_PTR(Command)

XTOOLS_NAMESPACE_END
