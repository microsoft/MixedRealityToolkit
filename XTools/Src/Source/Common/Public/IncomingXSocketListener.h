//////////////////////////////////////////////////////////////////////////
// IncomingConnectionListener.h
//
// Interface for objects that want to receive a callback when a new 
// incoming connection is made.  Use with ConnectionManager::AcceptConnections
// to start listening for incoming connection requests.  
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class IncomingXSocketListener XTABSTRACT
{
public:
	// Callback for when a new connection has been received from a remote machine on
	// the port that this listener registered for.  A ref-counted XSocket is provided;
	// if a reference to this connection is not retained when this function returns then the
	// connection is closed.  
	virtual void OnNewConnection(const XSocketPtr& newConnection) = 0;
};

XTOOLS_NAMESPACE_END