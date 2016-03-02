//////////////////////////////////////////////////////////////////////////
// XSocketImpl.h
//
// Implementation of the Connection interface.  Allows us to limit the 
// exposure of the RakNet APIs to code outside the common library
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "Message.h"

XTOOLS_NAMESPACE_BEGIN

class XSocketManagerImpl;

class XSocketImpl : public XSocket
{
public:
	// Initiate a connection to a remote peer
	XSocketImpl(
		XSocketManagerImpl* manager,
		SocketID id, 
		const PeerPtr& peer, 
		const RakNet::SystemAddress& address = RakNet::UNASSIGNED_SYSTEM_ADDRESS, 
		RakNet::RakNetGUID guid = RakNet::UNASSIGNED_RAKNET_GUID);

	virtual SocketID	GetID() const XTOVERRIDE;

	// XSocket Functions:

	// Send message to the remote peer
	virtual void		Send(const byte* message, uint32 messageSize, MessagePriority priority, MessageReliability reliability, MessageChannel channel) XTOVERRIDE;

	// Register the given listener to receive callbacks when events happen on this XSocket.
	// Releasing the given Receipt will automatically unregister the listener
	virtual ReceiptPtr	RegisterListener(XSocketListener* listener) XTOVERRIDE;

	// Returns the (simplified) status of the connection
	virtual Status		GetStatus() const XTOVERRIDE;

	virtual bool		IsConnected() const XTOVERRIDE;

	virtual std::string GetRemoteSystemName() const XTOVERRIDE;

	// Local Functions:

	// Returns true if this message was consumed by this function
	bool				OnReceiveMessage(const MessageConstPtr& msg);

	const PeerPtr&		GetPeer();
	PeerConstPtr		GetPeer() const;

	RakNet::SystemAddress GetAddress() const;
	void				SetAddress(const RakNet::SystemAddress& newAddress);

	const RakNet::RakNetGUID& GetRakNetGUID() const;
	void				SetRakNetGUID(const RakNet::RakNetGUID& guid);

	void				SetRegistrationReceipt(const ReceiptPtr& receipt);

	XSocketManagerImpl*		GetSocketManager(); 

private:
	void				UnregisterListener(XSocketListener* listener);

	void				OnConnected();
	void				OnLostConnection();
	void				OnConnectionAttemptFailed(byte failureID);

	SocketID	                m_id;
	PeerPtr						m_peer;
	XSocketListener*			m_listener;
	RakNet::SystemAddress		m_address;
	RakNet::RakNetGUID			m_raknetGuid;
	Status						m_status;
	ReceiptPtr					m_receipt;
	XSocketManagerImpl*			m_manager;
};

DECLARE_PTR(XSocketImpl)

XTOOLS_NAMESPACE_END