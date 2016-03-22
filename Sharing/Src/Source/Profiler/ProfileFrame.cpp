//////////////////////////////////////////////////////////////////////////
// ProfileFrame.cpp
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ProfileFrame.h"

XTOOLS_NAMESPACE_BEGIN

ProfileFrame::ProfileFrame()
{

}


int32 ProfileFrame::GetThreadCount() const
{
	return static_cast<int32>(m_threads.size());
}


ProfileThreadPtr ProfileFrame::GetThread(int32 threadIndex) const
{
	if (threadIndex < GetThreadCount())
	{
		return m_threads[threadIndex];
	}
	else
	{
		return nullptr;
	}
}


int32 ProfileFrame::GetLogMessageCount() const
{
	return static_cast<int32>(m_messages.size());
}


LogMessagePtr ProfileFrame::GetLogMessage(int32 messageIndex) const
{
	if (messageIndex < GetLogMessageCount())
	{
		return m_messages[messageIndex];
	}
	else
	{
		return nullptr;
	}
}


void ProfileFrame::AddThread(const ProfileThreadPtr& newThread)
{
	m_threads.push_back(newThread);
}


void ProfileFrame::AddMessage(const LogMessagePtr& newMessage)
{
	m_messages.push_back(newMessage);
}

XTOOLS_NAMESPACE_END
