//////////////////////////////////////////////////////////////////////////
// TunnelBridge.h
//
// Forward packets between two remote peers.  Uses its own connection mgr
// and runs on its own thread so that tunneled messages get forwarded
// asap, without having to wait for other
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <Common.h>
#include <Private/NetworkCommonPrivate.h>

XTOOLS_NAMESPACE_BEGIN

class TunnelBridge : public RefCounted, public NetworkConnectionListener
{
public:
	TunnelBridge(const NetworkConnectionPtr& serverConnection, const NetworkConnectionPtr& secondaryClientConnection);
	virtual ~TunnelBridge();

	bool IsConnected() const;

private:
	enum TunnelIndex
	{
		ServerIndex = 0,
		SecondaryClientIndex = 1,
		TunnelConnectionCount
	};

	// NetworkConnectionListener Functions:
	virtual void OnConnected(const NetworkConnectionPtr& connection) XTOVERRIDE;
	virtual void OnConnectFailed(const NetworkConnectionPtr& connection) XTOVERRIDE;
	virtual void OnDisconnected(const NetworkConnectionPtr& connection) XTOVERRIDE;
	virtual void OnMessageReceived(const NetworkConnectionPtr& connection, NetworkInMessage& message) XTOVERRIDE;

	void SendTunnelControlMessage(const NetworkConnectionPtr& connection, TunnelMsgType msgType) const;

	const NetworkConnectionPtr& GetOtherConnection(const NetworkConnectionPtr& connection);

	void SendConnectionMessages();

	NetworkConnectionPtr m_connections[TunnelConnectionCount];
};

DECLARE_PTR(TunnelBridge)

XTOOLS_NAMESPACE_END