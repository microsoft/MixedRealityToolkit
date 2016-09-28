//////////////////////////////////////////////////////////////////////////
// NewSessionRequest.cpp
//
// Implementation of the new session request.
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewSessionRequest.h"

XTOOLS_NAMESPACE_BEGIN

const std::string NewSessionRequest::gMessageType("NewSessionRequest");
const std::string NewSessionRequest::gSessionNameKey("Name");
const std::string NewSessionRequest::gSessionTypeKey("Type");

NewSessionRequest::NewSessionRequest()
	: m_message(JSONMessage::Create(gMessageType))
{
	SetSessionType(SessionType::ADHOC);
}


NewSessionRequest::NewSessionRequest(const JSONMessagePtr& message)
	: m_message(message)
{

}


//static 
const std::string& NewSessionRequest::GetMessageType()
{
	return gMessageType;
}


std::string NewSessionRequest::ToJSONString() const
{
	return m_message->ToString();
}


std::string NewSessionRequest::GetSessionName() const
{
	return m_message->GetStringValue(gSessionNameKey);
}


void NewSessionRequest::SetSessionName(const std::string& name)
{
	m_message->SetValue(gSessionNameKey, name);
}

SessionType NewSessionRequest::GetSessionType() const
{
	return (SessionType)m_message->GetIntValue(gSessionTypeKey);
}

void NewSessionRequest::SetSessionType(SessionType type)
{
	m_message->SetValue(gSessionTypeKey, (int32)type);
}


XTOOLS_NAMESPACE_END
