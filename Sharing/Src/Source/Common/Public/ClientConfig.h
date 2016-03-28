//////////////////////////////////////////////////////////////////////////
// ClientConfig.h
//
// Simple type to hold configuration settings to be passed to the SharingManager
// on the client to define how it should fit within the XTools network system
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class ClientConfig : public AtomicRefCounted
{
public:
	ClientConfig(ClientRole role);

	/// Returns the role of this client
	ClientRole GetRole() const;

	/// Returns the machine name or IPv4 address of the Session Discovery Server to connect to
	std::string GetServerAddress() const;

	/// Set the machine name or IPv4 address of the Session Discovery Server to connect to.
	/// Returns false if the client's role prevents it from connecting directly to the server
	bool SetServerAddress(const std::string& serverAddress);

	/// Return the port of the Session Discovery Server to connect to
	int32 GetServerPort() const;

	/// Set the port of the Session Discovery Server to connect to.  
	/// Returns false if the client's role prevents it from connecting directly to the server
	bool SetServerPort(int32 port);

	/// Returns a pointer to a class through which all logging will flow, if any.
	LogWriter* GetLogWriter() const;

	/// Provide a pointer to a class through which all logging will flow. 
	void SetLogWriter(LogWriter* logger);

	/// Returns whether or not to provide audio endpoint (speaker / mic) services.
	/// Default is off (false).
	bool GetIsAudioEndpoint() const;

	/// Set whether or not to provide audio endpoint (speaker / mic) services.
	/// Default is off (false).
	void SetIsAudioEndpoint(bool isAudioEndpoint);

	/// Returns true if the profiler is enabled
	bool GetProfilerEnabled() const;

	/// Set whether or not the profiler should be enabled
	void SetProfilerEnabled(bool enabled);

private:
	std::string m_serverAddress;

	int32		m_serverPort;

	LogWriter*	m_logger;

	ClientRole	m_role;

	bool		m_isAudioEndpoint;

	bool		m_bProfilerEnabled;
};

DECLARE_PTR(ClientConfig)

XTOOLS_NAMESPACE_END