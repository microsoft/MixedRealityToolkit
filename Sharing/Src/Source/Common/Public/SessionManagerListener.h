//////////////////////////////////////////////////////////////////////////
// SessionManagerListener.h
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

// Disable unused parameter warnings.  Pure virtual functions don't play nice with SWIG directors, 
// but we still want to see the names of the parameters to make reading the code clearer
#pragma warning( push )
#pragma warning( disable : 4100 ) 

XT_LISTENER_DECLARE(SessionManagerListener)

/// Base class for receiving callbacks from the SessionManager about the 
/// creation and deletion of sessions.  
class SessionManagerListener XTABSTRACT : public Listener
{
public:
	virtual ~SessionManagerListener() {}

	/// Called when a session creation request succeeded
	virtual void OnCreateSucceeded(const SessionPtr& newSession) {}

	/// Called when a session creation request failed
	virtual void OnCreateFailed(const XStringPtr& reason) {}

	/// Callback for when a new session has been created on the server
	virtual void OnSessionAdded(const SessionPtr& newSession) {}

	/// Callback for when a session has closed
	virtual void OnSessionClosed(const SessionPtr& session) {}

	/// Callback for when a new user has joined a session
	virtual void OnUserJoinedSession(const SessionPtr& session, const UserPtr& newUser) {}

	/// Callback for when a user has left a session
	virtual void OnUserLeftSession(const SessionPtr& session, const UserPtr& user) {}

    /// Callback for a when a user's state has changed.
	virtual void OnUserChanged(const SessionPtr& session, const UserPtr& user) {}

	/// Callback for when a connection to the Session List Server has been established.
	virtual void OnServerConnected() {}

    /// Callback for when connectivity to the Session List Server has gone away.
    virtual void OnServerDisconnected() {}
};

#pragma warning( pop )

XTOOLS_NAMESPACE_END