//////////////////////////////////////////////////////////////////////////
// ProfilerStreamManagerImpl.cpp
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ProfilerStreamManagerImpl.h"
#include "ProfilerStreamImpl.h"

XTOOLS_NAMESPACE_BEGIN

//static 
ProfilerStreamManagerPtr ProfilerStreamManager::Create()
{
	return new ProfilerStreamManagerImpl();
}


ProfilerStreamManagerImpl::ProfilerStreamManagerImpl()
	: m_socketMgr(XSocketManager::Create())
{

}


ProfilerStreamPtr ProfilerStreamManagerImpl::CreateStream(std::string remoteSystemName, SystemRole role)
{
	uint16 port;
	if (role == SystemRole::SessionDiscoveryServerRole || role == SystemRole::SessionServerRole)
	{
		port = kProfilingPortServer;
	}
	else if (role == SystemRole::PrimaryClientRole)
	{
		port = kProfilingPortPrimaryClient;
	}
	else
	{
		port = kProfilingPortSecondaryClient;
	}

	ProfilerStreamPtr newStream = new ProfilerStreamImpl(remoteSystemName, port, m_socketMgr);
	m_streams.push_back(newStream);

	return newStream;
}


void ProfilerStreamManagerImpl::CloseStream(const ProfilerStreamPtr& stream)
{
	stream->Disconnect();

	for (size_t i = 0; i < m_streams.size(); ++i)
	{
		if (stream == m_streams[i])
		{
			m_streams.erase(m_streams.begin() + i);
			break;
		}
	}
}


void ProfilerStreamManagerImpl::Update()
{
	m_socketMgr->Update();
}

XTOOLS_NAMESPACE_END
