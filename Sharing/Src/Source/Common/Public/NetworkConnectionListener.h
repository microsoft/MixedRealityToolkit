//////////////////////////////////////////////////////////////////////////
// NetworkConnectionListener.h
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

// Disable unused parameter warnings.  Pure virtual functions don't play nice with SWIG, but we still want 
// to see the names of the parameters to make reading the code clearer
#pragma warning( push )
#pragma warning( disable : 4100 ) 

XTOOLS_NAMESPACE_BEGIN

class NetworkConnection;
DECLARE_PTR(NetworkConnection)

XT_LISTENER_DECLARE(NetworkConnectionListener)

/// Interface class for objects that want to receive notifications from
/// NetworkConnections about connectivity changes and incoming messages.  
class NetworkConnectionListener XTABSTRACT : public Listener
{
public:
	virtual ~NetworkConnectionListener() {}

	/// Called from the \ref NetworkConnection this has registered with when a connection has been established with a remote peer
	/// \param connection The NetworkConnection this callback is coming from
	virtual void OnConnected(const NetworkConnectionPtr& connection) {}

	/// Called from the \ref NetworkConnection this has registered with when a connection attempt to a remote peer has failed
	/// \param connection The NetworkConnection this callback is coming from
	virtual void OnConnectFailed(const NetworkConnectionPtr& connection)  {}

	/// Called from the \ref NetworkConnection this has registered with when a connection to a remote peer has been lost or terminated
	/// \param connection The NetworkConnection this callback is coming from
	virtual void OnDisconnected(const NetworkConnectionPtr& connection) {}

	/// Called from the \ref NetworkConnection this has registered with when a message of the registered type arrives from the remote client
	/// \param connection The NetworkConnection this callback is coming from
	/// \param message The message received from the remote client
	virtual void OnMessageReceived(const NetworkConnectionPtr& connection, NetworkInMessage& message) {}
};

XTOOLS_NAMESPACE_END

#pragma warning( pop )
