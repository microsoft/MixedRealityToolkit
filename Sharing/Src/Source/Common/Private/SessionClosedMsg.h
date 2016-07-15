// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// SessionClosedMsg.h
// Message sent to notify clients that a session has been closed
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
