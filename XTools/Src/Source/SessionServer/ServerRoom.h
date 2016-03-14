//////////////////////////////////////////////////////////////////////////
// ServerRoom.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <Private/Buffer.h>
#include "ServerAnchorList.h"

XTOOLS_NAMESPACE_BEGIN

class ServerRoom : public AtomicRefCounted, public SyncObject
{
public:
	ServerRoom();

	XStringPtr	GetName() const;
	RoomID GetID() const;

	void SetAnchor(const std::string& name, const BufferPtr& data);

	void RemoveAnchor(const std::string& name);

	BufferPtr GetAnchorData(const std::string& name);

private:
	SyncString			m_name;
	SyncLong			m_id;
	ServerAnchorList	m_anchors;
};

DECLARE_PTR(ServerRoom)

XTOOLS_NAMESPACE_END
