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
		Accept,
		DiscoveryResponse
	};

	explicit Command(Type commandType, const XSocketImplPtr& socket) 
		: m_socket(socket)
		, m_peerID(kInvalidPeerID)
		, m_port(0)
		, m_maxConnections(0)
		, m_type(commandType)
		, m_role(SystemRole::PrimaryClientRole) {}

	Command(Type commandType, PeerID peerID, uint16 port, uint16 maxConnections, SystemRole role)
		: m_peerID(peerID)
		, m_port(port)
		, m_maxConnections(maxConnections) 
		, m_type(commandType) 
		, m_role(role) {}

	const XSocketImplPtr& GetSocket() const { return m_socket; }
	PeerID GetPeerID() const { return m_peerID; }
	uint16 GetPort() const { return m_port; }
	uint16 GetMaxConnections() const { return m_maxConnections; }
	Type GetType() const { return m_type; }
	SystemRole GetRole() const { return m_role; }

private:
	XSocketImplPtr	m_socket;
	PeerID m_peerID;
	uint16 m_port;
	uint16 m_maxConnections;
	Type m_type;
	SystemRole m_role;
};
DECLARE_PTR(Command)

inline CommandPtr CreateOpenCommand(const XSocketImplPtr& socket)
{
	return new Command(Command::Type::Open, socket);
}

inline CommandPtr CreateAcceptCommand(PeerID peerID, uint16 port, uint16 maxConnections)
{
	return new Command(Command::Type::Accept, peerID, port, maxConnections, SystemRole::PrimaryClientRole);
}

inline CommandPtr CreateDiscoveryResponseCommand(PeerID peerID, uint16 port, SystemRole role)
{
	return new Command(Command::Type::DiscoveryResponse, peerID, port, 2, role);
}

XTOOLS_NAMESPACE_END
