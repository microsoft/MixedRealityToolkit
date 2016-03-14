//////////////////////////////////////////////////////////////////////////
// SessionAddedMsg.cpp
//
// Message sent to notify clients that a session has been added
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SessionAddedMsg.h"
#include "SessionDescriptorImpl.h"

XTOOLS_NAMESPACE_BEGIN

const std::string SessionAddedMsg::gMessageType("SessionAddedMsg");
const std::string SessionAddedMsg::gSessionKey("session");

SessionAddedMsg::SessionAddedMsg()
{

}

SessionAddedMsg::SessionAddedMsg(const JSONMessagePtr& message)
{
	// now walk the parse tree and dig out the properties
	web::json::value sessionObj = message->GetObjectValue(gSessionKey);

	m_session = SessionDescriptorImpl::FromJSON(sessionObj.as_object());
}

//static 
const std::string& SessionAddedMsg::GetMessageType()
{
	return gMessageType;
}


std::string SessionAddedMsg::ToJSONString() const
{
	web::json::value content = web::json::value::object();

	// Set the message type:
	utility::string_t keyWString = utility::conversions::to_string_t(kSessionMessageTypeKey);
	utility::string_t valueWString = utility::conversions::to_string_t(gMessageType);
	content[keyWString] = web::json::value(valueWString);

	utility::string_t sessionListKey16 = utility::conversions::to_string_t(gSessionKey);
	content[sessionListKey16] = m_session->AsJSON();

	std::string str = utility::conversions::to_utf8string(content.serialize());

	return str;
}


SessionDescriptor* SessionAddedMsg::GetSessionDescriptor() const
{
	return m_session.get();
}


void SessionAddedMsg::SetSessionDescriptor(SessionDescriptorImplPtr psess)
{
	m_session = psess;
}


XTOOLS_NAMESPACE_END
