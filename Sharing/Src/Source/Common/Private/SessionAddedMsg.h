// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// SessionAddedMsg.h
// Message sent to notify clients that a session has been added
//////////////////////////////////////////////////////////////////////////

#pragma once


XTOOLS_NAMESPACE_BEGIN

class SessionAddedMsg
{
public:
	SessionAddedMsg();
	SessionAddedMsg(const JSONMessagePtr& message);

	static const std::string& GetMessageType();

	std::string		ToJSONString() const;

	SessionDescriptor* GetSessionDescriptor() const;

	void SetSessionDescriptor(SessionDescriptorImplPtr psess);

private:
	SessionDescriptorImplPtr m_session;

	static const std::string gMessageType;
	static const std::string gSessionKey;
};

XTOOLS_NAMESPACE_END
