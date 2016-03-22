//////////////////////////////////////////////////////////////////////////
// NewSessionReply.cpp
//
// Implementation of the new session request.
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewSessionReply.h"

XTOOLS_NAMESPACE_BEGIN

const std::string NewSessionReply::gMessageType("NewSessionReply");
const std::string NewSessionReply::gSessionResult("result");
const std::string NewSessionReply::gSessionID("sessionID");
const std::string NewSessionReply::gSessionType("sessionType");
const std::string NewSessionReply::gSessionName("name");
const std::string NewSessionReply::gNewSessionAddress("address");
const std::string NewSessionReply::gNewSessionPort("port");
const std::string NewSessionReply::gFailureReason("failureReason");

NewSessionReply::NewSessionReply(uint32 sessionID, SessionType sessionType, const std::string& name, const std::string& address, uint16 port)
	: m_message(JSONMessage::Create(gMessageType))
{
	m_message->SetValue(gSessionResult, true);
	m_message->SetValue(gSessionID, sessionID);
	m_message->SetValue(gSessionType, sessionType);
	m_message->SetValue(gSessionName, name);
	m_message->SetValue(gNewSessionAddress, address);
	m_message->SetValue(gNewSessionPort, port);
}


NewSessionReply::NewSessionReply(const std::string& failureReason)
	: m_message(JSONMessage::Create(gMessageType))
{
	m_message->SetValue(gSessionResult, false);
	m_message->SetValue(gFailureReason, failureReason);
}


NewSessionReply::NewSessionReply(const JSONMessagePtr& message)
	: m_message(message)
{

}

//static 
const std::string& NewSessionReply::GetMessageType()
{
	return gMessageType;
}

std::string NewSessionReply::ToJSONString() const
{
	return m_message->ToString();
}


bool NewSessionReply::GetResult() const
{
    return m_message->GetBoolValue(gSessionResult);
}

uint32 NewSessionReply::GetSessionID() const
{
	return m_message->GetUIntValue(gSessionID);
}

SessionType NewSessionReply::GetSessionType() const
{
	return static_cast<SessionType>(m_message->GetIntValue(gSessionType));
}

std::string NewSessionReply::GetSessionName() const
{
    return m_message->GetStringValue(gSessionName);
}

std::string NewSessionReply::GetAddress() const
{
	return m_message->GetStringValue(gNewSessionAddress);
}

uint16 NewSessionReply::GetPort() const
{
    return static_cast<uint16>(m_message->GetIntValue(gNewSessionPort));
}

std::string NewSessionReply::GetFailureReason() const
{
	return m_message->GetStringValue(gFailureReason);
}

XTOOLS_NAMESPACE_END
