//////////////////////////////////////////////////////////////////////////
// ProfilerStreamImpl.cpp
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ProfilerStreamImpl.h"
#include "Private/NetworkConnectionImpl.h"
#include "LogMessageImpl.h"

XTOOLS_NAMESPACE_BEGIN

ProfilerStreamImpl::ProfilerStreamImpl(std::string remoteSystemName, uint16 remotePort, const XSocketManagerPtr& socketMgr)
	: m_listenerList(ListenerList::Create())
	, m_socketMgr(socketMgr)
	, m_remoteSystemName(remoteSystemName)
	, m_remotePort(remotePort)
{
	// Create a network connection to receive the profiling data stream
	NetworkMessagePoolPtr messagePool = new NetworkMessagePool(kDefaultMessagePoolSize);
	m_connection = new NetworkConnectionImpl(messagePool);

	// Register as a listener for incoming profiling messages
	m_connection->AddListener(MessageID::Profiling, this);
}


void ProfilerStreamImpl::AddListener(ProfilerStreamListener* newListener)
{
	m_listenerList->AddListener(newListener);
}


void ProfilerStreamImpl::RemoveListener(ProfilerStreamListener* oldListener)
{
	m_listenerList->RemoveListener(oldListener);
}


bool ProfilerStreamImpl::IsConnected() const
{
	return m_connection->IsConnected();
}


void ProfilerStreamImpl::Connect()
{
	XSocketPtr socket = m_socketMgr->OpenConnection(m_remoteSystemName, m_remotePort);

	m_connection->SetSocket(socket);
}


void ProfilerStreamImpl::Disconnect()
{
	if (m_connection)
	{
		m_connection->Disconnect();
	}
}


std::string ProfilerStreamImpl::GetRemoteSystemName() const
{
	return m_remoteSystemName;
}


void ProfilerStreamImpl::OnConnected(const NetworkConnectionPtr&)
{
	m_listenerList->NotifyListeners(&ProfilerStreamListener::OnConnected);
}


void ProfilerStreamImpl::OnConnectFailed(const NetworkConnectionPtr&)
{
	m_listenerList->NotifyListeners(&ProfilerStreamListener::OnConnectFailed);
}


void ProfilerStreamImpl::OnDisconnected(const NetworkConnectionPtr& )
{
	m_listenerList->NotifyListeners(&ProfilerStreamListener::OnDisconnected);
}


void ProfilerStreamImpl::OnMessageReceived(const NetworkConnectionPtr& , NetworkInMessage& message)
{
	// Parse the incoming message

	ProfileFramePtr frame = new ProfileFrame();

	// Read the number of threads
	const int32 numThreads = message.ReadInt32();

	for (int32 threadIndex = 0; threadIndex < numThreads; ++threadIndex)
	{
		// Read the threadID
		const uint64 threadID = message.ReadUInt64();

		// Read the number of entries in this thread
		const uint32 numEntries = message.ReadUInt32();

		ProfileThreadPtr newThread = new ProfileThread(threadID, numEntries);

		// Read each entry
		for (uint32 i = 0; i < numEntries; ++i)
		{
			const std::string name = message.ReadStdString();
			const uint64 startTime = message.ReadUInt64();
			const uint64 duration = message.ReadUInt64();
			const int32 parentIndex = message.ReadInt32();

			newThread->AddSample(new ProfileSample(name, startTime, duration, parentIndex));
		}

		frame->AddThread(newThread);
	}


	// Read the number of log messages
	const uint32 numLogMessages = message.ReadUInt32();

	// Read each message
	for (uint32 i = 0; i < numLogMessages; ++i)
	{
		const LogSeverity severity = (LogSeverity)message.ReadByte();
		const std::string logMessage = message.ReadStdString();

		frame->AddMessage(new LogMessageImpl(severity, logMessage));
	}

	m_listenerList->NotifyListeners(&ProfilerStreamListener::OnReceiveProfileFrame, frame);
}


XTOOLS_NAMESPACE_END
