// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// SessionListener.h
// Inherit from this class to receive event notifications from a session
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
