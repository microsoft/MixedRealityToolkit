//////////////////////////////////////////////////////////////////////////
// OperationFactory.cpp
//
// Quick scalable way to create Operations from an enum
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "OperationFactory.h"

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

void OperationFactory::RegisterMaker(Operation::Type type, const OpMakerPtr& maker)
{
	m_makers[type] = maker;
}


OperationPtr OperationFactory::Make(Operation::Type type, AuthorityLevel authLevel) const
{
	auto maker = m_makers.find(type);
	if (maker != m_makers.end())
	{
		return maker->second->Create(authLevel);
	}
	else
	{
		LogError("Operation type does not have a maker in the factory");
		XTASSERT(false);
		return NULL;
	}
}

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
