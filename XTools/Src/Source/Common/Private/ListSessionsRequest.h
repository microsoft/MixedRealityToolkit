//////////////////////////////////////////////////////////////////////////
// ListSessionsRequest.h
//
// This class wraps the RequestSessionList message (JSON message at the network level).  
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class ListSessionsRequest
{
public:
	ListSessionsRequest();
	explicit ListSessionsRequest(const JSONMessagePtr& message);

	static const std::string& GetMessageType();

	virtual std::string ToJSONString() const;

private:
	JSONMessagePtr	m_message;

	static const std::string gMessageType;
};

XTOOLS_NAMESPACE_END