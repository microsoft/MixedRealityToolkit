//////////////////////////////////////////////////////////////////////////
// BroadcastForwarder.h
//
// Listens for broadcast messages then forwards them to all the other connections
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class BroadcastForwarder : public RefCounted, public NetworkConnectionListener
{
public:
	BroadcastForwarder();

	void AddConnection(const NetworkConnectionPtr& connection);
	void RemoveConnection(const NetworkConnectionPtr& connection);

private:
	// NetworkConnectionListener Functions:
	virtual void OnMessageReceived(const NetworkConnectionPtr& connection, NetworkInMessage& message) XTOVERRIDE;

	struct ConnectionInfo
	{
		NetworkConnectionPtr	m_connection;
	};

	std::vector<ConnectionInfo>	m_connections;
	
};

DECLARE_PTR(BroadcastForwarder)

XTOOLS_NAMESPACE_END