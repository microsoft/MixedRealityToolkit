// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// OperationFactory.cpp
// Quick scalable way to create Operations from an enum
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
