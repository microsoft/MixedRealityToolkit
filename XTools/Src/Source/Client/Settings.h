//////////////////////////////////////////////////////////////////////////
// Settings.h
//
// Provides access through the wrapper API to the config settings
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class Settings : public AtomicRefCounted
{
public:
	Settings();

	const XStringPtr& GetServerAddress() const;

	int32 GetServerPort() const;

	const XStringPtr& GetViewerAddress() const;

	int32 GetViewerPort() const;

	const XStringPtr& GetLocalUserName() const;


#if !defined(SWIG)
    static const char* kSettingsFileName;

    static const char* kViewerAddressSettingName;
    static const char* kViewerPortSettingName;

    static const char* kServerAddressSettingName;
    static const char* kServerPortSettingName;

    static const char* kUserNameOverride;

    void ApplySetting(const std::string& key, const std::string& value);

	// Load the settings in the default file, if it exists.  This will replace any existing settings.
	// Returns true if successful
	bool LoadSettings();
#endif 

private:
	// Load the settings in the given file.  This will replace any existing settings.
	// Returns true if successful
	bool LoadSettings(const std::string& filename);

	std::map<std::string, std::string>	m_settings;

	XStringPtr m_serverName;
	int32	m_serverPort;

	XStringPtr m_viewerName;
	int32	m_viewerPort;

	XStringPtr m_userName;
};

DECLARE_PTR(Settings)

XTOOLS_NAMESPACE_END
