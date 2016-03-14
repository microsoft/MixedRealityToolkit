//////////////////////////////////////////////////////////////////////////
// UserLeftSessionMsg.cpp
//
// Session message sent to notify users in a session that new user has left
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UserLeftSessionMsg.h"

XTOOLS_NAMESPACE_BEGIN

const std::string UserLeftSessionMsg::gMessageType("UserLeftSessionMsg");
const std::string UserLeftSessionMsg::gSessionIDKey("sessionID");
const std::string UserLeftSessionMsg::gUserIDKey("userID");


UserLeftSessionMsg::UserLeftSessionMsg(uint32 sessionID, UserID userID)
	: m_message(JSONMessage::Create(gMessageType))
{
	m_message->SetValue(gSessionIDKey, sessionID);
	m_message->SetValue(gUserIDKey, userID);
}


UserLeftSessionMsg::UserLeftSessionMsg(const JSONMessagePtr& message)
	: m_message(message)
{

}


//static 
const std::string& UserLeftSessionMsg::GetMessageType()
{
	return gMessageType;
}


std::string UserLeftSessionMsg::ToJSONString() const
{
	return m_message->ToString();
}


uint32 UserLeftSessionMsg::GetSessionID() const
{
	return m_message->GetUIntValue(gSessionIDKey);
}


UserID UserLeftSessionMsg::GetUserID() const
{
	return m_message->GetIntValue(gUserIDKey);
}

XTOOLS_NAMESPACE_END
