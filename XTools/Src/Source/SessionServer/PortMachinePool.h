//////////////////////////////////////////////////////////////////////////
// PortPool.h
//
// Each session has its own port, pool the port ids here and reuse port id appropriately
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <Public/BasicTypes.h>
#include <Public/CommonSettings.h>
#include <string>

XTOOLS_NAMESPACE_BEGIN

const uint16 kNumberOfPortMachinePairs = 256;
const uint16 kInvalidPortId = 0;

struct PortMachinePair
{
    PortMachinePair() : portID(kInvalidPortId) {}

	std::string	address;
	uint16 portID;
};

// the port pool holds 256 port IDs for use and reuse.  Port IDs are uint16.
class PortMachinePool : public RefCounted
{
public:
	PortMachinePool(const std::string& machineName, uint16 startingPort);

	PortMachinePair m_nullPortMachinePair;

	// getPort will get the next available port/machine in the list.  If none are available, 
	// then s_nullPortMachinePair will be returned.
	PortMachinePair GetPortMachinePair();

	// release port will release the flag on the Port ID and allow it to be reused.
	void ReleasePortMachinePair(PortMachinePair port);

private:
	bool Match(PortMachinePair pmp1, PortMachinePair pmp2);

	struct PortMachineUsage
	{
		PortMachineUsage() : inUse(false) {}

		PortMachinePair pair;
		bool inUse;
	};

	PortMachineUsage ports[kNumberOfPortMachinePairs];
};

DECLARE_PTR(PortMachinePool)

XTOOLS_NAMESPACE_END