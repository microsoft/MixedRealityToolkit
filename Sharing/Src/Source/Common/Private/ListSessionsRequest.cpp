// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// ListSessionsRequest.cpp
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ListSessionsRequest.h"

XTOOLS_NAMESPACE_BEGIN

const std::string ListSessionsRequest::gMessageType("ListSessionsRequest");

ListSessionsRequest::ListSessionsRequest()
	: m_message(JSONMessage::Create(gMessageType))
{

}


ListSessionsRequest::ListSessionsRequest(const JSONMessagePtr& message)
	: m_message(message)
{

}


//static 
const std::string& ListSessionsRequest::GetMessageType()
{
	return gMessageType;
}


std::string ListSessionsRequest::ToJSONString() const
{
	return m_message->ToString();
}

XTOOLS_NAMESPACE_END
