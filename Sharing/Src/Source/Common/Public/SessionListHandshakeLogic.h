//////////////////////////////////////////////////////////////////////////
// SessionListHandshakeLogic.h
//
// The logic and packet formats for the handshake between the desktop 
// app and the Session list server
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class SessionListHandshakeLogic : public HandshakeLogic
{
public:
	// Pass true to use the logic for the server side of the handshake,
	// false for the desktop side of the handshake
	explicit SessionListHandshakeLogic(bool isServer);

	/// HandshakeLogic Functions:
	// Fill the given message object with a handshake message based on the criteria of this handshake logic
	virtual void CreateOutgoingMessage(const NetworkOutMessagePtr& msg) const XTOVERRIDE;

	// Parse the message received from a remote peer and return true if the message is an acceptable handshake.  
	virtual bool ValidateIncomingMessage(NetworkInMessage& msg) const XTOVERRIDE;

private:
	void CreateMessageFromServer(const NetworkOutMessagePtr& msg) const;
	bool ValidateMessageFromServer(NetworkInMessage& msg) const;

	void CreateMessageFromDesktop(const NetworkOutMessagePtr& msg) const;
	bool ValidateMessageFromDesktop(NetworkInMessage& msg) const;

	bool	m_isServer;
};

XTOOLS_NAMESPACE_END