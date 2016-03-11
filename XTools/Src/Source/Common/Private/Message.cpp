//////////////////////////////////////////////////////////////////////////
// Message.cpp
//
// 
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Message.h"


XTOOLS_NAMESPACE_BEGIN

Message::Message(SocketID socketID)
	: m_payload(0)
	, m_socketID(socketID)
	, m_peerID(kInvalidPeerID)
	, m_address(RakNet::UNASSIGNED_SYSTEM_ADDRESS)
{
	
}


Message::Message(SocketID socketID, PeerID peerID, const RakNet::SystemAddress& address, uint32 messageSize)
	: m_payload(messageSize)
	, m_socketID(socketID)
	, m_peerID(peerID)
	, m_address(address)
{

}


bool Message::IsValid() const
{
	return m_payload.GetSize() > 0;
}


SocketID Message::GetSocketID() const
{
	return m_socketID;
}


PeerID Message::GetPeerID() const
{
	return m_peerID;
}


const RakNet::SystemAddress& Message::GetSystemAddress() const
{
	return m_address;
}


const byte* Message::GetData() const
{
	return m_payload.GetData();
}


uint32 Message::GetSize() const
{
	return m_payload.GetSize();
}


void Message::SetData(const byte* buffer, uint32 size)
{
	m_payload.Set(buffer, size);
}


void Message::AppendData(const byte* buffer, uint32 size)
{
	m_payload.Append(buffer, size);
}


byte Message::GetMessageID() const
{
	if (m_payload.GetSize() == 0)
	{
		return 255;
	}

	if (m_payload.GetData()[0] == ID_TIMESTAMP)
	{
		XTASSERT(m_payload.GetSize() > sizeof(RakNet::MessageID) + sizeof(RakNet::Time));
		return m_payload.GetData()[sizeof(RakNet::MessageID) + sizeof(RakNet::Time)];
	}
	else
	{
		return m_payload.GetData()[0];
	}
}

XTOOLS_NAMESPACE_END
