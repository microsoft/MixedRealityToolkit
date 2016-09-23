//////////////////////////////////////////////////////////////////////////
// FilePersistentSessionProvider.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "SyncData.h"
#include "SyncElementSerializer.h"
#include <filesystem>

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

class FileSystemSyncDataProvider : public SyncDataProvider
{
public:

	typedef std::experimental::filesystem::v1::path path;

	static ref_ptr<FileSystemSyncDataProvider> Create(
		const SyncElementSerializerPtr& serializer, const path& directory, const path& extension);

	// Inherited via SyncDataProvider
	virtual size_t DataCount() XTOVERRIDE;
	virtual std::string GetDataName(int index) XTOVERRIDE;
	virtual SyncDataPtr GetData(int index) XTOVERRIDE;
	virtual SyncDataPtr FindData(std::string name) XTOVERRIDE;
	virtual SyncDataPtr FindOrCreateData(std::string name) XTOVERRIDE;

private:

	FileSystemSyncDataProvider(const FileSystemSyncDataProvider&) {} // No copy constructor
	FileSystemSyncDataProvider(const SyncElementSerializerPtr& serializer, const path& directory, const path& extension);

private:
	path m_directory;
	path m_extension;

	std::vector<SyncDataPtr> m_syncData;
	std::vector<std::string> m_syncDataNames;
	SyncElementSerializerPtr m_serializer;
};

DECLARE_PTR(FileSystemSyncDataProvider)

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END