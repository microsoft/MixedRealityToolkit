// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// JoinSessionReply.h
// This class wraps the ReplyJoinSession message (JSON message at the network level).  
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class JoinSessionReply
{
public:
	JoinSessionReply(bool result);
	JoinSessionReply(const JSONMessagePtr& message);

	static const std::string&	GetMessageType();

	std::string					ToJSONString() const;

	bool						GetResult() const;

private:
	JSONMessagePtr	m_message;

	static const std::string gMessageType;
	static const std::string gSessionResult;
};

XTOOLS_NAMESPACE_END
