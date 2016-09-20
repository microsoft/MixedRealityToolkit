//////////////////////////////////////////////////////////////////////////
// Main.cpp
//
// The file defines the entry point of the application.  According to the
// arguments in the command line, the function installs or uninstalls or
// starts the service by calling into different routines.  Code from Windows
// Service sample code.  
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "../SharingService/SessionServer.h"
#include "../SharingService/ServiceInstaller.h"
#include "../Common/Public/FileSystemSyncDataProvider.h"
#include "../Common/Public/XMLSyncElementSerializer.h"
#include <iostream>
#include <iosfwd>

// 
// Settings of the service
// 

// Internal name of the service
#define SERVICE_NAME             L"SharingService"

// Displayed name of the service
#define SERVICE_DISPLAY_NAME     L"Sharing Service"

// Service start options.  
#define SERVICE_START_TYPE       SERVICE_AUTO_START	// start automatically by the service control manager during system startup

// List of service dependencies - "dep1\0dep2\0\0"
#define SERVICE_DEPENDENCIES     L""

// The user account to run this service as
#define SERVICE_ACCOUNT     L"NT AUTHORITY\\Network Service"

// The password of the user account to run this service as
#define SERVICE_PASSWORD     L""

// Track memory allocations
//XT_TRACK_MEMORY

namespace // Intentionally Anonymous
{
	bool is_flag(const _TCHAR* arg)
	{
		return (*arg == L'-' || *arg == L'/');
	}

	int find_flag(const _TCHAR* search, int argc, _TCHAR* argv[])
	{
		for (int i = 0; i < argc; ++i)
		{
			const _TCHAR* arg = argv[i];

			// Only consider entries that are flagged
			if (!is_flag(arg)) { continue; }
			if (_wcsicmp(search, arg + 1) == 0) return i;
		}

		return -1;
	}

	const _TCHAR* find_arg(const _TCHAR* search, int argc, _TCHAR* argv[], const _TCHAR* defaultValue = L"")
	{
		int flag = find_flag(search, argc, argv);
		if (flag < 0) return nullptr; // Flag not found
		if (flag + 1 >= argc) return defaultValue; // Found the flag, but no following arg
		if (is_flag(argv[flag + 1])) return defaultValue; // Found the flag, but following arg is another flag

		return argv[flag + 1];
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	if (find_flag(L"install", argc, argv) >= 0)
	{
		// Install the service when the command is 
		// "-install" or "/install".
		InstallService(
			SERVICE_NAME,               // Name of service
			SERVICE_DISPLAY_NAME,       // Name to display
			SERVICE_START_TYPE,         // Service start type
			SERVICE_DEPENDENCIES,       // Dependencies
			SERVICE_ACCOUNT,			// Service running account
			SERVICE_PASSWORD            // Password of the account
		);

		RunService(SERVICE_NAME);
	}
	else if (find_flag(L"remove", argc, argv) >= 0)
	{
		// Uninstall the service when the command is 
		// "-remove" or "/remove".
		UninstallService(SERVICE_NAME);
	}
	else
	{
		XTools::Sync::SyncDataProviderPtr dataProvider;

		// Look for save flag, if no path specified, use cwd
		if (const _TCHAR* path = find_arg(L"save", argc, argv, L"./"))
		{
			dataProvider = new XTools::Sync::FileSystemSyncDataProvider(
				new XTools::Sync::XMLSyncElementSerializer(true, false), path, L".sml");
		}

		XTools::SessionServer server(SERVICE_NAME, dataProvider);

		if (find_flag(L"local", argc, argv) >= 0)
		{
			wprintf(L"Running Sharing Service locally.  Enter 'q' to quit.  \n");

			server.OnStart(0, NULL);

			char input = '\0';
			while (input != 'q')
			{
				std::cin >> input;
			}

			server.OnStop();
		}
		else
		{
			wprintf(L"Parameters:\n");
			wprintf(L" -install  to install the service.\n");
			wprintf(L" -remove   to remove the service.\n");
			wprintf(L" -local    to run from the command line, not as a service.\n");

			if (!ServiceBase::Run(server))
			{
				wprintf(L"Service failed to run w/err 0x%08lx\n", GetLastError());
			}
		}
	}

	return 0;
}
