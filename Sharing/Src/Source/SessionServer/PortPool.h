// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// PortPool.h
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
