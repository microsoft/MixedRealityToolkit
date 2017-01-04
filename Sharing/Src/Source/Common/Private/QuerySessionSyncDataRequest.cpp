//////////////////////////////////////////////////////////////////////////
// QuerySessionSyncDataRequest.cpp
//
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "QuerySessionSyncDataRequest.h"

XTOOLS_NAMESPACE_BEGIN

namespace
{
	static const std::string kMessageType("QuerySessionSyncDataRequest");
	static const std::string kMessageSyncUris("syncUris");
}

QuerySessionSyncDataRequest::QuerySessionSyncDataRequest(const std::string uris[], size_t uriCount)
	: m_message(JSONMessage::Create(kMessageType))
{
	web::json::value value = web::json::value::array(uriCount);

	for (size_t i = 0; i < uriCount; ++i)
	{
		value[i] = web::json::value::string(utility::conversions::to_string_t(uris[i]));
	}

	m_message->SetValue(kMessageSyncUris, value);
}


QuerySessionSyncDataRequest::QuerySessionSyncDataRequest(const JSONMessagePtr& message)
	: m_message(message)
{

}

void QuerySessionSyncDataRequest::GetSyncDataUris(std::vector<std::string>& out) const
{
	web::json::value syncUrisValue = m_message->GetObjectValue(kMessageSyncUris);
	web::json::array& syncUrisArray = syncUrisValue.as_array();
	for (auto syncUri : syncUrisArray)
	{
		out.push_back(utility::conversions::to_utf8string(syncUri.as_string()));
	}
}

//static 
const std::string& QuerySessionSyncDataRequest::GetMessageType()
{
	return kMessageType;
}


std::string QuerySessionSyncDataRequest::ToJSONString() const
{
	return m_message->ToString();
}

XTOOLS_NAMESPACE_END
