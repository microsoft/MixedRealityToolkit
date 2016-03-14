//////////////////////////////////////////////////////////////////////////
// PortPool.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

// Keeps a running list of available networks ports
class PortPool : public AtomicRefCounted
{
public:
	PortPool(uint16 startingPort, uint16 poolSize);

	bool GetPort(uint16& port);

	void ReleasePort(uint16 port);

private:
	std::list<uint16> m_ports;
};

DECLARE_PTR(PortPool)

XTOOLS_NAMESPACE_END
