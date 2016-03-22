//////////////////////////////////////////////////////////////////////////
// SessionClosedMsg.h
//
// Message sent to notify clients that a session has been closed
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class SessionClosedMsg
{
public:
	SessionClosedMsg(uint32 sessionID);
	SessionClosedMsg(const JSONMessagePtr& message);

	static const std::string& GetMessageType();

	std::string		ToJSONString() const;

	uint32			GetSessionID() const;

private:
	JSONMessagePtr	m_message;

	static const std::string gMessageType;
	static const std::string gSessionIDKey;
};

XTOOLS_NAMESPACE_END
