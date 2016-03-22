//////////////////////////////////////////////////////////////////////////
// DiscoveryClient.h
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

DECLARE_PTR_PRE(DiscoveryClient)

class DiscoveryClient : public AtomicRefCounted
{
public:
	static ref_ptr<DiscoveryClient> Create();

	/// Broadcast on the local network to discover any remote XTools machines
	virtual void Ping() = 0;

	/// Returns the number of systems discovered
	virtual uint32 GetDiscoveredCount() const = 0;

	/// Returns the information about a discovered system
	virtual DiscoveredSystemPtr GetDiscoveredSystem(uint32 index) const = 0;

	virtual void Update() = 0;

	virtual void AddListener(DiscoveryClientListener* newListener) = 0;

	virtual void RemoveListener(DiscoveryClientListener* oldListener) = 0;
};

DECLARE_PTR_POST(DiscoveryClient)

XTOOLS_NAMESPACE_END
