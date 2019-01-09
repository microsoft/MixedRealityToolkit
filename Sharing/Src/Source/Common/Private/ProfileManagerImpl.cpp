//////////////////////////////////////////////////////////////////////////
// ProfileManagerImpl.cpp
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ProfileManagerImpl.h"

XTOOLS_NAMESPACE_BEGIN

XTOOLS_REFLECTION_DEFINE(ProfileManager);

XTOOLS_REFLECTION_DEFINE(ProfileManagerImpl)
.BaseClass<ProfileManager>()
.BaseClass<IUpdateable>();

ProfileManagerPtr GetProfileManager()
{
	return ProfileManagerImpl::GetInstance();
}

ProfileManagerImpl* ProfileManagerImpl::m_sInstance = nullptr;
Mutex ProfileManagerImpl::m_sInstanceMutex;

//static 
ProfileManagerImplPtr ProfileManagerImpl::GetInstance()
{
	return m_sInstance;
}

ProfileManagerImpl::ProfileManagerImpl(const XSocketManagerPtr& socketMgr, SystemRole role)
	: m_socketMgr(socketMgr)
	, m_isConnected(0)
{
	// Create a network connection object
	NetworkMessagePoolPtr messagePool = new NetworkMessagePool(kDefaultMessagePoolSize);
	m_connection = new NetworkConnectionImpl(messagePool);

	// Listen for events from the network connection
	m_connection->AddListener(MessageID::Profiling, this);

	// Start listening for incoming connections
	uint16 port;
	switch (role)
	{
	case XTools::SessionDiscoveryServerRole:
		port = kProfilingPortServer;
		// Start the discovery server to allow this instance to be discovered
		m_discoveryResponseReceipt = m_socketMgr->AcceptDiscoveryPings(kDiscoveryServerPort, role);
		break;
	case XTools::SessionServerRole:
		port = kProfilingPortServer;
		break;
	case XTools::PrimaryClientRole:
		port = kProfilingPortPrimaryClient;
		break;
	case XTools::SecondaryClientRole:
	default:
		port = kProfilingPortSecondaryClient;
		break;
	}

	m_incomingConnectionReceipt = m_socketMgr->AcceptConnections(port, 1, this);
	if (!m_incomingConnectionReceipt)
	{
		LogWarning("Failed to start listening for incoming profiler connections.  This usually means another app is already using the port");
	}

	{
		ScopedLock lock(m_sInstanceMutex);
		if (XTVERIFY(m_sInstance == nullptr))
		{
			m_sInstance = this;
		}
	}
}


ProfileManagerImpl::~ProfileManagerImpl()
{
	ScopedLock lock(m_sInstanceMutex);
	if (XTVERIFY(m_sInstance == this))
	{
		m_sInstance = nullptr;
	}
}


void ProfileManagerImpl::BeginRange(const std::string& name)
{
	if (m_isConnected)
	{
		ThreadInfoPtr threadInfo = GetLocalThreadInfo();
		if (threadInfo)
		{
			ProfileEntry newEntry(name, threadInfo->m_currentIndex);
			newEntry.Start();

			{
				ScopedLock lock(threadInfo->m_mutex);
				threadInfo->m_entries.push_back(newEntry);

				threadInfo->m_currentIndex = (int32)threadInfo->m_entries.size() - 1;
			}
		}
	}
}


void ProfileManagerImpl::EndRange()
{
	if (m_isConnected)
	{
		ThreadInfoPtr threadInfo = GetLocalThreadInfo();
		if (threadInfo)
		{
			ScopedLock lock(threadInfo->m_mutex);

			uint32 endingIndex = threadInfo->m_currentIndex;
			if (endingIndex != -1)
			{
				ProfileEntry& entry = threadInfo->m_entries[endingIndex];

				entry.End();

				threadInfo->m_currentIndex = entry.GetParent();
			}
		}
	}
}


void ProfileManagerImpl::Log(LogSeverity severity, const std::string& message)
{
	if (m_isConnected)
	{
		ScopedLock lock(m_logMutex);
		m_logMessages.push_back(LogEntry{ severity, message });
	}
}


void ProfileManagerImpl::Update()
{
	// If we're connected to the profiling tool...
	if (m_isConnected)
	{
		// Send the accumulated profiling entries to the profiler tool
		NetworkOutMessagePtr msg = m_connection->CreateMessage(MessageID::Profiling);

		EndRange();

		// Record the time it takes to pack up and send the data
		ProfileEntry sendInfoEntry("Send Profiling Data", 0);
		sendInfoEntry.Start();

		// Add the profiling entries to the outgoing packet
		{
			ScopedLock threadMapLock(m_threadStackMutex);

			// Write the number of threads
			int32 numThreads = (int32)m_threadEntries.size();
			msg->Write(numThreads);

			for (auto mapIt = m_threadEntries.begin(); mapIt != m_threadEntries.end(); ++mapIt)
			{
				ThreadInfoPtr threadInfo = mapIt->second;
				ScopedLock entryListLock(threadInfo->m_mutex);

				// Write the threadID
				msg->Write(threadInfo->m_threadID);

				std::vector<ProfileEntry>& entries = threadInfo->m_entries;

				// Write the number of entries in this thread
				uint32 numEntries = (uint32)threadInfo->m_entries.size();
				msg->Write(numEntries);

				// Write each entry
				for (size_t i = 0; i < numEntries; ++i)
				{
					msg->Write(entries[i].GetName());	// Write the name
					msg->Write(entries[i].GetStartTime());
					msg->Write(entries[i].GetDuration());
					msg->Write(entries[i].GetParent());
				}

				// Clear out the list and get ready for the next frame
				threadInfo->m_currentIndex = -1;
			}

			m_threadEntries.clear();
		}

		// Add the log messages to the packet
		{
			ScopedLock lock(m_logMutex);

			// Write the number of log messages
			uint32 numMessages = (uint32)m_logMessages.size();
			msg->Write(numMessages);

			for (uint32 i = 0; i < numMessages; ++i)
			{
				msg->Write((byte)m_logMessages[i].m_severity);
				msg->Write(m_logMessages[i].m_message);
			}

			m_logMessages.clear();
		}


		BeginRange("Profile Frame");

		m_connection->Send(msg, MessagePriority::Low, MessageReliability::ReliableOrdered, MessageChannel::ProfileChannel);

		// Add the time it took to pack up and send the data to the start of the next frame
		sendInfoEntry.End();
		ThreadInfoPtr threadInfo = GetLocalThreadInfo();
		if (threadInfo)
		{
			ScopedLock lock(threadInfo->m_mutex);
			threadInfo->m_entries.push_back(sendInfoEntry);
		}
	}
}


void ProfileManagerImpl::OnNewConnection(const XSocketPtr& newConnection)
{
	m_connection->SetSocket(newConnection);
}


void ProfileManagerImpl::OnConnected(const NetworkConnectionPtr& )
{
	m_isConnected = 1;
}


void ProfileManagerImpl::OnDisconnected(const NetworkConnectionPtr& )
{
	m_isConnected = 0;

	// Clear out the accumulated profile entries so far, since they won't be sent
	ScopedLock threadMapLock(m_threadStackMutex);
	for (auto mapIt = m_threadEntries.begin(); mapIt != m_threadEntries.end(); ++mapIt)
	{
		ThreadInfoPtr threadInfo = mapIt->second;
		ScopedLock entryListLock(threadInfo->m_mutex);

		threadInfo->m_entries.clear();
		threadInfo->m_currentIndex = -1;
	}
}


ProfileManagerImpl::ThreadInfoPtr ProfileManagerImpl::GetLocalThreadInfo()
{
	ThreadInfoPtr threadInfo;

	uint64 threadID = Platform::GetThreadID();

	{
		ScopedLock lock(m_threadStackMutex);

		threadInfo = m_threadEntries[threadID];
		if (threadInfo == nullptr)
		{
			threadInfo = new ThreadInfo();
			threadInfo->m_currentIndex = -1;
			threadInfo->m_threadID = threadID;
			m_threadEntries[threadID] = threadInfo;
		}
	}

	return threadInfo;
}

XTOOLS_NAMESPACE_END
