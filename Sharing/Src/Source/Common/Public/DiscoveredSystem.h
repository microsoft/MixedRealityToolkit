//////////////////////////////////////////////////////////////////////////
// DiscoveredSystem.h
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

/// Information about a system that has been discovered
class DiscoveredSystem : public AtomicRefCounted
{
public:
	DiscoveredSystem(const std::string& name, const std::string& address, SystemRole role);

	std::string	GetName() const;
	std::string	GetAddress() const;
	SystemRole	GetRole() const;

private:
	std::string		m_name;
	std::string		m_address;
	SystemRole		m_role;
};

DECLARE_PTR(DiscoveredSystem)

XTOOLS_NAMESPACE_END
