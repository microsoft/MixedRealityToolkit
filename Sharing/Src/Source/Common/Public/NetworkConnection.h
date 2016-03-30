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
	/// \param msg The message to send
	/// \param priority Determines the priority this message has relative to other outgoing messages.  Higher priority messages are sent first
	/// \param reliability Determines what guarantees should be placed on the message's delivery
	/// \param channel Messages sent in the same channel will be ordered relative to each other if sent with Ordered or Reliable reliability settings
	/// \param releaseMessage Set to true to return this NetworkOutMessage to the pool.  Set to false if intend to send the message again before it is released. 
	virtual void Send(const NetworkOutMessagePtr& msg, MessagePriority priority = MessagePriority::Medium, MessageReliability reliability = MessageReliability::ReliableOrdered, MessageChannel channel = MessageChannel::Default, bool releaseMessage = true) = 0;

	/// Send the given message to the device of devices of a particular user.  
	/// \param user The User to send the message to
	/// \param deviceRole Specifies which of the user's devices to send the message to based on the devices' roles.  A roll of "Unspecified" will cause to the message to be sent to all of the user's devices.  
	/// \param msg The message to send
	/// \param priority Determines the priority this message has relative to other outgoing messages.  Higher priority messages are sent first
	/// \param reliability Determines what guarantees should be placed on the message's delivery
	/// \param channel Messages sent in the same channel will be ordered relative to each other if sent with Ordered or Reliable reliability settings
	/// \param releaseMessage Set to true to return this NetworkOutMessage to the pool.  Set to false if intend to send the message again before it is released. 
	virtual void SendTo(const UserPtr& user, ClientRole deviceRole, const NetworkOutMessagePtr& msg, MessagePriority priority = MessagePriority::Medium, MessageReliability reliability = MessageReliability::ReliableOrdered, MessageChannel channel = MessageChannel::Default, bool releaseMessage = true) = 0;

	/// Instruct the recipient to send this messages on to all other connected peers
	/// \param msg The message to send
	/// \param priority Determines the priority this message has relative to other outgoing messages.  Higher priority messages are sent first
	/// \param reliability Determines what guarantees should be placed on the message's delivery
	/// \param channel Messages sent in the same channel will be ordered relative to each other if sent with Ordered or Reliable reliability settings
	/// \param releaseMessage Set to true to return this NetworkOutMessage to the pool.  Set to false if intend to send the message again before it is released. 
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


	virtual void AddListenerAsync(byte messageType, NetworkConnectionListener* newListener) = 0;

	virtual void RemoveListenerAsync(byte messageType, NetworkConnectionListener* oldListener) = 0;

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
