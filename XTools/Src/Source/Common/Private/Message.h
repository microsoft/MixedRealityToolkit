//////////////////////////////////////////////////////////////////////////
// Message.h
//
// 
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "Buffer.h"

XTOOLS_NAMESPACE_BEGIN

class XSocketImpl;
DECLARE_PTR(XSocketImpl)

class Message : public AtomicRefCounted
{
public:
	Message();

	// Returns false if the size is zero
	bool					IsValid() const;
	
	const XSocketImplPtr&	GetSocket() const;
	void					SetSocket(const XSocketImplPtr& socket);

	PeerID					GetPeerID() const;
	void					SetPeerID(PeerID peerID);

	const RakNet::SystemAddress& GetSystemAddress() const;
	void					SetSystemAddress(const RakNet::SystemAddress& sysAddress);

	const byte*				GetData() const;
	uint32					GetSize() const;
	void					SetData(const byte* buffer, uint32 size);

	// Return the ID of the message, which is the first byte of the payload
	byte					GetMessageID() const;

private:
	Buffer					m_payload;
	XSocketImplPtr			m_socket;
	PeerID					m_peerID;
	RakNet::SystemAddress	m_address;
};

DECLARE_PTR(Message)

XTOOLS_NAMESPACE_END
