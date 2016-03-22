//////////////////////////////////////////////////////////////////////////
// XSocket.h
//
// Represents a network socket connection to a single remote machine.  All network
// communications should be sent and received through here.  
// XSocket is reference counted, so only access through XSocketPtr.
// Once there are no more references, the socket will automatically close.  
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN


// Forward declaration of the interface to receive events from this socket
class XSocketListener;

class XSocket : public AtomicRefCounted, public Reflection::XTObject
{
	XTOOLS_REFLECTION_DECLARE(XSocket)

public:
	enum Status
	{
		Disconnected = 0,	// Can happen if the remote machine terminates the connection
		Disconnecting,		// A disconnection is pending.  
		Connecting,			// A connection is pending
		Connected			// The connection is successful and active
	};

	// Get unique socket ID
	virtual SocketID GetID() const = 0;

	// Send message to the remote peer
	virtual void Send(const byte* message, uint32 messageSize, MessagePriority priority, MessageReliability reliability, MessageChannel channel) = 0;

	// Register the given listener to receive callbacks when events happen on this socket.
	// Releasing the given Receipt will automatically unregister the listener
	virtual ReceiptPtr RegisterListener(XSocketListener* listener) = 0;

	// Returns the (simplified) status of the socket
	virtual Status GetStatus() const = 0;

	// Shortcut for GetStatus == Connected
	virtual bool IsConnected() const = 0;

	virtual const std::string& GetRemoteSystemName() const = 0;

	virtual uint16 GetRemoteSystemPort() const = 0;
};

DECLARE_PTR(XSocket)

XTOOLS_NAMESPACE_END
