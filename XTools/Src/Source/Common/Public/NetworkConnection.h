//////////////////////////////////////////////////////////////////////////
// NetworkConnection.h
//
// API for a network connection exposed through the SWIG wrappers
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

#if !defined(SWIG)
class XSocket;
DECLARE_PTR(XSocket)
#endif

class NetworkConnection : public AtomicRefCounted
{
public:
	virtual bool IsConnected() const = 0;

	/// Get unique connection GUID and pass back to user
	virtual ConnectionGUID GetConnectionGUID() const = 0;

	/// Send the given message with specific settings.  Set releaseMessage to true to release the NetworkMessage
	/// and return it to the message pool for later reuse.  
	virtual void Send(const NetworkOutMessagePtr& msg, MessagePriority priority = MessagePriority::Medium, MessageReliability reliability = MessageReliability::ReliableOrdered, MessageChannel channel = MessageChannel::Default, bool releaseMessage = true) = 0;

	/// Send the given message to the device of devices of a particular user.  
	/// \param user The User to send the message to
	/// \param deviceRole Specifies which of the user's devices to send the message to based on the devices' roles.  A roll of "Unspecified" will cause to the message to be sent to all of the user's devices.  
	virtual void SendTo(const UserPtr& user, ClientRole deviceRole, const NetworkOutMessagePtr& msg, MessagePriority priority = MessagePriority::Medium, MessageReliability reliability = MessageReliability::ReliableOrdered, MessageChannel channel = MessageChannel::Default, bool releaseMessage = true) = 0;

	/// Instruct the recipient to sent this messages on to all other connected peers
	virtual void Broadcast(const NetworkOutMessagePtr& msg, MessagePriority priority = MessagePriority::Medium, MessageReliability reliability = MessageReliability::ReliableOrdered, MessageChannel channel = MessageChannel::Default, bool releaseMessage = true) = 0;

	/// Register to receive callbacks for a specific message type.  Multiple listeners can register for the same message type, 
	/// and the same listener can register for multiple message types.  
	/// The generated wrapper for this class will store a reference to the listener so it is not garbage collected.  
	/// Native code should use AddListenerWithReceipt.  
	/// Message types must be greater than MessageID::Start.   
	virtual void AddListener(byte messageType, NetworkConnectionListener* newListener) = 0;

	/// Unregister the given listener from receiving callbacks about incoming messages of the given type.  
	/// The generated wrapper for this class will release its reference to the given listener when this is called.  
	virtual void RemoveListener(byte messageType, NetworkConnectionListener* oldListener) = 0;

	/// Same as RegisterCallback, but the callbacks will be called on the network thread as soon as they arrive,
	/// rather than being queued and processed later on the main thread.  
	/// Returns true if successful, false if someone else has already registered for this messageType.  
	/// NOTE: Registering and Unregistering for Async callbacks must be done manually, and does not use receipts.
	/// The is because users need to unregister BEFORE their destruction is complete to avoid race condition errors
	virtual bool RegisterAsyncCallback(byte messageType, NetworkConnectionListener* cb) = 0;

	/// Manually unregister for async callbacks
	virtual void UnregisterAsyncCallback(byte messageType) = 0;

	/// Create a message that is intended for a single destination.  This message is pulled
	/// from a pools of messages to try to avoid allocations.  You can return the message
	/// to the pool manually by passing it to ReturnMessage(), or by setting the releaseMessage
	/// parameter to true when calling Send().  
	virtual NetworkOutMessagePtr CreateMessage(byte messageType) = 0;

	/// Return a message to the message pool.  
	virtual void ReturnMessage(const NetworkOutMessagePtr& msg) = 0;

	/// Force the connection to end if it is currently connected.  
	/// Will trigger OnDisconnected callbacks on listeners
	virtual void Disconnect() = 0;

	/// Returns the address of the remote machine that this object is connected to.  
	/// If unconnected, returns null.  
	virtual XStringPtr GetRemoteAddress() const = 0;

#if !defined(SWIG)
	virtual const XSocketPtr& GetSocket() const = 0;
	virtual void SetSocket(const XSocketPtr& socket) = 0;
#endif
};

DECLARE_PTR(NetworkConnection)

XTOOLS_NAMESPACE_END
