//////////////////////////////////////////////////////////////////////////
// HandshakeLogic.h
//
// 
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class HandshakeLogic : public RefCounted
{
public:
	// Fill the given message object with a handshake message based on the criteria of this handshake logic
	virtual void CreateOutgoingMessage(const NetworkOutMessagePtr& msg) const = 0;

	// Parse the message received from a remote peer and return true if the message is an acceptable handshake.  
	virtual bool ValidateIncomingMessage(NetworkInMessage& msg) const = 0;
};

DECLARE_PTR(HandshakeLogic)

XTOOLS_NAMESPACE_END
