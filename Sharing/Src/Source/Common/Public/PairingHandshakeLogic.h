//////////////////////////////////////////////////////////////////////////
// PairingHandshakeLogic.h
//
// The logic and packet formats for the pairing handshake between the desktop 
// app and Baraboo
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class PairingHandshakeLogic : public HandshakeLogic
{
public:
	// Pass true to use the logic for the baraboo side of the handshake,
	// false for the desktop side of the handshake
    explicit PairingHandshakeLogic(const UserPtr& user, bool isReceiver);

	/// HandshakeLogic Functions:
	// Fill the given message object with a handshake message based on the criteria of this handshake logic
	virtual void CreateOutgoingMessage(const NetworkOutMessagePtr& msg) const XTOVERRIDE;

	// Parse the message received from a remote peer and return true if the message is an acceptable handshake.  
	virtual bool ValidateIncomingMessage(NetworkInMessage& msg) const XTOVERRIDE;

private:
	void CreateMessageFromConnector(const NetworkOutMessagePtr& msg) const;
	bool ValidateMessageFromConnector(NetworkInMessage& msg) const;

	void CreateMessageFromReceiver(const NetworkOutMessagePtr& msg) const;
	bool ValidateMessageFromReceiver(NetworkInMessage& msg) const;

	UserPtr m_user;
    bool	m_isReceiver;
};

XTOOLS_NAMESPACE_END