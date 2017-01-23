//////////////////////////////////////////////////////////////////////////
// ElementFactory.cpp
//
// Scalable way to create new element objects from their respective enums
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ElementFactory.h"

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

void ElementFactory::RegisterMaker(ElementType type, const ElementMakerPtr& maker)
{
	m_makers[type] = maker;
}


ElementPtr ElementFactory::Make(ElementType type, SyncContext* syncContext, const XStringPtr& name, XGuid id, UserID ownerID, const XValue& startingValue) const
{
	XTASSERT(name);

	auto maker = m_makers.find(type);
	if (maker != m_makers.end())
	{
		return maker->second->Create(syncContext, name, id, ownerID, startingValue);
	}
	else
	{
		return NULL;
	}
}

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
