//////////////////////////////////////////////////////////////////////////
// SessionMessageRouter.cpp
//
// Allows user code to register functions to call back to when different
// types of JSON messages
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SessionMessageRouter.h"

XTOOLS_NAMESPACE_BEGIN

void SessionMessageRouter::RegisterHandler(const MessageHandlerProxyPtr& handler)
{
	m_handlerMap[handler->GetMessageType()] = handler;
}


bool SessionMessageRouter::CallHandler(const JSONMessagePtr& message, const NetworkConnectionPtr& connection) const
{
	try
	{
		std::string messageType = message->GetStringValue(kSessionMessageTypeKey);

		auto handler = m_handlerMap.find(messageType);
		if (handler != m_handlerMap.end())
		{
			handler->second->HandleMessage(message, connection);
		}
		else
		{
			LogWarning("Received a session message for which there is no handler registered");
			return false;
		}
	}
	catch (web::json::json_exception e)
	{
		LogError("Bad JSON message received: %s", e.what());
		return false;
	}

	return true;
}

XTOOLS_NAMESPACE_END
