//////////////////////////////////////////////////////////////////////////
// UserPresenceManager.h
//
// Interface for maintaining and distributing user presence notions.
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class UserPresenceManager : public AtomicRefCounted
{
public:
	/// Register an object to receive notifications when the user's settings change.  
	/// Multiple listeners can be registered.  The wrapper class will hold a reference to the listener
	/// to ensure it is not garbage collected until this class is destroyed or the listener is removed. 
	/// \param newListener The listener object that will receive callbacks from this SessionManager
	virtual void AddListener(UserPresenceManagerListener* newListener) = 0;

	/// Remove a previously registered listener.  The wrapper class will release its reference to the given listener.  
	/// \param oldListener The listener object that will no longer receive callbacks.  
	virtual void RemoveListener(UserPresenceManagerListener* oldListener) = 0;

    virtual bool GetMuteState() const = 0;

    virtual void SetMuteState(bool muteState) = 0;

    virtual void SetName(const XStringPtr& name) = 0;

    virtual XStringPtr GetName() const = 0;

    virtual void SetUser(const UserPtr& localUser) = 0;
};

DECLARE_PTR(UserPresenceManager)

XTOOLS_NAMESPACE_END
