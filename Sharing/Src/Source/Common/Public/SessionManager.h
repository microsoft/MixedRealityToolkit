//////////////////////////////////////////////////////////////////////////
// SessionManager.h
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

/// Keeps track of all available sessions on the server.  Allows the user to create new sessions,
/// and register to receive notifications about when sessions are created and deleted.  
class SessionManager : public AtomicRefCounted
{
public:
	/// Register an object to receive callbacks when a session is created or closed on the server
	/// Multiple listeners can be registered.  The wrapper class will hold a reference to the listener
	/// to ensure it is not garbage collected until this class is destroyed or the listener is removed. 
	/// \param newListener The listener object that will receive callbacks from this SessionManager
	virtual void AddListener(SessionManagerListener* newListener) = 0;

	/// Remove a previously registered listener.  The wrapper class will release its reference to the given listener.  
	/// \param oldListener The listener object that will no longer receive callbacks from this SessionManager.  
	virtual void RemoveListener(SessionManagerListener* oldListener) = 0;

	/// Request that the server create a session.  Returns true if the request is successfully submitted.
	/// SessionManagerListener::OnCreateSucceeded() will be called if the request succeeds, 
	/// SessionManagerListener::OnCreateFailed() will be called if it fails
	/// \param sessionName The new name to give the session
	/// \param type The type of the the new session
	/// \returns Returns true if the creation request was successfully sent to the server.  This does not mean that the session was created.  
	virtual bool CreateSession(const XStringPtr& sessionName) { return CreateSession(sessionName, SessionType::ADHOC); }
	virtual bool CreateSession(const XStringPtr& sessionName, SessionType type) = 0;

	/// Returns the number of active sessions
	virtual int32 GetSessionCount() const = 0;

	/// Returns the session at the given index
	virtual SessionPtr GetSession(int32 index) = 0;

	/// Returns the session that we are currently a part of
	virtual SessionPtr GetCurrentSession() = 0;

	/// Returns the \ref User object for the local user
    virtual const UserPtr& GetCurrentUser() const = 0;

	/// Returns true if we currently have a connection to the Session List Server
	virtual bool IsServerConnected() const = 0;

#if !defined(SWIG)
    virtual void UpdateCurrentUserOnServer() = 0;
#endif
};

DECLARE_PTR(SessionManager)

XTOOLS_NAMESPACE_END