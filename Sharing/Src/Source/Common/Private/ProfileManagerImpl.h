//////////////////////////////////////////////////////////////////////////
// ProfileManagerImpl.h
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "ProfileEntry.h"

XTOOLS_NAMESPACE_BEGIN

class ProfileManagerImpl : 
	public ProfileManager, 
	public IncomingXSocketListener,
	public NetworkConnectionListener,
	public IUpdateable
{
	XTOOLS_REFLECTION_DECLARE(ProfileManagerImpl)

public:
	static ref_ptr<ProfileManagerImpl> GetInstance();

	ProfileManagerImpl(const XSocketManagerPtr& socketMgr, SystemRole role);
	virtual ~ProfileManagerImpl();

	virtual void BeginRange(const std::string& name) XTOVERRIDE;
	virtual void EndRange() XTOVERRIDE;

	virtual void Log(LogSeverity severity, const std::string& message) XTOVERRIDE;

	virtual void Update() XTOVERRIDE;

private:

	struct ThreadInfo : public AtomicRefCounted
	{
		std::vector<ProfileEntry>	m_entries;
		Mutex						m_mutex;
		uint64						m_threadID;
		int32						m_currentIndex;
	};
	DECLARE_PTR(ThreadInfo)


	// IncomingXSocketListener Functions:
	virtual void OnNewConnection(const XSocketPtr& newConnection) XTOVERRIDE;

	// NetworkConnectionListener Functions:
	virtual void OnConnected(const NetworkConnectionPtr& connection) XTOVERRIDE;
	virtual void OnDisconnected(const NetworkConnectionPtr& connection) XTOVERRIDE;

	// Local Functions:
	ThreadInfoPtr GetLocalThreadInfo();

	static ProfileManagerImpl*	m_sInstance;
	static Mutex			m_sInstanceMutex;

	std::map<uint64, ThreadInfoPtr> m_threadEntries;
	Mutex							m_threadStackMutex;

	struct LogEntry
	{
		LogSeverity m_severity;
		std::string m_message;
	};

	std::vector<LogEntry>	m_logMessages;
	Mutex					m_logMutex;

	XSocketManagerPtr		m_socketMgr;
	NetworkConnectionPtr	m_connection;
	ReceiptPtr				m_incomingConnectionReceipt;
	ReceiptPtr				m_discoveryResponseReceipt;

	volatile int			m_isConnected;
};

DECLARE_PTR(ProfileManagerImpl)

XTOOLS_NAMESPACE_END
