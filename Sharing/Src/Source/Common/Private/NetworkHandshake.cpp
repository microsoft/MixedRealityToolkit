//////////////////////////////////////////////////////////////////////////
// NetworkHandshake.cpp
//
// Class that validates a new connection by passing back and forth handshake
// messages using user-provided handshake logic.  Notifies the given callback
// when the handshake either succeeds or fails
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Public/NetworkHandshake.h>
#include "NetworkOutMessageImpl.h"
#include "NetworkInMessageImpl.h"

XTOOLS_NAMESPACE_BEGIN

NetworkHandshake::NetworkHandshake(const XSocketPtr& socket, const HandshakeLogicPtr& logic, const HandshakeCallback& callback)
: m_socket(socket)
, m_logic(logic)
, m_callback(callback)
{
    XTASSERT(m_socket);

    if (m_socket)
    {
        m_listenerReceipt = m_socket->RegisterListener(this);

        if (m_socket->IsConnected())
        {
            OnConnected(m_socket);
        }
    }
}


void NetworkHandshake::OnConnected(const XSocketPtr& )
{
	// Send the handshake message to the remote peer
	NetworkOutMessagePtr msg = new NetworkOutMessageImpl();
	msg->Write((byte)MessageID::Handshake);

	// Let the logic object fill in the contents of the handshake message
	m_logic->CreateOutgoingMessage(msg);

	Send(msg);
}


void NetworkHandshake::OnConnectionFailed(const XSocketPtr&, FailureReason )
{
    ReturnFailure(HandshakeResult::Failure);
}


void NetworkHandshake::OnDisconnected(const XSocketPtr&)
{
    ReturnFailure(HandshakeResult::Failure);
}


void NetworkHandshake::OnMessageReceived(const XSocketPtr& , const byte* message, uint32 messageLength)
{
	// Wrap the message in a NetworkMessage on the stack and call the callback.
	NetworkInMessageImpl msg(message, messageLength);

	bool bValidPreamble = true;

    byte preamble = msg.ReadByte();

	// Read the message ID off the front before passing it off to the callback
    if (preamble != (byte)MessageID::Handshake)
	{
        LogError("Invalid handshake preamble. Got %u. Expected %u", preamble, (byte)MessageID::Handshake);
		bValidPreamble = false;
	}

	// We've received a handshake message from the remote client.  Pass it to the logic object to see if 
	// the handshake should succeed or fail
	if (bValidPreamble && m_logic->ValidateIncomingMessage(msg))
	{
		// The handshake is good.  Pass the validated connection to the callback
		ReturnSuccess();
	}
	else
	{
		// Handshake failed.  Kill the connection and notify the callback. 
        // Failures detected here are fatal to the connection and retries should be suppressed.
        ReturnFailure(HandshakeResult::FatalFailure);
	}
}


void NetworkHandshake::OnMessageReceivedAsync(const XSocketPtr& socket, const byte* message, uint32 messageLength)
{
	// Intentionally blank

	XT_UNREFERENCED_PARAM(socket);
	XT_UNREFERENCED_PARAM(message);
	XT_UNREFERENCED_PARAM(messageLength);
}


void NetworkHandshake::ReturnSuccess()
{
	// Hold on to a reference to this object so it is not destroyed by the callback before it can complete
	NetworkHandshakePtr temp = this;
	XT_UNREFERENCED_PARAM(temp);

	// Unregister the listener before returning the connection
	m_listenerReceipt = NULL;

	// Notify the callback that the handshake was successful by passing the valid connection
    m_callback.Call(m_socket, m_socket->GetID(), HandshakeResult::Success);

	// Release our reference to the connection; the callback function should have added a reference
	m_socket = NULL;
}


void NetworkHandshake::ReturnFailure(HandshakeResult result)
{
	// Hold on to a reference to this object so it is not destroyed by the callback before it can complete
	NetworkHandshakePtr temp = this;
	XT_UNREFERENCED_PARAM(temp);

    SocketID id = m_socket->GetID();

	// Release the failed connection to cause it to immediately close
	m_listenerReceipt = NULL;
	m_socket = NULL;

	// Notify the callback that the handshake was unsuccessful by passing it a null connection
    m_callback.Call(NULL, id, result);
}


void NetworkHandshake::Send(const NetworkOutMessagePtr& msg)
{
	XTASSERT(m_socket->IsConnected());

	m_socket->Send(msg->GetData(), msg->GetSize(), MessagePriority::Immediate, MessageReliability::ReliableOrdered, MessageChannel::Default);
}


XTOOLS_NAMESPACE_END

