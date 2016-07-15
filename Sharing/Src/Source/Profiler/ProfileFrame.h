// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// ProfileFrame.h
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
