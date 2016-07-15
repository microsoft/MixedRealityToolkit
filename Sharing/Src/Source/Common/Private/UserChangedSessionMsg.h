// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// UserChangedSessionMsg.h
// Session message sent to notify users in a session that a remote user's state has changed.
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class UserChangedSessionMsg
{
public:
    UserChangedSessionMsg(uint32 sessionID, const std::string& userName, UserID userID, bool muteState);
    UserChangedSessionMsg(const JSONMessagePtr& message);

	static const std::string& GetMessageType();

	std::string		ToJSONString() const;

	uint32			GetSessionID() const;
	std::string		GetSessionUserName() const;
    UserID			GetSessionUserID() const;
    bool			GetSessionUserMuteState() const;

private:
	JSONMessagePtr	m_message;

	static const std::string gMessageType;
	static const std::string gSessionIDKey;
	static const std::string gUserNameKey;
    static const std::string gUserIDKey;
    static const std::string gUserMuteStateKey;
};

XTOOLS_NAMESPACE_END
