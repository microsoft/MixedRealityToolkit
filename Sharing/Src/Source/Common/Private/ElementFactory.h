//////////////////////////////////////////////////////////////////////////
// ElementFactory.h
//
// Scalable way to create new element objects from their respective enums
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

//////////////////////////////////////////////////////////////////////////
class ElementMaker : public RefCounted
{
public:
	virtual ElementPtr Create(SyncContext* syncContext, const XStringPtr& name, XGuid id, UserID ownerID, const XValue& startingValue) const = 0;
};
DECLARE_PTR(ElementMaker)


//////////////////////////////////////////////////////////////////////////
class ElementFactory
{
public:
	void RegisterMaker(ElementType type, const ElementMakerPtr& maker);

	ElementPtr Make(ElementType type, SyncContext* syncContext, const XStringPtr& name, XGuid id, UserID ownerID, const XValue& startingValue) const;

private:
	std::map<ElementType, ElementMakerPtr> m_makers;
};


//////////////////////////////////////////////////////////////////////////
template<typename T>
class ElementMakerT : public ElementMaker
{
public:
	virtual ElementPtr Create(SyncContext* syncContext, const XStringPtr& name, XGuid id, UserID , const XValue& startingValue) const XTOVERRIDE
	{
		return new T(syncContext, name, id, startingValue);
	}
};

//////////////////////////////////////////////////////////////////////////
template<typename T>
class ElementMakerWithOwnerT : public ElementMaker
{
public:
	virtual ElementPtr Create(SyncContext* syncContext, const XStringPtr& name, XGuid id, UserID ownerID, const XValue& startingValue) const XTOVERRIDE
	{
		return new T(syncContext, name, id, ownerID, startingValue);
	}
};

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
