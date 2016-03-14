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

class Message : public AtomicRefCounted
{
public:
	Message();
	Message(uint32 messageSize);

	// Returns false if the size is zero
	bool							IsValid() const;
	
	const RakNet::SystemAddress&	GetSystemAddress() const;
	RakNet::RakNetGUID				GetRakNetGUID() const;

	// The ID of the PeerPtr object that this message came from
	PeerID							GetPeerID() const;

	const byte*						GetData() const;
	uint32							GetSize() const;

	// Return the ID of the message, which is usually the first byte of
	// the payload
	byte							GetMessageID() const;

private:
	friend class MessageQueue;

	Buffer					m_payload;
	RakNet::SystemAddress	m_address;
	RakNet::RakNetGUID		m_rakNetGuid;
	PeerID					m_peerID;
};

DECLARE_PTR(Message)

XTOOLS_NAMESPACE_END
