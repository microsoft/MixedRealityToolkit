// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// SessionChangeCallback.h
// Interface class for objects that want to receive notifications from
// NetworkConnections
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "XSession.h"

// Disable unused parameter warnings.  Pure virtual functions don't play nice with SWIG, but we still want 
// to see the names of the parameters to make reading the code clearer
#pragma warning( push )
#pragma warning( disable : 4100 ) 

XTOOLS_NAMESPACE_BEGIN

class SessionChangeCallback XTABSTRACT
{
public:
	virtual ~SessionChangeCallback() {}

	virtual void OnUserJoinedSession(uint32 sessionID, const std::string& userName, UserID userID, bool muteState) = 0;

	virtual void OnUserLeftSession(uint32 sessionID, UserID userID) = 0;

	virtual void OnSessionEmpty(const XSessionConstPtr& session) = 0;

    virtual void OnUserChanged(uint32 sessionID, const std::string& userName, UserID userId, bool muteState) = 0;
};

XTOOLS_NAMESPACE_END

#pragma warning( pop )
