//////////////////////////////////////////////////////////////////////////
// Settings.cpp
//
// Provides access through the wrapper API to the config settings
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Settings.h"

XTOOLS_NAMESPACE_BEGIN

// Define Statics
const char* Settings::kSettingsFileName = "XToolsConfig.txt";

const char* Settings::kViewerAddressSettingName = "ViewerAddress";
const char* Settings::kViewerPortSettingName = "ViewerPort";

const char* Settings::kServerAddressSettingName = "ServerAddress";
const char* Settings::kServerPortSettingName = "ServerPort";

const char* Settings::kUserNameOverride = "UserNameOverride";

Settings::Settings()
: m_serverName(new XString())
, m_serverPort(kSessionServerPort)
, m_viewerName(new XString("localhost"))
, m_viewerPort(kAppPluginPort)
{
#if defined(XTOOLS_PLATFORM_WINDOWS_DESKTOP)
	// Get the user's login name and use it as the default
	const DWORD c_maxNameSize = 128;
	char userName[c_maxNameSize] = { 0 };
	DWORD nameLength = c_maxNameSize;
	GetUserNameA(userName, &nameLength);

	m_userName = new XString(userName);

#else
	m_userName = new XString("UnknownUser");

#endif
}


const XStringPtr& Settings::GetServerAddress() const
{
	return m_serverName;
}


int32 Settings::GetServerPort() const
{
	return m_serverPort;
}


const XStringPtr& Settings::GetViewerAddress() const
{
	return m_viewerName;
}


int32 Settings::GetViewerPort() const
{
	return m_viewerPort;
}


const XStringPtr& Settings::GetLocalUserName() const
{
	return m_userName;
}


bool Settings::LoadSettings()
{
	return LoadSettings(kSettingsFileName);
}


bool Settings::LoadSettings(const std::string& filename)
{
	std::fstream fs;
	fs.open(filename, std::fstream::in);
	if (fs.is_open())
	{
		char buffer[256];

		while (fs.good())
		{
			fs.getline(buffer, sizeof(buffer));

			if (strlen(buffer) > 0)
			{
				char *next_token = NULL;
				std::string keyString = strtok_s(buffer, " ", &next_token);
				char* valueString = strtok_s(NULL, " ", &next_token);

				// Check for commented out params
				bool isComment = keyString[0] == '#' || (keyString.length() >= 2 && keyString[0] == '/' && keyString[1] == '/');
				if (!isComment)
				{
					ApplySetting(keyString, valueString);
				}
			}
		}

		fs.close();

		return true;
	}
	else
	{
		return false;
	}
}


void Settings::ApplySetting(const std::string& key, const std::string& value)
{
	// This sucks, but works for now
	if (key == kServerAddressSettingName)
	{
		m_serverName = new XString(value);
	}
	else if (key == kServerPortSettingName)
	{
		m_serverPort = static_cast<uint16>(strtol(value.c_str(), NULL, 10));
	}
	else if (key == kViewerAddressSettingName)
	{
		m_viewerName = new XString(value);
	}
	else if (key == kViewerPortSettingName)
	{
		m_viewerPort = static_cast<uint16>(strtol(value.c_str(), NULL, 10));
	}
	else if (key == kUserNameOverride)
	{
		m_userName = new XString(value);
	}
}

XTOOLS_NAMESPACE_END
