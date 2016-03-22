//////////////////////////////////////////////////////////////////////////
// NetworkHandshake.h
//
// Class that validates a new connection by passing back and forth handshake
// messages using user-provided handshake logic.  Notifies the given callback
// when the handshake either succeeds or fails
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <Public/Utils/Callback3.h>

XTOOLS_NAMESPACE_BEGIN

enum HandshakeResult
{
    Success = 0,
    Failure,
    FatalFailure
};

typedef Callback3<const XSocketPtr&, SocketID, HandshakeResult> HandshakeCallback;

class NetworkHandshake : public RefCounted, public XSocketListener
{
public:
	NetworkHandshake(const XSocketPtr& socket, const HandshakeLogicPtr& logic, const HandshakeCallback& callback);

private:
	// NetworkConnectionListener Functions:
	virtual void OnConnected(const XSocketPtr& socket) XTOVERRIDE;
	virtual void OnConnectionFailed(const XSocketPtr& socket, FailureReason reason) XTOVERRIDE;
	virtual void OnDisconnected(const XSocketPtr& socket) XTOVERRIDE;
	virtual void OnMessageReceived(const XSocketPtr& socket, const byte* message, uint32 messageLength) XTOVERRIDE;
	virtual void OnMessageReceivedAsync(const XSocketPtr& socket, const byte* message, uint32 messageLength) XTOVERRIDE;

	// Local Functions:
	void ReturnSuccess();
    void ReturnFailure(HandshakeResult result);

	void Send(const NetworkOutMessagePtr& msg);

	XSocketPtr				m_socket;
	HandshakeLogicPtr		m_logic;
	HandshakeCallback		m_callback;
	ReceiptPtr				m_listenerReceipt;
};

DECLARE_PTR(NetworkHandshake)

XTOOLS_NAMESPACE_END