//////////////////////////////////////////////////////////////////////////
// ProfileThread.cpp
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ProfileThread.h"

XTOOLS_NAMESPACE_BEGIN

ProfileThread::ProfileThread(uint64 threadID, uint32 reserveSamples)
: m_threadID(threadID)
{
	m_samples.reserve(reserveSamples);
}


uint64 ProfileThread::GetThreadID() const
{
	return m_threadID;
}


int32 ProfileThread::GetSampleCount() const
{
	return static_cast<int32>(m_samples.size());
}


ProfileSamplePtr ProfileThread::GetSample(int32 sampleIndex) const
{
	if (sampleIndex < GetSampleCount())
	{
		return m_samples[sampleIndex];
	}
	else
	{
		return nullptr;
	}
}


void ProfileThread::AddSample(const ProfileSamplePtr& newSample)
{
	m_samples.push_back(newSample);
}

XTOOLS_NAMESPACE_END
