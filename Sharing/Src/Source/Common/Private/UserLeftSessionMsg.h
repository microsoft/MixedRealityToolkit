//////////////////////////////////////////////////////////////////////////
// UserLeftSessionMsg.h
//
// Session message sent to notify users in a session that new user has left
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class UserLeftSessionMsg
{
public:
	UserLeftSessionMsg(uint32 sessionID, UserID userID);
	UserLeftSessionMsg(const JSONMessagePtr& message);

	static const std::string& GetMessageType();

	std::string		ToJSONString() const;

	uint32			GetSessionID() const;
	UserID			GetUserID() const;

private:
	JSONMessagePtr	m_message;

	static const std::string gMessageType;
	static const std::string gSessionIDKey;
	static const std::string gUserIDKey;
};

XTOOLS_NAMESPACE_END