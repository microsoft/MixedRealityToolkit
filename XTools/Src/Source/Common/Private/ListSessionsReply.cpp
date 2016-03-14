//////////////////////////////////////////////////////////////////////////
// ListSessionsReply.cpp
//
// 
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ListSessionsReply.h"
#include "SessionDescriptorImpl.h"

XTOOLS_NAMESPACE_BEGIN

const std::string ListSessionsReply::gMessageType("ListSessionsReply");
const std::string ListSessionsReply::gSessionListKey("sessionList");

ListSessionsReply::ListSessionsReply()
{

}

ListSessionsReply::ListSessionsReply(const JSONMessagePtr& message)
{
	// now walk the parse tree and dig out the properties
	web::json::value sessionList = message->GetObjectValue(gSessionListKey);

	for (auto iter = sessionList.as_array().begin(); iter != sessionList.as_array().end(); ++iter)
	{
		if (!iter->is_null())
		{
			SessionDescriptorImpl* session;
			session = SessionDescriptorImpl::FromJSON(iter->as_object());
			m_sessions.push_back(session);
		}
	}
}

//static 
const std::string& ListSessionsReply::GetMessageType()
{
	return gMessageType;
}

std::string ListSessionsReply::ToJSONString() const
{
	web::json::value content = web::json::value::object();

	// Set the message type:
	utility::string_t keyWString = utility::conversions::to_string_t(kSessionMessageTypeKey);
	utility::string_t valueWString = utility::conversions::to_string_t(gMessageType);
	content[keyWString] = web::json::value(valueWString);

	// add the sessions
	web::json::value jSessions = web::json::value::array(m_sessions.size());
	int idx = 0;
	for (auto iter = m_sessions.begin(); iter != m_sessions.end(); iter++)
	{
		jSessions[idx++] = (*iter)->AsJSON();
	}

	utility::string_t sessionListKey16 = utility::conversions::to_string_t(gSessionListKey);
	content[sessionListKey16] = jSessions;

	std::string str = utility::conversions::to_utf8string(content.serialize());
	return str;
}


int32 ListSessionsReply::GetSessionCount() const
{
	return (int32)m_sessions.size();
}

SessionDescriptor* ListSessionsReply::GetSessionDescriptor(const int32 i) const
{
	return m_sessions[i].get();
}


void ListSessionsReply::SetSessionCount(int count)
{
	m_sessionCount = count;
	m_sessions.resize(count, NULL);
}

void ListSessionsReply::SetSessionDescriptor(const int i, SessionDescriptorImplPtr psess)
{
	XTASSERT(i >= 0 && i < m_sessionCount);

	m_sessions[i] = psess;
}


XTOOLS_NAMESPACE_END
