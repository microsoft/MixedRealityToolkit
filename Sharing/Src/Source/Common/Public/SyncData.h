//////////////////////////////////////////////////////////////////////////
// PersistentSessionProvider.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

class SyncData : public RefCounted
{
public:
	virtual std::string Name() = 0;
	virtual void Load(const ObjectElementPtr& syncRoot) = 0;
	virtual void Save(const ObjectElementConstPtr& syncRoot) = 0;
};

DECLARE_PTR(SyncData);


class SyncDataProvider : public RefCounted
{
public:

	virtual size_t DataCount() = 0;
	virtual SyncDataPtr GetData(int index) = 0;
};

DECLARE_PTR(SyncDataProvider)

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END