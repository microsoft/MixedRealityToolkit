//////////////////////////////////////////////////////////////////////////
// UserPresenceManagerListener.h
//
// Base class for receiving updates on a UserPresenceManager.
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

// Disable unused parameter warnings.  Pure virtual functions don't play nice with SWIG, but we still want 
// to see the names of the parameters to make reading the code clearer
#pragma warning( push )
#pragma warning( disable : 4100 ) 

XT_LISTENER_DECLARE(UserPresenceManagerListener)

class UserPresenceManagerListener XTABSTRACT : public Listener
{
public:
	virtual ~UserPresenceManagerListener() {}

    // Called when a session creation request succeeded
    virtual void OnUserPresenceChanged(const UserPtr& user) {}
};

#pragma warning( pop )

XTOOLS_NAMESPACE_END