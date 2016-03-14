//////////////////////////////////////////////////////////////////////////
// DiscoveryClientListener.h
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

// Disable unused parameter warnings.  Pure virtual functions don't play nice with SWIG directors, 
// but we still want to see the names of the parameters to make reading the code clearer
#pragma warning( push )
#pragma warning( disable : 4100 ) 

XT_LISTENER_DECLARE(DiscoveryClientListener)

class DiscoveryClientListener XTABSTRACT : public Listener
{
public:
	virtual ~DiscoveryClientListener() {}
	virtual void OnRemoteSystemDiscovered(const DiscoveredSystemPtr& remoteSystem) {}
	virtual void OnRemoteSystemLost(const DiscoveredSystemPtr& remoteSystem) {}
};

#pragma warning( pop )

XTOOLS_NAMESPACE_END
