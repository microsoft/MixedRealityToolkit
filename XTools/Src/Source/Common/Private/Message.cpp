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

Message::Message()
	: m_payload(0)
	, m_socketID(kInvalidSocketID)
	, m_peerID(kInvalidPeerID)
	, m_address(RakNet::UNASSIGNED_SYSTEM_ADDRESS)
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


void Message::SetSocketID(SocketID socketID)
{
	m_socketID = socketID;
}


PeerID Message::GetPeerID() const
{
	return m_peerID;
}


void Message::SetPeerID(PeerID peerID)
{
	m_peerID = peerID;
}


const RakNet::SystemAddress& Message::GetSystemAddress() const
{
	return m_address;
}


void Message::SetSystemAddress(const RakNet::SystemAddress& sysAddress)
{
	m_address = sysAddress;
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
