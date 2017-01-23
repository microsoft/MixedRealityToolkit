//////////////////////////////////////////////////////////////////////////
// XSocketManager.h
//
// Provides a common place to create connections, receive connections,
// and later look up connections to remote machines.
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <Public/Utils/Callback.h>
#include <Public/SystemRole.h>

XTOOLS_NAMESPACE_BEGIN

// Forward declaration of the interface to receive events when a remote machine connects to this machine
class IncomingXSocketListener;

class XSocketManager : public AtomicRefCounted, public Reflection::XTObject, public IUpdateable
{
	XTOOLS_REFLECTION_DECLARE(XSocketManager)

public:
	// Create a connection manager. 
	static ref_ptr<XSocketManager> Create();

	// Opens a connection to the given remote host.  The 'remoteName' can be either and IP address or a machine name.  
	virtual XSocketPtr OpenConnection(const std::string& remoteName, uint16 remotePort) = 0;

	// Opens a port on the local machine to listen for incoming connections.  When a remote client connects,
	// a XSocket instance is created for it and passed to the given listener.  Only 'maxConnections' clients
	// can connect at the same time.  
	virtual ReceiptPtr AcceptConnections(uint16 port, uint16 maxConnections, IncomingXSocketListener* listener) = 0;

	// Open a port to listen for discovery connections and respond with a description of this machine
	virtual ReceiptPtr AcceptDiscoveryPings(uint16 port, SystemRole role) = 0;

	// Returns the address of this machine as seen by the remote machine connected by the given socket
	virtual std::string GetLocalAddressForRemoteClient(const XSocketPtr& socket) const = 0;

	// Get a reference to an event that will get signalled when a message has arrived.  
	virtual Event& GetMessageArrivedEvent() = 0;

	// Returns a list of all the IP addresses for this machine
	static IPAddressList GetLocalMachineAddresses();
};

DECLARE_PTR(XSocketManager)

XTOOLS_NAMESPACE_END
