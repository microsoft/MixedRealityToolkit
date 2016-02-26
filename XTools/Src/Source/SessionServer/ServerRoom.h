//////////////////////////////////////////////////////////////////////////
// ServerRoom.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <Private/Buffer.h>

XTOOLS_NAMESPACE_BEGIN

class ServerRoom : public AtomicRefCounted, public SyncObject
{
public:
	typedef std::shared_ptr<Buffer> BufferPtr;

	ServerRoom();

	XStringPtr	GetName() const;
	RoomID GetID() const;

	bool AddAnchor(const std::string& name, const BufferPtr& data);

	void RemoveAnchor(const std::string& name);

	BufferPtr GetAnchorData(const std::string& name);

private:
	std::map<std::string, BufferPtr> m_anchors;
	SyncString			m_name;
	SyncLong			m_id;
};

DECLARE_PTR(ServerRoom)

XTOOLS_NAMESPACE_END
