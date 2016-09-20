//////////////////////////////////////////////////////////////////////////
// PersistentSessionProvider.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

/// Provides abstracted save/load functionality for Sync::Element hierarchies
class SyncData : public AtomicRefCounted
{
public:
	virtual bool Load(const ObjectElementPtr& syncRoot) = 0;
	virtual bool Save(const ObjectElementConstPtr& syncRoot) = 0;
};

DECLARE_PTR(SyncData);

/// Used to provide a set of known SyncData objects from different sources, e.g. file system, http, etc.
class SyncDataProvider : public RefCounted
{
public:

	virtual size_t DataCount() = 0;
	virtual std::string GetDataName(int index) = 0;
	virtual SyncDataPtr GetData(int index) = 0;
};

DECLARE_PTR(SyncDataProvider)

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END