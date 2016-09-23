//////////////////////////////////////////////////////////////////////////
// FilePersistentSessionProvider.cpp
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Public/FileSystemSyncDataProvider.h>
#include <fstream>

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

using std::experimental::filesystem::v1::directory_iterator;
using namespace std::experimental::filesystem::v1;

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
			if (!exists(m_dataPath)) { return true; }
			if (file_size(m_dataPath) == 0) { return true; }

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

FileSystemSyncDataProviderPtr FileSystemSyncDataProvider::Create(
	const SyncElementSerializerPtr& serializer, const path& directory, const path& extension)
{
	// Need this for errors to print if a session hasn't started yet
	XTools::LoggerPtr logger = new XTools::Logger();

	if (!XTVERIFY(serializer != nullptr)) { return FileSystemSyncDataProviderPtr(); }

	if (!extension.has_extension() || extension.extension() != extension)
	{
		LogError("Extension(%s) was not an extension", extension.generic_string().c_str());
		return FileSystemSyncDataProviderPtr();
	}

	const bool directoryExists = exists(directory);
	if (directoryExists && !is_directory(directory))
	{
		LogError("Directory(%s) was not a directory", directory.generic_string().c_str());
		return FileSystemSyncDataProviderPtr();
	}
	else if (!directoryExists && !create_directory(directory))
	{
		LogError("Could not create directory(%s)", directory.generic_string().c_str());
		return FileSystemSyncDataProviderPtr();
	}

	return new FileSystemSyncDataProvider(serializer, directory, extension);
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
	if (!XTVERIFY(index >= 0 && index < (int)m_syncData.size()))
	{
		return SyncDataPtr();
	}

	return m_syncData[index];
}

SyncDataPtr FileSystemSyncDataProvider::FindData(std::string name)
{
	for (int i = 0; i < DataCount(); ++i)
	{
		if (m_syncDataNames[i] == name)
		{
			return m_syncData[i];
		}
	}

	return SyncDataPtr();
}

SyncDataPtr FileSystemSyncDataProvider::FindOrCreateData(std::string name)
{
	SyncDataPtr result = FindData(name);
	if (result == nullptr)
	{
		path filePath = (path(m_directory) / name).replace_extension(m_extension);
		if (!exists(filePath))
		{
			// Create an empty sync document
			std::ofstream file(filePath, std::ios::binary);
			if (file.is_open())
			{
				m_serializer->Save(file, nullptr);
				file.close();
			}
		}

		result = new FileSystemSyncData(m_serializer, filePath);
		m_syncDataNames.push_back(name);
		m_syncData.push_back(result);
	}

	return result;
}

FileSystemSyncDataProvider::FileSystemSyncDataProvider(
	const SyncElementSerializerPtr& serializer, const path& directory, const path& extension)
{
	m_serializer = serializer;
	m_directory = directory;
	m_extension = extension;

	for (const auto& entry : directory_iterator(m_directory))
	{
		if (!is_regular_file(entry.status())) { continue; }
		if (entry.path().extension() != m_extension) { continue; }

		m_syncDataNames.push_back(entry.path().filename().replace_extension().generic_string());
		m_syncData.push_back(new FileSystemSyncData(serializer, entry.path()));
	}
}

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
