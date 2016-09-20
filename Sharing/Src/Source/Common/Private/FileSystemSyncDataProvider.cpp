//////////////////////////////////////////////////////////////////////////
// FilePersistentSessionProvider.cpp
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Public/FileSystemSyncDataProvider.h>
#include <filesystem>
#include <fstream>

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

using std::experimental::filesystem::v1::path;
using std::experimental::filesystem::v1::directory_iterator;
using std::experimental::filesystem::v1::is_directory;

namespace // Intentionally Anonymous
{
	class FileSystemSyncData : public SyncData
	{
	public:
		FileSystemSyncData(const SyncElementSerializerPtr& serializer, const path& path) 
			: m_dataPath(path) 
			, m_serializer(serializer)
		{
			
		}

		// Inherited via SyncData
		virtual bool Load(const ObjectElementPtr & syncRoot) XTOVERRIDE
		{
			std::ifstream file(m_dataPath, std::ios::binary);
			if (!file.is_open())
			{
				LogError("Could not open file(%s) for read", m_dataPath);
				return false;
			}

			return m_serializer->Load(file, syncRoot);
		}
		virtual bool Save(const ObjectElementConstPtr & syncRoot) XTOVERRIDE
		{
			// First we write out to the Next file
			path pathToNext = m_dataPath;
			pathToNext.replace_extension(".next");

			std::ofstream file(pathToNext, std::ios::binary);
			if (!file.is_open())
			{
				LogError("Could not open file(%s) for write", pathToNext);
				return false;
			}

			if (!m_serializer->Save(file, syncRoot))
			{
				LogError("Could not save to file(%s)", pathToNext);
				return false;
			}

			// Make sure the file is flushed.
			file.close();

			// Then rename Current->Previous and Next->Current; 
			// Then delete Previous
			path pathToPrev = m_dataPath;
			pathToPrev.replace_extension(".prev");
			rename(m_dataPath, pathToPrev);
			rename(pathToNext, m_dataPath);
			remove(pathToPrev);

			return true;
		}

	private:

		path m_dataPath;
		SyncElementSerializerPtr m_serializer;
	};
}

FileSystemSyncDataProvider::FileSystemSyncDataProvider(const SyncElementSerializerPtr& serializer, const wchar_t* _directory, const wchar_t* _extension)
{
	if (!XTVERIFY(serializer != nullptr)) { return; }
	if (!XTVERIFY(_directory != nullptr)) { return; }
	if (!XTVERIFY(_extension != nullptr)) { return; }

	path extension(_extension);
	if (!extension.has_extension())
	{
		LogError("Extension(%s) was not an extension", _extension);
		return;
	}

	path directory(_directory);
	if (!is_directory(directory))
	{
		LogError("Directory(%s) was not a directory", _directory);
		return;
	}
	
	for (const auto& entry : directory_iterator(directory))
	{
		if (!is_regular_file(entry.status())) { continue; }
		if (entry.path().extension() != extension) { continue; }
		m_syncDataNames.push_back(entry.path().filename().replace_extension().generic_string());
		m_syncData.push_back(new FileSystemSyncData(serializer, entry.path()));
	}
}

size_t FileSystemSyncDataProvider::DataCount()
{
	return m_syncData.size();
}


std::string FileSystemSyncDataProvider::GetDataName(int index)
{
	if (!XTVERIFY(index >= 0 && index < (int)m_syncDataNames.size()))
	{
		return std::string();
	}

	return m_syncDataNames[index];
}

SyncDataPtr FileSystemSyncDataProvider::GetData(int index)
{
	if (!XTVERIFY(index >=0 && index < (int)m_syncData.size()))
	{
		return SyncDataPtr();
	}

	return m_syncData[index];
}

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
