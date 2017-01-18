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


int _tmain(int argc, _TCHAR* argv[])
{
	if ((argc > 1) && ((*argv[1] == L'-' || (*argv[1] == L'/'))))
	{
		if (_wcsicmp(L"install", argv[1] + 1) == 0)
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
		else if (_wcsicmp(L"remove", argv[1] + 1) == 0)
		{
			// Uninstall the service when the command is 
			// "-remove" or "/remove".
			UninstallService(SERVICE_NAME);
		}
		else if (_wcsicmp(L"local", argv[1] + 1) == 0)
		{
			wprintf(L"Running Sharing Service locally.  Enter 'q' to quit.  \n");

			XTools::SessionServer* server = new XTools::SessionServer(SERVICE_NAME);
			server->OnStart(0, NULL);

			char input = '\0';
			while (input != 'q')
			{
				std::cin >> input;
			}

			server->OnStop();

			delete server;
		}
	}
	else
	{
		wprintf(L"Parameters:\n");
		wprintf(L" -install  to install the service.\n");
		wprintf(L" -remove   to remove the service.\n");
		wprintf(L" -local    to run from the command line, not as a service.\n");

		XTools::SessionServer server(SERVICE_NAME);
		if (!ServiceBase::Run(server))
		{
			wprintf(L"Service failed to run w/err 0x%08lx\n", GetLastError());
		}
	}

	return 0;
}
