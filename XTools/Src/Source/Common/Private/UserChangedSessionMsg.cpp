//////////////////////////////////////////////////////////////////////////
// UserChangedSessionMsg.cpp
//
// Session message sent to notify users in a session that a remote user's state has changed.
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UserChangedSessionMsg.h"

XTOOLS_NAMESPACE_BEGIN

const std::string UserChangedSessionMsg::gMessageType("UserChangedSessionMsg");
const std::string UserChangedSessionMsg::gSessionIDKey("sessionID");
const std::string UserChangedSessionMsg::gUserNameKey("userName");
const std::string UserChangedSessionMsg::gUserIDKey("userID");
const std::string UserChangedSessionMsg::gUserMuteStateKey("muteState");


UserChangedSessionMsg::UserChangedSessionMsg(uint32 sessionID, const std::string& userName, UserID userID, bool muteState)
	: m_message(JSONMessage::Create(gMessageType))
{
	m_message->SetValue(gSessionIDKey, sessionID);
	m_message->SetValue(gUserNameKey, userName);
    m_message->SetValue(gUserIDKey, userID);
    m_message->SetValue(gUserMuteStateKey, muteState);
}


UserChangedSessionMsg::UserChangedSessionMsg(const JSONMessagePtr& message)
	: m_message(message)
{

}


//static 
const std::string& UserChangedSessionMsg::GetMessageType()
{
	return gMessageType;
}


std::string UserChangedSessionMsg::ToJSONString() const
{
	return m_message->ToString();
}


uint32 UserChangedSessionMsg::GetSessionID() const
{
	return m_message->GetUIntValue(gSessionIDKey);
}


std::string UserChangedSessionMsg::GetSessionUserName() const
{
	return m_message->GetStringValue(gUserNameKey);
}


UserID UserChangedSessionMsg::GetSessionUserID() const
{
	return m_message->GetUIntValue(gUserIDKey);
}


bool UserChangedSessionMsg::GetSessionUserMuteState() const
{
    return m_message->GetBoolValue(gUserMuteStateKey);
}

XTOOLS_NAMESPACE_END
