//////////////////////////////////////////////////////////////////////////
// Operation.cpp
//
// 
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Operation.h"

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

const char* kOpTypeNames[Operation::TypeCount] =
{
	"Ack",
	"Noop",
	"Create",
	"Modify",
	"Delete",
	"Insert",
	"Update",
	"Remove",
	"Move"
};


Operation::Operation(Type type, AuthorityLevel authLevel)
: m_type(type)
, m_authLevel(authLevel)
{

}


Operation::Operation(Type type, AuthorityLevel authLevel, const std::vector<XGuid>& hierarchy)
	: m_hierarchy(hierarchy)
	, m_type(type)
	, m_authLevel(authLevel)
{

}


Operation::Type Operation::GetType() const
{
	return m_type;
}


const char* Operation::GetTypeName() const
{
	return kOpTypeNames[m_type];
}


AuthorityLevel Operation::GetAuthorityLevel() const
{
	return m_authLevel;
}


void Operation::SetAuthorityLevel(AuthorityLevel authLevel)
{
	m_authLevel = authLevel;
}


const std::vector<XGuid>& Operation::GetHierarchy() const
{
	return m_hierarchy;
}


bool Operation::IsDescendantOf(XGuid elementID) const
{
	for (size_t i = 0; i < m_hierarchy.size(); ++i)
	{
		if (m_hierarchy[i] == elementID)
		{
			return true;
		}
	}

	return false;
}


NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
