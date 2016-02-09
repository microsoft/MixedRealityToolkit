//////////////////////////////////////////////////////////////////////////
// PortPool.cpp
//
// A simple port ID pooling class that reuses port IDs
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PortMachinePool.h"

XTOOLS_NAMESPACE_BEGIN

PortMachinePool::PortMachinePool(const std::string& machineName, uint16 startingPort)
{
	XTASSERT(startingPort > kNumberOfPortMachinePairs);

	// very simple pool init, starting at kSessionServerPort and going down
	for (int i = 0; i < kNumberOfPortMachinePairs; i++)
	{
		ports[i].pair.portID = startingPort - (uint16)(i + 1); // count down from Session Server for now
		ports[i].pair.address = machineName;
		ports[i].inUse = false;
	}
}

PortMachinePair PortMachinePool::GetPortMachinePair()
{
	// no load leveling or anything fancy, just get the next available port
	for (int i = 0; i < kNumberOfPortMachinePairs; i++)
	{
		if (ports[i].inUse == false)
		{
			ports[i].inUse = true;
            LogInfo("GetPortMachinePair: returning pmp %s:%u", ports[i].pair.address.c_str(), ports[i].pair.portID);
			return ports[i].pair;
		}
	}

    LogError("GetPortMachinePair: Failed to find an available port-machine pair.");
	return m_nullPortMachinePair; // this is the error message when we try to use too many
}

void PortMachinePool::ReleasePortMachinePair(PortMachinePair pair)
{
	// find it and reset its use flag
	for (int i = 0; i < kNumberOfPortMachinePairs; i++)
	{
		if (Match(pair, ports[i].pair))
		{
			ports[i].inUse = false;
            LogInfo("ReleasePortMachinePair: releasing pmp %s:%u", ports[i].pair.address.c_str(), ports[i].pair.portID);
			return;
		}
	}

	// We're returning a pair that we don't recognize...
	XTASSERT(false);
}

bool PortMachinePool::Match(PortMachinePair pmp1, PortMachinePair pmp2)
{
	if (pmp1.address.compare(pmp2.address) == 0 && pmp1.portID == pmp2.portID)
	{
		return true;
	}

	return false;
}

XTOOLS_NAMESPACE_END