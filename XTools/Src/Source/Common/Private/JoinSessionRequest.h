//////////////////////////////////////////////////////////////////////////
// JoinSessionRequest.h
//
// This class wraps the JoinSession message (JSON message at the network level).  
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class JoinSessionRequest
{
public:
    JoinSessionRequest(const std::string& userName, UserID userID, bool muteState);
	JoinSessionRequest(const JSONMessagePtr& message);

	static const std::string& GetMessageType();

	std::string		ToJSONString() const;

	std::string		GetUserName() const;
	UserID			GetUserID() const;

    bool            GetMuteState() const;

private:
	JSONMessagePtr	m_message;

	static const std::string gMessageType;
	static const std::string gUserNameKey;
    static const std::string gUserIDKey;
    static const std::string gMuteStateKey;
};

XTOOLS_NAMESPACE_END
