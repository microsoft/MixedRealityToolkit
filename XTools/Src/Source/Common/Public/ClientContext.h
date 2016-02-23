//////////////////////////////////////////////////////////////////////////
// ClientContext.h
//
// Container class for resources required by multiple parts of the client 
// management code.  Avoids the use of a bunch of singletons.
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class ClientContext : public AtomicRefCounted
{
public:
	ClientContext(const ClientConfigPtr& config);

	const XSocketManagerPtr&		GetXSocketManager() const;
	const NetworkConnectionPtr&		GetSessionListConnection() const;
	const NetworkConnectionPtr&		GetSessionConnection() const;
	const NetworkConnectionPtr&		GetPairedConnection() const;
	const Sync::SyncManagerPtr&		GetSyncManager() const;
	const Sync::SyncManagerPtr&		GetInternalSyncManager() const;
	const UserPresenceManagerPtr&	GetUserPresenceManager() const;

	// May be NULL if ClientConfig wasn't set to enable audio endpoint.
	const AudioManagerPtr&			GetAudioManager() const;

	const UserPtr&					GetLocalUser() const;

	ClientRole						GetClientRole() const;
	bool							IsPrimaryClient() const;

private:
	XSocketManagerPtr			m_connectionMgr;
	NetworkConnectionPtr		m_serverConnection;		// Connection to the session list server. 
	NetworkConnectionPtr		m_sessionConnection;	// Connection to the current session.  Will be disconnected if NOT in a session
	NetworkConnectionPtr		m_pairedConnection;		// Connection to the other client that this client is paired to
	Sync::SyncManagerPtr		m_syncMgr;
	Sync::SyncManagerPtr		m_internalSyncMgr;
	UserPresenceManagerPtr      m_userPresenceMgr;
	AudioManagerPtr				m_audioManager;

	UserPtr						m_localUser;
	ClientRole					m_clientRole;
};

DECLARE_PTR(ClientContext)

XTOOLS_NAMESPACE_END
