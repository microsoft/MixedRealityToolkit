//////////////////////////////////////////////////////////////////////////
// ReplySessionListImpl.cpp
//
// Implementation of the Session List
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SessionDescriptorImpl.h"
#include <string>
#include <Private/json/cpprest/asyncrt_utils.h>

XTOOLS_NAMESPACE_BEGIN

using namespace std;

static const utility::char_t* kSessionName = U("name");
static const utility::char_t* kSessionID = U("id");
static const utility::char_t* kSessionType = U("type");
static const utility::char_t* kSessionMemeberCount = U("member count");
static const utility::char_t* kSessionMemberList = U("member list");
static const utility::char_t* kSessionAddress = U("address");
static const utility::char_t* kSessionPort = U("port");

std::string SessionDescriptorImpl::GetName() const
{
	return utility::conversions::to_utf8string(m_name);
}

uint32 SessionDescriptorImpl::GetID() const
{
	return m_id;
}

SessionType SessionDescriptorImpl::GetSessionType() const
{
	return m_sessionType;
}

int SessionDescriptorImpl::GetUserCount() const
{
	return m_userCount;
}

User* SessionDescriptorImpl::GetUser(const int i) const
{
	XTASSERT(i >= 0 && i < m_userCount);

	return m_users[i].get();
}

std::string SessionDescriptorImpl::GetAddress() const
{
	return m_address;
}


uint16 SessionDescriptorImpl::GetPortID() const
{
	return m_port;
}

void SessionDescriptorImpl::SetName(const std::string& str)
{
	m_name = utility::conversions::to_string_t(str);
}

void SessionDescriptorImpl::SetID(uint32 sessionID)
{
	m_id = sessionID;
}

void SessionDescriptorImpl::SetSessionType(const SessionType sess)
{
	m_sessionType = sess;
}

void SessionDescriptorImpl::SetUserCount(const int count)
{
	m_userCount = count;
	m_users.resize(count, NULL);
}

void SessionDescriptorImpl::SetUser(const int i, UserImplPtr puser)
{
	XTASSERT(i >= 0 && i < m_userCount);

	m_users[i] = puser;
}

void SessionDescriptorImpl::SetAddress(const std::string& address)
{
	m_address = address;
}


void SessionDescriptorImpl::SetPortID(uint16 port)
{
	m_port = port;
}

web::json::value SessionDescriptorImpl::AsJSON() const
{
	web::json::value result = web::json::value::object();
	result[kSessionName] = web::json::value::string(utility::conversions::to_string_t(m_name));
	result[kSessionID] = web::json::value(m_id);
	result[kSessionType] = web::json::value((uint32)m_sessionType);
	result[kSessionMemeberCount] = web::json::value::number(m_userCount);
	result[kSessionAddress] = web::json::value::string(utility::conversions::to_string_t(m_address));
	result[kSessionPort] = web::json::value((uint32)m_port);

	// now the array of members
	web::json::value jMembers = web::json::value::array(m_users.size());
	int idx = 0;
	for (auto iter = m_users.begin(); iter != m_users.end(); iter++)
	{
		jMembers[idx++] = (*iter)->AsJSON();
	}
	result[kSessionMemberList] = jMembers;

	return result;
}

SessionDescriptorImpl* SessionDescriptorImpl::FromJSON(const web::json::object& object)
{
	SessionDescriptorImpl* sd = new SessionDescriptorImpl();
	sd->m_name = object.at(kSessionName).as_string();
	sd->m_id = object.at(kSessionID).as_number().to_uint32();
	sd->m_sessionType = (SessionType)object.at(kSessionType).as_number().to_uint32();
	sd->m_userCount = object.at(kSessionMemeberCount).as_integer();
	sd->m_address = utility::conversions::to_utf8string(object.at(kSessionAddress).as_string());
	sd->m_port = (uint16)object.at(kSessionPort).as_number().to_uint32();

	// get the array of members
	web::json::value mems = object.at(kSessionMemberList);
	for (auto iter = mems.as_array().begin(); iter != mems.as_array().end(); ++iter)
	{
		if (!iter->is_null())
		{
			UserImpl* pu;
			pu = UserImpl::FromJSON(iter->as_object());
			sd->m_users.push_back(pu);
		}
	}

	return sd;
}

XTOOLS_NAMESPACE_END