//////////////////////////////////////////////////////////////////////////
// SessionAddedMsg.h
//
// Message sent to notify clients that a session has been added
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
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