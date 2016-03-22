//////////////////////////////////////////////////////////////////////////
// XSocketListener.h
//
// Interface for objects to listen for events coming from a socket.  
// Must first be registered with XSocket::RegisterListener()
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class XSocketListener XTABSTRACT
{
public:

	enum FailureReason
	{
		Failure_CannotConnect,
		Failure_AlreadyConnected,
		Failure_MaxConnectionsReached,
		Failure_Unknown
	};
	
	// Callback when a pending connection request succeeds.  Once this event is received it is possible 
	// to send and receive messages
	virtual void OnConnected(const XSocketPtr& socket) = 0;

	// Callback when a connection request fails.  Provides a hint as to why the attempt failed
	virtual void OnConnectionFailed(const XSocketPtr& socket, FailureReason reason) = 0;

	// Callback for when the remote machine closes the connection
	virtual void OnDisconnected(const XSocketPtr& socket) = 0;

	// Callback for when a message is received.  The first byte of the message is the type ID of the message, and
	// should be used to determine what to do with the rest of the message
	virtual void OnMessageReceived(const XSocketPtr& socket, const byte* message, uint32 messageLength) = 0;

	// Same as OnMessageReceived, but called from the network thread.  All incoming messages call back to both
	// OnMessageReceived and OnMessageReceivedAsync to allow listeners to forward the message in the appropriate
	// way to multiple different systems
	virtual void OnMessageReceivedAsync(const XSocketPtr& socket, const byte* message, uint32 messageLength) = 0;
};

XTOOLS_NAMESPACE_END
