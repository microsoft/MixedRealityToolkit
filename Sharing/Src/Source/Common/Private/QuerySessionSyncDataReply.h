//////////////////////////////////////////////////////////////////////////
// QuerySessionSyncDataRequest.h
//
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class QuerySessionSyncDataReply
{
public:

	struct SyncData
	{
		bool Verify() const;
		void Clear();
		const XValue& GetValue(size_t uriIndex, size_t sessionIndex) const;

		std::vector<std::string> Uris;
		std::vector<std::string> SessionNames;
		std::vector<XValue> Values;
	};

public:
	QuerySessionSyncDataReply(const SyncData& syncData);
	QuerySessionSyncDataReply(const JSONMessagePtr& message);

	void GetSyncData(SyncData& out) const;
	
	static const std::string& GetMessageType();

	std::string		ToJSONString() const;

private:
	JSONMessagePtr	m_message;
};

XTOOLS_NAMESPACE_END
