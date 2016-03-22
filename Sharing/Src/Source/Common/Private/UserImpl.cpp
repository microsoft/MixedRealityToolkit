//////////////////////////////////////////////////////////////////////////
// UserImpl.cpp
//
// Implementation of the Session List
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UserImpl.h"

XTOOLS_NAMESPACE_BEGIN

// gcc seems to require a .cpp file to contain the definition of the constant
#if defined(XTOOLS_PLATFORM_OSX)
const UserID User::kInvalidUserID;
#endif

static const utility::char_t* kMemberName = U("name");
static const utility::char_t* kMemberId = U("id");
static const utility::char_t* kMuteId = U("muteState");

UserImpl::UserImpl()
	: m_name(new XString("Invalid User"))
	, m_id(User::kInvalidUserID)
	, m_muteState(false)
{

}

UserImpl::UserImpl(const std::string& name, UserID id, bool muteState)
	: m_name(new XString(name))
	, m_id(id)
	, m_muteState(muteState)
{

}

const XStringPtr& UserImpl::GetName() const
{
	return m_name;
}

UserID UserImpl::GetID() const
{
	return m_id;
}

bool UserImpl::IsValid() const
{
	return (m_id != kInvalidUserID && m_name->GetLength() > 0);
}

bool UserImpl::GetMuteState() const
{
	return m_muteState;
}

void UserImpl::SetID(UserID id)
{
	m_id = id;
}

void UserImpl::SetName(const XStringPtr& name)
{
	XTASSERT(name);
	m_name = name;
}

void UserImpl::SetMuteState(bool muteState)
{
	m_muteState = muteState;
}

web::json::value UserImpl::AsJSON() const
{
	web::json::value result = web::json::value::object();
	result[kMemberName] = web::json::value::string(utility::conversions::to_string_t(m_name->GetString()));
	result[kMemberId] = web::json::value(m_id);
	result[kMuteId] = web::json::value(m_muteState);
	return result;
}

UserImpl* UserImpl::FromJSON(const web::json::object& object)
{
	UserImpl* pu = new UserImpl();
	pu->m_name = new XString( utility::conversions::to_utf8string(object.at(kMemberName).as_string()) );
	pu->m_id = object.at(kMemberId).as_number().to_uint32();
	pu->m_muteState = object.at(kMuteId).as_bool();
	return pu;
}

XTOOLS_NAMESPACE_END