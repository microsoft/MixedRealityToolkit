//////////////////////////////////////////////////////////////////////////
// ProfileThread.h
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class ProfileThread : public AtomicRefCounted
{
public:
	ProfileThread(uint64 threadID, uint32 reserveSamples);

	uint64				GetThreadID() const;

	int32				GetSampleCount() const;
	ProfileSamplePtr	GetSample(int32 sampleIndex) const;

	void				AddSample(const ProfileSamplePtr& newSample);

private:
	std::vector<ProfileSamplePtr>	m_samples;
	uint64							m_threadID;
};

DECLARE_PTR(ProfileThread)

XTOOLS_NAMESPACE_END
