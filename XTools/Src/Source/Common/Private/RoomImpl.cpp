//////////////////////////////////////////////////////////////////////////
// RoomImpl.cpp
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RoomImpl.h"

XTOOLS_NAMESPACE_BEGIN

RoomImpl::RoomImpl()
	: m_name(new XString("UnnamedRoom"))
	, m_id(kInvalidRoomID)
{
	SetupMemberBinding();
}


RoomImpl::RoomImpl(const XStringPtr& name, RoomID id)
	: m_name(name)
	, m_id(id)
{
	SetupMemberBinding();
}


XStringPtr RoomImpl::GetName() const
{
	return m_name;
}


RoomID RoomImpl::GetID() const
{
	return m_id;
}


void RoomImpl::SetupMemberBinding()
{
	AddMember(&m_name, "Name");
	AddMember(&m_id, "ID");
}

XTOOLS_NAMESPACE_END
