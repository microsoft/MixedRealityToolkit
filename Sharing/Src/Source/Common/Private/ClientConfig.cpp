//////////////////////////////////////////////////////////////////////////
// ClientConfig.cpp
//
// Simple type to hold configuration settings to be passed to the SharingManager
// on the client to define how it should fit within the XTools network system
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Public/ClientConfig.h>

XTOOLS_NAMESPACE_BEGIN

ClientConfig::ClientConfig(ClientRole role)
: m_serverAddress("localhost")
, m_serverPort(kSessionServerPort)
, m_logger(NULL)
, m_role(role)
, m_isAudioEndpoint(false)
, m_bProfilerEnabled(true)
{

}


ClientRole ClientConfig::GetRole() const
{
	return m_role;
}


std::string ClientConfig::GetServerAddress() const
{
	return m_serverAddress;
}


bool ClientConfig::SetServerAddress(const std::string& serverAddress)
{
	if (m_role == ClientRole::Primary)
	{
		m_serverAddress = serverAddress;
		return true;
	}
	else
	{
		LogWarning("Cannot set the server address on a client with a secondary role");
		return false;
	}
}


int32 ClientConfig::GetServerPort() const
{
	return m_serverPort;
}


bool ClientConfig::SetServerPort(int32 port)
{
	if (m_role == ClientRole::Primary)
	{
		m_serverPort = port;
		return true;
	}
	else
	{
		LogWarning("Cannot set the server port on a client with a secondary role");
		return false;
	}
}


LogWriter* ClientConfig::GetLogWriter() const
{
	return m_logger;
}


void ClientConfig::SetLogWriter(LogWriter* logger)
{
	m_logger = logger;
}


bool ClientConfig::GetIsAudioEndpoint() const
{
	return m_isAudioEndpoint;
}


void ClientConfig::SetIsAudioEndpoint(bool isAudioEndpoint)
{
	m_isAudioEndpoint = isAudioEndpoint;
}


bool ClientConfig::GetProfilerEnabled() const
{
	return m_bProfilerEnabled;
}


void ClientConfig::SetProfilerEnabled(bool enabled)
{
	m_bProfilerEnabled = enabled;
}

XTOOLS_NAMESPACE_END
