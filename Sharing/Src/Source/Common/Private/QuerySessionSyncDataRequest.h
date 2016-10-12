//////////////////////////////////////////////////////////////////////////
// QuerySessionSyncDataRequest.h
//
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class QuerySessionSyncDataRequest
{
public:
	QuerySessionSyncDataRequest(const std::string uris[], size_t uriCount);
	QuerySessionSyncDataRequest(const JSONMessagePtr& message);

	void GetSyncDataUris(std::vector<std::string>& out) const;


	static const std::string& GetMessageType();

	std::string		ToJSONString() const;

private:
	JSONMessagePtr	m_message;
};

XTOOLS_NAMESPACE_END
