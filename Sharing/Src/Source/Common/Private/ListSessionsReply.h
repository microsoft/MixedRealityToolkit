// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// ListSessionsReply.h
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "SessionDescriptorImpl.h"

XTOOLS_NAMESPACE_BEGIN

class ListSessionsReply
{
public:
	ListSessionsReply();
	explicit ListSessionsReply(const JSONMessagePtr& message);

	static const std::string& GetMessageType();

	std::string ToJSONString() const;



	int32 GetSessionCount() const;
	SessionDescriptor* GetSessionDescriptor(const int32 i) const;

	void SetSessionCount(const int32 count);
	void SetSessionDescriptor(int32 i, SessionDescriptorImplPtr psess);

private:
	int32 m_sessionCount;
	std::vector<SessionDescriptorImplPtr> m_sessions;

	static const std::string gMessageType;
	static const std::string gSessionListKey;
};

XTOOLS_NAMESPACE_END
