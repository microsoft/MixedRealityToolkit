// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// NewSessionRequest.cpp
// Implementation of the new session request.
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NewSessionRequest.h"

XTOOLS_NAMESPACE_BEGIN

const std::string NewSessionRequest::gMessageType("NewSessionRequest");
const std::string NewSessionRequest::gSessionNameKey("Name");

NewSessionRequest::NewSessionRequest()
	: m_message(JSONMessage::Create(gMessageType))
{
	
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


XTOOLS_NAMESPACE_END
