//////////////////////////////////////////////////////////////////////////
// JoinSessionRequest.cpp
//
// Implementation of the new session request.
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "JoinSessionRequest.h"

XTOOLS_NAMESPACE_BEGIN

const std::string JoinSessionRequest::gMessageType("JoinSessionRequest");
const std::string JoinSessionRequest::gUserNameKey("userName");
const std::string JoinSessionRequest::gUserIDKey("userID");
const std::string JoinSessionRequest::gMuteStateKey("muteState");

JoinSessionRequest::JoinSessionRequest(const std::string& userName, UserID userID, bool muteState)
	: m_message(JSONMessage::Create(gMessageType))
{
	m_message->SetValue(gUserNameKey, userName);
	m_message->SetValue(gUserIDKey, userID);
    m_message->SetValue(gMuteStateKey, muteState);
}


JoinSessionRequest::JoinSessionRequest(const JSONMessagePtr& message)
	: m_message(message)
{

}


//static 
const std::string& JoinSessionRequest::GetMessageType()
{
	return gMessageType;
}


std::string JoinSessionRequest::ToJSONString() const
{
	return m_message->ToString();
}


std::string JoinSessionRequest::GetUserName() const
{
	return m_message->GetStringValue(gUserNameKey);
}


UserID JoinSessionRequest::GetUserID() const
{
	return m_message->GetIntValue(gUserIDKey);
}

bool JoinSessionRequest::GetMuteState() const
{
    return m_message->GetBoolValue(gMuteStateKey);
}

XTOOLS_NAMESPACE_END
