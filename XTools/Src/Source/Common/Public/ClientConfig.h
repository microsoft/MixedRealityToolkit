//////////////////////////////////////////////////////////////////////////
// ClientConfig.h
//
// Simple type to hold configuration settings to be passed to the XToolsManager
// on the client to define how it should fit within the XTools network system
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

// Define the role that the client should take.  This defines the connection behavior of the client.  
enum ClientRole : byte
{
	// A Primary client 
	// - Connects directly to the session server itself
	// - Is responsible for joining and leaving sessions
	// - Can tunnel the network traffic of a secondary client
	// - Has sync auth level Medium
	Primary = 0,

	// A Secondary client
	// - Connects to a Primary client, which tunnels its network traffic to the server
	// - Is not able to join or leave sessions on its own. 
	// - Has sync auth level Low
	Secondary
};


class ClientConfig : public AtomicRefCounted
{
public:
	ClientConfig(ClientRole role);

	// Returns the role of this client
	ClientRole GetRole() const;

	// Returns the machine name or IPv4 address of the Session Discovery Server to connect to
	std::string GetServerAddress() const;

	// Set the machine name or IPv4 address of the Session Discovery Server to connect to.
	// Returns false if the client's role prevents it from connecting directly to the server
	bool SetServerAddress(const std::string& serverAddress);

	// Return the port of the Session Discovery Server to connect to
	int32 GetServerPort() const;

	// Set the port of the Session Discovery Server to connect to.  
	// Returns false if the client's role prevents it from connecting directly to the server
	bool SetServerPort(int32 port);

	// Returns a pointer to a class through which all logging will flow, if any.
	LogWriter* GetLogWriter() const;

	// Provide a pointer to a class through which all logging will flow. 
	void SetLogWriter(LogWriter* logger);

	// Returns whether or not to provide audio endpoint (speaker / mic) services.
	// Default is off (false).
	bool GetIsAudioEndpoint() const;

	// Set whether or not to provide audio endpoint (speaker / mic) services.
	// Default is off (false).
	void SetIsAudioEndpoint(bool isAudioEndpoint);

private:
	std::string m_serverAddress;

	int32		m_serverPort;

	LogWriter*	m_logger;

	ClientRole	m_role;

	bool		m_isAudioEndpoint;
};

DECLARE_PTR(ClientConfig)

XTOOLS_NAMESPACE_END