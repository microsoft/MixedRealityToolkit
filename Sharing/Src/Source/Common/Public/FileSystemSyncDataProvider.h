//////////////////////////////////////////////////////////////////////////
// FilePersistentSessionProvider.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "SyncData.h"
#include "SyncElementSerializer.h"

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

class FileSystemSyncDataProvider : public SyncDataProvider
{
public:

	FileSystemSyncDataProvider(const SyncElementSerializerPtr& serializer, const wchar_t* directory, const wchar_t* extension);

	// Inherited via SyncDataProvider
	virtual size_t DataCount() XTOVERRIDE;
	virtual std::string GetDataName(int index) XTOVERRIDE;
	virtual SyncDataPtr GetData(int index) XTOVERRIDE;

private:

	std::vector<SyncDataPtr> m_syncData;
	std::vector<std::string> m_syncDataNames;
	SyncElementSerializerPtr m_serializer;
};

DECLARE_PTR(FileSystemSyncDataProvider)

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END