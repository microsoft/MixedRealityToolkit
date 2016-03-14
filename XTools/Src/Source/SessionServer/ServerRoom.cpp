//////////////////////////////////////////////////////////////////////////
// ServerRoom.cpp
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ServerRoom.h"

XTOOLS_NAMESPACE_BEGIN

ServerRoom::ServerRoom()
	: m_name(new XString("UnnamedRoom"))
	, m_id(Room::kInvalidRoomID)
{
	AddMember(&m_name, "Name");
	AddMember(&m_id, "ID");
	AddMember(&m_anchors, "Anchors");
}


XStringPtr ServerRoom::GetName() const
{
	return m_name;
}


RoomID ServerRoom::GetID() const
{
	return m_id;
}


void ServerRoom::SetAnchor(const std::string& name, const BufferPtr& data)
{
	m_anchors.SetAnchor(name, data);
}


void ServerRoom::RemoveAnchor(const std::string& name)
{
	m_anchors.RemoveAnchor(name);
}


BufferPtr ServerRoom::GetAnchorData(const std::string& name)
{
	return m_anchors.GetAnchorData(name);
}

XTOOLS_NAMESPACE_END
