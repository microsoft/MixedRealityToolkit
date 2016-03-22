//////////////////////////////////////////////////////////////////////////
// SyncListener.h
//
// Base class for objects that wish to register for notifications before and 
// after sync changes are applied
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

// This allows C++ calls to these functions to cause the Java or C# code overloading the function to execute
#if defined(SWIG)
%feature("director") SyncListener;
#endif

// Disable unused parameter warnings.  Pure virtual functions don't play nice with SWIG, but we still want 
// to see the names of the parameters to make reading the code clearer
#pragma warning( push )
#pragma warning( disable : 4100 ) 

class SyncListener : public Listener
{
public:
	// Called when a session creation request succeeded
	virtual void OnSyncChangesBegin() XTABSTRACT {}

	// Called when a session creation request failed
	virtual void OnSyncChangesEnd() XTABSTRACT {}
};

#pragma warning( pop )

XTOOLS_NAMESPACE_END