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
: m_payload(1024)
, m_address(RakNet::UNASSIGNED_SYSTEM_ADDRESS)
, m_rakNetGuid(RakNet::UNASSIGNED_RAKNET_GUID)
, m_peerID(0)
{
	
}


bool Message::IsValid() const
{
	return m_payload.GetSize() > 0;
}


const RakNet::SystemAddress& Message::GetSystemAddress() const
{
	return m_address;
}


RakNet::RakNetGUID Message::GetRakNetGUID() const
{
	return m_rakNetGuid;
}


PeerID Message::GetPeerID() const
{
	return m_peerID;
}


const byte* Message::GetData() const
{
	return m_payload.GetData();
}


uint32 Message::GetSize() const
{
	return m_payload.GetSize();
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
