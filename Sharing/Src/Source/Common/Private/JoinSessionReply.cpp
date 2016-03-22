//////////////////////////////////////////////////////////////////////////
// JoinSessionReply.cpp
//
// Implementation of the new session request.
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "JoinSessionReply.h"

XTOOLS_NAMESPACE_BEGIN

const std::string JoinSessionReply::gMessageType("NewSessionReply");
const std::string JoinSessionReply::gSessionResult("result");


JoinSessionReply::JoinSessionReply(bool result)
	: m_message(JSONMessage::Create(gMessageType))
{
	m_message->SetValue(gSessionResult, result);
}


JoinSessionReply::JoinSessionReply(const JSONMessagePtr& message)
	: m_message(message)
{

}


//static 
const std::string& JoinSessionReply::GetMessageType()
{
	return gMessageType;
}

std::string JoinSessionReply::ToJSONString() const
{
	return m_message->ToString();
}


bool JoinSessionReply::GetResult() const
{
	return m_message->GetBoolValue(gSessionResult);
}

XTOOLS_NAMESPACE_END
