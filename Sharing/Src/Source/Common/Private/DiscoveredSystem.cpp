//////////////////////////////////////////////////////////////////////////
// DiscoveredSystem.cpp
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Public/DiscoveredSystem.h>

XTOOLS_NAMESPACE_BEGIN

DiscoveredSystem::DiscoveredSystem(const std::string& name, const std::string& address, SystemRole role)
: m_name(name)
, m_address(address)
, m_role(role)
{

}


std::string DiscoveredSystem::GetName() const
{
	return m_name;
}


std::string	DiscoveredSystem::GetAddress() const
{
	return m_address;
}


SystemRole DiscoveredSystem::GetRole() const
{
	return m_role;
}

XTOOLS_NAMESPACE_END
