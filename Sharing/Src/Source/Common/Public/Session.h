//////////////////////////////////////////////////////////////////////////
// Session.h
//
// Represents the data for an active session.  Note: a session will exist
// for a long as you are in it.  If you are not in this session, it can
// close and become invalid at any time.  
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

enum MachineSessionState : byte
{
    DISCONNECTED = 0,
    JOINING,
    JOINED
};

enum SessionType : byte
{ 
	PERSISTENT = 0, 
	ADHOC
};

class Session : public AtomicRefCounted
{
public:

    // What's the current relationship of this machine to the session?
    virtual MachineSessionState GetMachineSessionState() const = 0;

	/// Register an object to receive notifications when the status of the session changes.  
	/// Multiple listeners can be registered.  The wrapper class will hold a reference to the listener
	/// to ensure it is not garbage collected until this class is destroyed or the listener is removed. 
	/// \param newListener The listener object that will receive callbacks from this SessionManager
	virtual void AddListener(SessionListener* newListener) = 0;

	/// Remove a previously registered listener.  The wrapper class will release its reference to the given listener.  
	/// \param oldListener The listener object that will no longer receive callbacks from this SessionManager.  
	virtual void RemoveListener(SessionListener* oldListener) = 0;

	// Returns true if we have joined this session
	virtual bool IsJoined() const = 0;

    // Initiate a join process.  This is async; the SessionListener is notified when the join succeeds or fails.
    // Returns false if this machine is already part of this session, or a join is already in progress.  
    virtual bool Join() = 0;

    // Cause the machine to leave the session.  Closes the connection immediately 
    virtual void Leave() = 0;

    // Returns the number of users currently in the session
    virtual int32 GetUserCount() const = 0;

    // Returns the user at the given index
    virtual UserPtr GetUser(int32 i) = 0;

	virtual SessionType GetSessionType() const = 0;

	virtual const XStringPtr&	GetName() const = 0;

    virtual NetworkConnectionPtr GetSessionNetworkConnection() const = 0;
};

DECLARE_PTR(Session)

XTOOLS_NAMESPACE_END
