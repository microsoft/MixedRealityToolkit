//////////////////////////////////////////////////////////////////////////
// DiscoveryServer.h
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

/// Allows this instance of XTools to respond to discovery request pings
class DiscoveryServer : public AtomicRefCounted, public IUpdateable
{
public:
	static ref_ptr<DiscoveryServer> Create(SystemRole role);
};

DECLARE_PTR(DiscoveryServer)

XTOOLS_NAMESPACE_END
