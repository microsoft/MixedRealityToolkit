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

	FileSystemSyncDataProvider(const SyncElementSerializerPtr& serializer, const char* directory, const char* extension);

	// Inherited via SyncDataProvider
	virtual size_t DataCount() override;
	virtual SyncDataPtr GetData(int index) override;

private:

	std::vector<SyncDataPtr> m_syncData;
	SyncElementSerializerPtr m_serializer;
};

DECLARE_PTR(FileSystemSyncDataProvider)

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END