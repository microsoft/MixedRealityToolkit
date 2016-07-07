// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// SessionClosedMsg.cpp
// Message sent to notify clients that a session has been closed
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SessionClosedMsg.h"

XTOOLS_NAMESPACE_BEGIN

const std::string SessionClosedMsg::gMessageType("SessionClosedMsg");
const std::string SessionClosedMsg::gSessionIDKey("sessionID");


SessionClosedMsg::SessionClosedMsg(uint32 sessionID)
	: m_message(JSONMessage::Create(gMessageType))
{
	m_message->SetValue(gSessionIDKey, sessionID);
}


SessionClosedMsg::SessionClosedMsg(const JSONMessagePtr& message)
	: m_message(message)
{

}

//static 
const std::string& SessionClosedMsg::GetMessageType()
{
	return gMessageType;
}


std::string SessionClosedMsg::ToJSONString() const
{
	return m_message->ToString();
}


uint32 SessionClosedMsg::GetSessionID() const
{
	return m_message->GetUIntValue(gSessionIDKey);
}

XTOOLS_NAMESPACE_END
