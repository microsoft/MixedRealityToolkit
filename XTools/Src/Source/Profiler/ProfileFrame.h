//////////////////////////////////////////////////////////////////////////
// ProfileFrame.h
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class ProfileFrame : public AtomicRefCounted
{
public:
	ProfileFrame();

	int32				GetThreadCount() const;

	ProfileThreadPtr	GetThread(int32 threadIndex) const;

	int32				GetLogMessageCount() const;
	LogMessagePtr		GetLogMessage(int32 messageIndex) const;

	void				AddThread(const ProfileThreadPtr& newThread);
	void				AddMessage(const LogMessagePtr& newMessage);

private:
	std::vector<ProfileThreadPtr>	m_threads;
	std::vector<LogMessagePtr>		m_messages;
};

DECLARE_PTR(ProfileFrame)

XTOOLS_NAMESPACE_END
