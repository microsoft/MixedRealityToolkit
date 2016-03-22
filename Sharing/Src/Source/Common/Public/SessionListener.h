//////////////////////////////////////////////////////////////////////////
// SessionListener.h
//
// Inherit from this class to receive event notifications from a session
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

XT_LISTENER_DECLARE(SessionListener)

class SessionListener XTABSTRACT : public Listener
{
public:
	virtual ~SessionListener() {}
	virtual void OnJoiningSession() {}
	virtual void OnJoinSucceeded() {}
	virtual void OnJoinFailed() {}
	virtual void OnSessionDisconnected() {}
};

XTOOLS_NAMESPACE_END
