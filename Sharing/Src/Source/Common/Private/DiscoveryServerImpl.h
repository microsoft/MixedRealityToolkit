//////////////////////////////////////////////////////////////////////////
// DiscoveryServerImpl.h
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "SystemDescription.h"

XTOOLS_NAMESPACE_BEGIN

class DiscoveryServerImpl : public DiscoveryServer
{
public:
	DiscoveryServerImpl(SystemRole role);

	virtual void Update() XTOVERRIDE;

private:
	PeerPtr m_peer;
};

XTOOLS_NAMESPACE_END
