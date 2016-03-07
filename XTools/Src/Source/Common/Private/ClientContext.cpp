//////////////////////////////////////////////////////////////////////////
// ClientContext.cpp
//
// 
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Public/ClientContext.h>
#include <Private/UserImpl.h>
#include <Private/UserPresenceManagerLeaderImpl.h>
#include <Private/UserPresenceManagerFollowerImpl.h>
#include <Private/TunnelConnection.h>
#include <Private/AudioManagerImpl.h>
#include <random>

XTOOLS_NAMESPACE_BEGIN

ClientContext::ClientContext(const ClientConfigPtr& config)
: m_connectionMgr(XSocketManager::Create())
, m_clientRole(ClientRole::Primary)
{
	if (config)
	{
		m_clientRole = config->GetRole();
	}

	// Create network connection objects for both the server and the paired client
	NetworkMessagePoolPtr messagePool = new NetworkMessagePool(kDefaultMessagePoolSize);

	m_pairedConnection = new NetworkConnectionImpl(messagePool);

	if (m_clientRole == ClientRole::Primary)
	{
		m_serverConnection = new NetworkConnectionImpl(messagePool);
		m_sessionConnection = new NetworkConnectionImpl(messagePool);
	}
	else
	{
		// Create a tunneled connection object for the server (via the paired app)
		m_sessionConnection = new TunnelConnection(m_pairedConnection);
	}

	
	if (m_clientRole == ClientRole::Primary)
	{
		UserID userID;
		do {
			std::random_device randomGenerator;
			userID = randomGenerator();
		} while (userID == 0 || userID == User::kInvalidUserID);

		LogInfo("Local User ID: %u", userID);

		// Name and Mute state will get populated later by the Session Manager.
		m_localUser = new UserImpl("UnknownUser", userID, false);
	}
	else
	{
		// Use an invalid user for now; we will assume the identity of the paired user once connected to the paired client
		m_localUser = new UserImpl();
	}


	Sync::AuthorityLevel authLevel = (m_clientRole == ClientRole::Primary) ? Sync::AuthorityLevel::Medium : Sync::AuthorityLevel::Low;
	m_syncMgr = Sync::SyncManager::Create(MessageID::SyncMessage, authLevel, m_localUser);
	m_internalSyncMgr = Sync::SyncManager::Create(MessageID::InternalSyncMessage, authLevel, m_localUser);

	// Add the paired connection to the sync manager.  
	// The sessions will have to connect the sync manager to their individual session connections
	m_syncMgr->AddConnection(m_pairedConnection);
	m_internalSyncMgr->AddConnection(m_pairedConnection);

	// Setup the user presence manager
	if (m_clientRole == ClientRole::Primary)
	{
		m_userPresenceMgr = new UserPresenceManagerLeaderImpl(m_pairedConnection, m_localUser);
	}
	else
	{
		m_userPresenceMgr = new UserPresenceManagerFollowerImpl(m_pairedConnection);
	}

	// Setup the audio manager (if required)
	if (config->GetIsAudioEndpoint())
	{
		// Secondary clients get an Audio Manager
		m_audioManager = new AudioManagerImpl(m_sessionConnection);
	}
}


const XSocketManagerPtr& ClientContext::GetXSocketManager() const
{
	return m_connectionMgr;
}


const NetworkConnectionPtr& ClientContext::GetSessionListConnection() const
{
	return m_serverConnection;
}


const NetworkConnectionPtr& ClientContext::GetSessionConnection() const
{
	return m_sessionConnection;
}


const NetworkConnectionPtr& ClientContext::GetPairedConnection() const
{
	return m_pairedConnection;
}


const Sync::SyncManagerPtr& ClientContext::GetSyncManager() const
{
	return m_syncMgr;
}


const Sync::SyncManagerPtr& ClientContext::GetInternalSyncManager() const
{
	return m_internalSyncMgr;
}


const UserPresenceManagerPtr& ClientContext::GetUserPresenceManager() const
{
	return m_userPresenceMgr;
}


const AudioManagerPtr& ClientContext::GetAudioManager() const
{
	return m_audioManager;
}


const UserPtr& ClientContext::GetLocalUser() const
{
	return m_localUser;
}


ClientRole ClientContext::GetClientRole() const
{
	return m_clientRole;
}


bool ClientContext::IsPrimaryClient() const
{
	return (m_clientRole == ClientRole::Primary);
}


XTOOLS_NAMESPACE_END