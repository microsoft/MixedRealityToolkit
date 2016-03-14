//////////////////////////////////////////////////////////////////////////
// PortPool.cpp
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PortPool.h"

XTOOLS_NAMESPACE_BEGIN

PortPool::PortPool(uint16 startingPort, uint16 poolSize)
{
	// Create a entry in the list of every port than can be used
	for (uint16 i = 0; i < poolSize; i++)
	{
		m_ports.push_back(startingPort - i);
	}
}


bool PortPool::GetPort(uint16& port)
{
	if (!m_ports.empty())
	{
		port = m_ports.front();
		m_ports.pop_front();
		return true;
	}
	else
	{
		return false;
	}
}


void PortPool::ReleasePort(uint16 port)
{
	m_ports.push_front(port);
}

XTOOLS_NAMESPACE_END
