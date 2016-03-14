//////////////////////////////////////////////////////////////////////////
// Reflection.cpp
//
// Provides the interface for enabling reflection on XTools types.
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "stdarg.h"
#include "Public/Reflection.h"

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Reflection)

TypeInfo::TypeInfo(std::string typeName)
	: m_typeName(typeName)
	, m_typeID(GetNextTypeID())
{
}

int TypeInfo::GetID() const
{
	return m_typeID;
}

std::string TypeInfo::GetName() const
{
	return m_typeName;
}

int TypeInfo::GetNextTypeID()
{
	static int s_nextTypeID = 0;
	return s_nextTypeID++;
}


bool TypeInfo::IsDerivedFrom(int baseTypeID) const
{
	if (m_typeID == baseTypeID) return true;

	for (auto it = m_baseTypes.begin(); it != m_baseTypes.end(); ++it)
	{
		if (it->m_typeInfo->IsDerivedFrom(baseTypeID))
		{
			return true;
		}
	}

	return false;
}


void* TypeInfo::CastToBase(void* derived, int baseTypeID) const
{
	if (!derived)
	{
		return nullptr;
	}

	if (m_typeID == baseTypeID) return derived;

	for (auto it = m_baseTypes.begin(); it != m_baseTypes.end(); ++it)
	{
		if (it->m_typeInfo->IsDerivedFrom(baseTypeID))
		{
			void* immediateBase = (*it->m_baseCaster)(derived);
			return it->m_typeInfo->CastToBase(immediateBase, baseTypeID);
		}
	}

	return nullptr;
}

const void* TypeInfo::CastToBase(const void* derived, int baseTypeID) const
{
	if (!derived)
	{
		return nullptr;
	}

	if (m_typeID == baseTypeID) return derived;

	for (auto it = m_baseTypes.begin(); it != m_baseTypes.end(); ++it)
	{
		if (it->m_typeInfo->IsDerivedFrom(baseTypeID))
		{
			const void* immediateBase = (*it->m_baseCaster)(derived);
			return it->m_typeInfo->CastToBase(immediateBase, baseTypeID);
		}
	}

	return nullptr;
}

XTOOLS_REFLECTION_DEFINE(XTObject);

NAMESPACE_END(Reflection)
XTOOLS_NAMESPACE_END
