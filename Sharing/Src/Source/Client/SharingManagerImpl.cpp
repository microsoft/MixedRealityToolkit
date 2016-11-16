//////////////////////////////////////////////////////////////////////////
// SharingManagerImpl.cpp
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SharingManagerImpl.h"
#include "DynamicLibrary.h"
#include "SideCarContextImpl.h"
#include "PairingManagerImpl.h"
#include <Private/ProfileManagerImpl.h>
#include <Private/SessionManagerImpl.h>
#include <Private/RoomManagerImpl.h>
#include <exception>

#if defined(XTOOLS_PLATFORM_WINDOWS_ANY)
#define STRING_FORMAT_CHARACTER "%ls"
#else
#define STRING_FORMAT_CHARACTER "%s"
#endif

// Define our overloads of the new and delete operators so we can track memory
//XT_TRACK_MEMORY

XTOOLS_NAMESPACE_BEGIN

// Statics

// Retry the server once a second.
const int SharingManagerImpl::s_serverConnectRetryRateTicks = 3 * CLOCKS_PER_SEC;

SharingManagerPtr SharingManager::Create(const ClientConfigPtr& config)
{
	return new SharingManagerImpl(config);
}

SharingManagerImpl::SharingManagerImpl(const ClientConfigPtr& config)
	: m_clientContext(new ClientContext(config))
	, m_settings(new Settings())
	, m_logManager(new Logger())
{
#if defined(XTOOLS_DEBUG)
	// Enable debug memory tracking and verification. 
	MemoryManager::EnableDebugAllocators();
#endif

	if (config && config->GetLogWriter())
	{
		m_logManager->SetWriter(config->GetLogWriter());
	}

	std::string machineName = Platform::GetLocalMachineNetworkName();
	LogInfo("Local Machine Name: %s", machineName.c_str());

	if (config)
	{
		// Put the values from the ClientConfig into the generic settings object
		m_settings->ApplySetting(Settings::kServerAddressSettingName, config->GetServerAddress());
		m_settings->ApplySetting(Settings::kServerPortSettingName, std::to_string(config->GetServerPort()));
	}

	// Load a settings config file.  Any values on the loaded config file will overwrite the settings passed in by the ClientConfig object
	m_settings->LoadSettings();

	if (m_clientContext->IsPrimaryClient())
	{
		// Set the user's name from the settings in case it was specified in the xtoolsconfig.txt file
		m_clientContext->GetUserPresenceManager()->SetName(m_settings->GetLocalUserName());

		// Create the session manager only for Primary clients
		m_sessionManager = new SessionManagerImpl(m_clientContext, m_clientContext->GetUserPresenceManager());
	}

	// Create the profile manager
	if (config->GetProfilerEnabled())
	{
		SystemRole systemRole = (m_clientContext->GetClientRole() == Primary) ? SystemRole::PrimaryClientRole : SystemRole::SecondaryClientRole;
		m_profileManager = new ProfileManagerImpl(m_clientContext->GetXSocketManager(), systemRole);
	}

	// Create the pairing manager
	m_paringManager = new PairingManagerImpl(m_clientContext);

	// Allocate the context for the sidecars
	m_sidecarContext = new SideCarContextImpl(m_clientContext->GetPairedConnection());

	m_roomManager = new RoomManagerImpl(m_clientContext);

	// Load and initialize the 'sidecar' plug-ins
	//SetupSideCars(); // TODO: No SideCar support for BUILD 2016.

	// Register to receive connection updates from the viewer.  
	m_clientContext->GetPairedConnection()->AddListener(MessageID::StatusOnly, this);

	if (m_clientContext->IsPrimaryClient())
	{
		LogInfo("Connecting to Server at %s:%i", m_settings->GetServerAddress()->GetString().c_str(), m_settings->GetServerPort());

		// Register to receive connection updates from the server.  
		m_clientContext->GetSessionListConnection()->AddListener(MessageID::StatusOnly, this);

		ConnectToServer();
	}

	// Add the update-able subsystems in the order they should get updated
	m_updateableSubsystems.push_back(m_clientContext->GetXSocketManager().get());

	m_updateableSubsystems.push_back(m_clientContext->GetInternalSyncManager().get());

	m_updateableSubsystems.push_back(m_clientContext->GetSyncManager().get());

	m_updateableSubsystems.push_back(reflection_cast<IUpdateable>(m_paringManager));
}


SharingManagerImpl::~SharingManagerImpl()
{
	LogInfo("SharingManager shutting down");

	// Manually release all the smart pointers so they can print any errors to
	// the logger before it gets cleared

	m_updateableSubsystems.clear();

	if (m_clientContext->IsPrimaryClient())
	{
		m_clientContext->GetSessionListConnection()->RemoveListener(MessageID::StatusOnly, this);
	}
	m_clientContext->GetPairedConnection()->RemoveListener(MessageID::StatusOnly, this);

	m_sidecars.clear();
	m_sidecarContext = nullptr;

	m_sessionManager = nullptr;

	m_paringManager = nullptr;

	m_profileManager = nullptr;

	m_roomManager = nullptr;

	m_clientContext = nullptr;

	m_settings = nullptr;

	m_listServerHandshake = nullptr;

	m_logManager->ClearWriter();
}


const SessionManagerPtr& SharingManagerImpl::GetSessionManager() const
{
	return m_sessionManager;
}


const UserPresenceManagerPtr& SharingManagerImpl::GetUserPresenceManager() const
{
	return m_clientContext->GetUserPresenceManager();
}

const AudioManagerPtr& SharingManagerImpl::GetAudioManager() const
{
	return m_clientContext->GetAudioManager();
}


const PairingManagerPtr& SharingManagerImpl::GetPairingManager() const
{
	return m_paringManager;
}


ObjectElementPtr SharingManagerImpl::GetRootSyncObject()
{
	return m_clientContext->GetSyncManager()->GetRootObject();
}


const RoomManagerPtr& SharingManagerImpl::GetRoomManager() const
{
	return m_roomManager;
}


void SharingManagerImpl::Update()
{
	{
		PROFILE_SCOPE("SharingManager Update");

		// This triggers callbacks, which could remove references to this object.  
		// So we create a smart pointer reference here to ensure that this object 
		// is not destroyed before this function returns.  
		SharingManagerPtr thisPtr;

		if (m_retryServerConnectionRequired)
		{
			bool timeout = (clock() - m_lastServerRetryAttemptTick) > s_serverConnectRetryRateTicks;
			if (timeout)
			{
				ConnectToServer();
			}
		}


		for (size_t i = 0; i < m_updateableSubsystems.size(); ++i)
		{
			IUpdateable* currentSubsystem = m_updateableSubsystems[i];
			if (currentSubsystem)
			{
				currentSubsystem->Update();
			}
		}


		{
			ScopedProfile sidecarUpdateProfiler("Update Sidecars");
			for (size_t i = 0; i < m_sidecars.size(); ++i)
			{
				try
				{
					m_sidecars[i].m_sidecar->Update();
				}
				catch (...)
				{
					LogError("Exception occured when updating sidecar " STRING_FORMAT_CHARACTER, m_sidecars[i].m_name.c_str());
				}
			}
		}
		

#if defined(XTOOLS_PLATFORM_WINDOWS_ANY) && defined(XTOOLS_DEBUG)
		// Check to see if there has been a stomp or buffer overrun
		// Does nothing in Release
		{
			ScopedProfile validateMemoryProfiler("Validate Memory");
			MemoryManager::ValidateMemory();
		}
#endif
	}

	// Tick the profiler last
	if (m_profileManager)
	{
		reflection_cast<IUpdateable>(m_profileManager)->Update();
	}
}


NetworkConnectionPtr SharingManagerImpl::GetPairedConnection()
{
	return m_clientContext->GetPairedConnection();
}


NetworkConnectionPtr SharingManagerImpl::GetServerConnection()
{
	return m_clientContext->GetSessionConnection();
}


#if defined(MSTEST)
NetworkConnectionPtr SharingManagerImpl::GetListServerConnection()
{
	return m_clientContext->GetSessionListConnection();
}
#endif


const SettingsPtr& SharingManagerImpl::GetSettings()
{
	return m_settings;
}


void SharingManagerImpl::SetServerConnectionInfo(const XStringPtr& address, uint32 port)
{
	if (!address)
	{
		LogError("Null address passed to SetServerConnectionInfo");
		return;
	}

	LogInfo("Changing Server to %s:%u...", address->GetString().c_str(), port);
	const std::string portString = std::to_string(port);
	m_settings->ApplySetting(Settings::kServerAddressSettingName, address->GetString());
	m_settings->ApplySetting(Settings::kServerPortSettingName, portString);

	SessionPtr currentSession = m_sessionManager->GetCurrentSession();

	if (currentSession)
	{
		currentSession->Leave();
	}

	// Disconnect and reconnect to server.
	NetworkConnectionPtr serverConnection = m_clientContext->GetSessionListConnection();
	if (serverConnection->IsConnected())
	{
		// This will call through to OnDisconnected, which will initiate a reconnection
		serverConnection->Disconnect();
	}
	else
	{
		ConnectToServer();
	}
}


UserPtr SharingManagerImpl::GetLocalUser()
{
	return m_clientContext->GetLocalUser();
}


void SharingManagerImpl::SetUserName(const XStringPtr& name)
{
	if (!name)
	{
		LogError("Null name passed to SetUserName");
		return;
	}
	else if (name->GetLength() < kMinUserNameLength)
	{
		LogError("Cannot change user's name: new name must be at least %i letters", kMinUserNameLength);
		return;
	}
	else if (name->GetLength() > kMaxUserNameLength)
	{
		LogError("Cannot change user's name: new name must be less than %i letters", kMaxUserNameLength);
		return;
	}

	m_settings->ApplySetting(Settings::kUserNameOverride, name->GetString());

	// Set the name of the local user
	m_clientContext->GetUserPresenceManager()->SetName(m_settings->GetLocalUserName());

	if (m_sessionManager)
	{
		m_sessionManager->UpdateCurrentUserOnServer();
	}
}


bool SharingManagerImpl::RegisterSyncListener(SyncListener* listener)
{
	return m_clientContext->GetSyncManager()->RegisterListener(listener);
}


void SharingManagerImpl::OnConnected(const NetworkConnectionPtr& connection)
{
	if (m_clientContext->IsPrimaryClient())
	{
		if (m_clientContext->GetSessionListConnection() == connection)
		{
			LogInfo("Connected to Session List Server");
		}
	}

	if (m_clientContext->GetPairedConnection() == connection)
	{
		LogInfo("Connected to Paired Client");
	}
}


void SharingManagerImpl::OnConnectFailed(const NetworkConnectionPtr& connection)
{
	if (m_clientContext->IsPrimaryClient())
	{
		if (m_clientContext->GetSessionListConnection() == connection)
		{
			LogInfo("Failed to connect to Session List Server.  Retrying...");
			ConnectToServer();
		}
	}

	if (m_clientContext->GetPairedConnection() == connection)
	{
		LogInfo("Conection Failed to Paired Client");
	}
}


void SharingManagerImpl::OnDisconnected(const NetworkConnectionPtr& connection)
{
	if (m_clientContext->IsPrimaryClient())
	{
		if (m_clientContext->GetSessionListConnection() == connection)
		{
			LogInfo("Disconnected from Session List Server.  Reconnecting...");
			ConnectToServer();
		}
	}
	else
	{
		// TODO: move this somewhere else
		m_clientContext->GetLocalUser()->SetID(User::kInvalidUserID);
		m_clientContext->GetLocalUser()->SetName(new XString(""));
	}

	if (m_clientContext->GetPairedConnection() == connection)
	{
		LogInfo("Disconnected from Paired Client");
	}
}


void SharingManagerImpl::ConnectToServer()
{
	PROFILE_FUNCTION();

	// Only the primary client should be calling this function
	XTASSERT(m_clientContext->IsPrimaryClient());

	m_lastServerRetryAttemptTick = clock();
	uint16 serverPort = static_cast<uint16>(m_settings->GetServerPort());

	// Create a connection to the session server
	XSocketPtr serverSocket = m_clientContext->GetXSocketManager()->OpenConnection(m_settings->GetServerAddress()->GetString(), serverPort);

	if (serverSocket)
	{
		// Initiate the handshake
		HandshakeCallback callback = CreateCallback3(this, &SharingManagerImpl::OnListServerHandshakeComplete);
		m_listServerHandshake = new NetworkHandshake(serverSocket, new SessionListHandshakeLogic(false), callback);
		m_retryServerConnectionRequired = false;
	}
	else
	{
		m_retryServerConnectionRequired = true;
	}
}


void SharingManagerImpl::SetupSideCars()
{
#if defined(XTOOLS_PLATFORM_WINDOWS_ANY)
	utility::string_t sidecarFolder(U(".\\SideCars\\"));
#elif defined(XTOOLS_PLATFORM_OSX)
	utility::string_t sidecarFolder(U("./SideCars/"));
#endif

	// Get a list of all the *.dll files in the sidecar folder
	std::vector<utility::string_t> files = GetLibrariesInFolder(sidecarFolder);

	// For each dll, call the entry point function to create a SideCar instance
	for (size_t i = 0; i < files.size(); ++i)
	{
		utility::string_t currentFile = files[i];

		LoadSideCar(sidecarFolder, currentFile);
	}
}


void SharingManagerImpl::LoadSideCar(const utility::string_t& sidecarFolder, const utility::string_t& currentFile)
{
	DynamicLibrary dynamicLibrary((sidecarFolder + currentFile).c_str());
	if (!dynamicLibrary.IsValid())
	{
		LogError("Failed to load SideCar DLL " STRING_FORMAT_CHARACTER, currentFile.c_str());
		const char* errorMessage = dynamicLibrary.GetErrorMessage();
		if (errorMessage)
		{
			LogError(errorMessage);
		}
		return;
	}

	// Set the logging function
	LogFuncSetter setLoggerFunc = dynamicLibrary.GetLogFunc();
	if (setLoggerFunc)
	{
		setLoggerFunc(&LogNative);
	}

	// Set the profiling function
	ProfileFuncSetter setProfileFunc = dynamicLibrary.GetProfileFunc();
	if (setProfileFunc)
	{
		setProfileFunc(&GetProfileManager);
	}

	// Get the entry point and create an instance of the sidecar
	SideCarEntryPointFunc entryPoint = dynamicLibrary.GetEntryPoint();
	if (entryPoint)
	{
		SideCarPtr newSideCar = entryPoint();
		if (newSideCar)
		{
			// Initialize the sidecar instance so that it can register for callbacks
			try
			{
				newSideCar->Initialize(m_sidecarContext);
			}
			catch (...)
			{
				LogError("Failed to initialize sidecar " STRING_FORMAT_CHARACTER, currentFile.c_str());
				return;
			}

			SideCarInfo newSidecarInfo;
			newSidecarInfo.m_sidecar = newSideCar;
			newSidecarInfo.m_name = currentFile;
			m_sidecars.push_back(newSidecarInfo);

			LogInfo(STRING_FORMAT_CHARACTER	" loaded successfully", currentFile.c_str());
		}
	}
	else
	{
		LogError("Failed to find entrypoint for " STRING_FORMAT_CHARACTER, currentFile.c_str());
	}
}


void SharingManagerImpl::OnListServerHandshakeComplete(const XSocketPtr& newConnection, SocketID, HandshakeResult result)
{
	XTASSERT(m_clientContext->IsPrimaryClient());

	m_listServerHandshake = nullptr;

	if (newConnection && result == HandshakeResult::Success)
	{
		// We've successfully connected.  Set the connection
		m_clientContext->GetSessionListConnection()->SetSocket(newConnection);
		m_retryServerConnectionRequired = false;
	}
	else if (result == HandshakeResult::FatalFailure)
	{
		LogInfo("Server handshake failed with a fatal error. Aborting connection.");
		m_retryServerConnectionRequired = false;
	}
	else
	{
		// We were unable to connect to the server; try again
		ConnectToServer();
	}
}



// TODO: Abstract platform specific implementations into their own classes to avoid the need for
// lots of #ifs
std::vector<utility::string_t> SharingManagerImpl::GetLibrariesInFolder(const utility::string_t& folder) const
{
	std::vector<utility::string_t> names;

#if defined(XTOOLS_PLATFORM_WINDOWS_DESKTOP)
	wchar_t search_path[200];
	wsprintf(search_path, L"%s*.dll", folder.c_str());
	WIN32_FIND_DATA fd;
	HANDLE hFind = ::FindFirstFile(search_path, &fd);
	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				names.push_back(fd.cFileName);
			}
		} while (::FindNextFile(hFind, &fd));
		::FindClose(hFind);
	}
	else
	{
		LogWarning("Failed to find directory " STRING_FORMAT_CHARACTER, folder.c_str());
	}
#elif defined(XTOOLS_PLATFORM_OSX)
	DIR* directory = nullptr;
	char realPath[PATH_MAX];
	realpath(folder.c_str(), realPath);
	directory = opendir(realPath);
	if (directory != nullptr)
	{
		class dirent* directoryEntry = nullptr;
		class stat directoryEntryStats;
		const utility::string_t libraryExtension(U(".dylib"));
		while ((directoryEntry = readdir(directory)) != nullptr)
		{
			utility::string_t fullPath = folder + directoryEntry->d_name;

			if (stat(fullPath.c_str(), &directoryEntryStats) == -1)
			{
				continue;
			}

			const bool isDirectory = (directoryEntryStats.st_mode & S_IFDIR) != 0;
			if (isDirectory)
			{
				continue;
			}

			if (fullPath.compare(fullPath.length() - libraryExtension.length(), libraryExtension.length(), libraryExtension) == 0)
			{
				names.push_back(directoryEntry->d_name);
			}
		}
		closedir(directory);
	}
#else
	// TODO: Support for platforms other than Windows Desktop and OSX
	XT_UNREFERENCED_PARAM(folder);
#endif

	return names;
}

XTOOLS_NAMESPACE_END