//////////////////////////////////////////////////////////////////////////
// NetworkThreadCommands.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

//////////////////////////////////////////////////////////////////////////
// Abstract base class for commands sent to the network thread
class Command : public Reflection::XTObject, public AtomicRefCounted
{
	XTOOLS_REFLECTION_DECLARE(Command)
};
DECLARE_PTR(Command)


//////////////////////////////////////////////////////////////////////////
// Contains the data for commands to open a new connection
class OpenCommand : public Command
{
	XTOOLS_REFLECTION_DECLARE(OpenCommand)
public:
	explicit OpenCommand(const XSocketImplPtr& socket) : m_socket(socket) {}

	const XSocketImplPtr& GetSocket() { return m_socket; }

private:
	XSocketImplPtr m_socket;
};
DECLARE_PTR(OpenCommand)


//////////////////////////////////////////////////////////////////////////
// Contains the data for commands to start accepting connections from remote clients
class AcceptCommand : public Command
{
	XTOOLS_REFLECTION_DECLARE(AcceptCommand)
public:
	AcceptCommand(PeerID peerID, uint16 port, uint16 maxConnections)
		: m_peerID(peerID)
		, m_port(port)
		, m_maxConnections(maxConnections) {}

	PeerID GetPeerID() const { return m_peerID; }
	uint16 GetPort() const { return m_port; }
	uint16 GetMaxConnections() const { return m_maxConnections; }

private:
	PeerID m_peerID;
	uint16 m_port;
	uint16 m_maxConnections;
};
DECLARE_PTR(AcceptCommand)

XTOOLS_NAMESPACE_END
