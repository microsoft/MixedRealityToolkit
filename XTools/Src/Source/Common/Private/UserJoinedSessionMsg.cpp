//////////////////////////////////////////////////////////////////////////
// UserJoinedSessionMsg.cpp
//
// Session message sent to notify users in a session that new user has arrived
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UserJoinedSessionMsg.h"

XTOOLS_NAMESPACE_BEGIN

const std::string UserJoinedSessionMsg::gMessageType("UserJoinedSessionMsg");
const std::string UserJoinedSessionMsg::gSessionIDKey("sessionID");
const std::string UserJoinedSessionMsg::gUserNameKey("userName");
const std::string UserJoinedSessionMsg::gUserIDKey("userID");
const std::string UserJoinedSessionMsg::gUserMuteStateKey("muteState");


UserJoinedSessionMsg::UserJoinedSessionMsg(uint32 sessionID, const std::string& userName, UserID userID, bool muteState)
	: m_message(JSONMessage::Create(gMessageType))
{
	m_message->SetValue(gSessionIDKey, sessionID);
	m_message->SetValue(gUserNameKey, userName);
    m_message->SetValue(gUserIDKey, userID);
    m_message->SetValue(gUserMuteStateKey, muteState);
}


UserJoinedSessionMsg::UserJoinedSessionMsg(const JSONMessagePtr& message)
	: m_message(message)
{

}


//static 
const std::string& UserJoinedSessionMsg::GetMessageType()
{
	return gMessageType;
}


std::string UserJoinedSessionMsg::ToJSONString() const
{
	return m_message->ToString();
}


uint32 UserJoinedSessionMsg::GetSessionID() const
{
	return m_message->GetUIntValue(gSessionIDKey);
}


std::string UserJoinedSessionMsg::GetSessionUserName() const
{
	return m_message->GetStringValue(gUserNameKey);
}


UserID UserJoinedSessionMsg::GetSessionUserID() const
{
	return m_message->GetIntValue(gUserIDKey);
}


bool UserJoinedSessionMsg::GetSessionUserMuteState() const
{
    return m_message->GetBoolValue(gUserMuteStateKey);
}

XTOOLS_NAMESPACE_END
