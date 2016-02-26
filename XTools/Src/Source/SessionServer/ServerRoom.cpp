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
}


XStringPtr ServerRoom::GetName() const
{
	return m_name;
}


RoomID ServerRoom::GetID() const
{
	return m_id;
}


bool ServerRoom::AddAnchor(const std::string& name, const BufferPtr& data)
{
	auto mapIter = m_anchors.find(name);
	if (mapIter == m_anchors.end())
	{
		m_anchors[name] = data;
		return true;
	}
	else
	{
		return false;
	}
}


void ServerRoom::RemoveAnchor(const std::string& name)
{
	auto mapIter = m_anchors.find(name);
	if (mapIter != m_anchors.end())
	{
		m_anchors.erase(mapIter);
	}
}


ServerRoom::BufferPtr ServerRoom::GetAnchorData(const std::string& name)
{
	auto mapIter = m_anchors.find(name);
	if (mapIter != m_anchors.end())
	{
		return mapIter->second;
	}
	else
	{
		return nullptr;
	}
}

XTOOLS_NAMESPACE_END
