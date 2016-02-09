//////////////////////////////////////////////////////////////////////////
// SessionMessageRouter.h
//
// Allows user code to register functions to call back to when different
// types of JSON messages
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <Public/Utils/Callback2.h>
#include <map>
#include <type_traits>
#include <Public/Reflection.h>

XTOOLS_NAMESPACE_BEGIN

static const std::string kSessionMessageTypeKey("MsgType");

class MessageHandlerProxy : public RefCounted
{
public:
	virtual ~MessageHandlerProxy() {}
	virtual const std::string& GetMessageType() const = 0;
	virtual void HandleMessage(const JSONMessagePtr& message, const NetworkConnectionPtr& connection) const = 0;
};

DECLARE_PTR(MessageHandlerProxy)



class SessionMessageRouter
{
public:
	SessionMessageRouter() {}

	void RegisterHandler(const MessageHandlerProxyPtr& handler);

	// Returns false if there is no handlers registered for the given message, or the message is incorrect or poorly formatted
	bool CallHandler(const JSONMessagePtr& message, const NetworkConnectionPtr& connection) const;

private:
	std::map<std::string, MessageHandlerProxyPtr> m_handlerMap;
};



template<typename T>
class MessageHandlerProxyT : public MessageHandlerProxy
{
	typedef typename stripped_type<T>::type MsgType;

public:
	MessageHandlerProxyT(Callback2<T, const NetworkConnectionPtr&> handlerCallback)
		: m_handler(handlerCallback)
	{}

private:
	virtual const std::string& GetMessageType() const XTOVERRIDE
	{
		return MsgType::GetMessageType();
	}

	virtual void HandleMessage(const JSONMessagePtr& message, const NetworkConnectionPtr& connection) const XTOVERRIDE
	{
		MsgType messageWrapper(message);

		m_handler.Call(messageWrapper, connection);
	}

	Callback2<T, const NetworkConnectionPtr&> m_handler;
};

XTOOLS_NAMESPACE_END
