//////////////////////////////////////////////////////////////////////////
// Event.h
//
// Platform-independent wrapper for wait handles
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#if defined(XTOOLS_PLATFORM_WINDOWS_DESKTOP) || defined(XTOOLS_PLATFORM_WINRT)
# include <windows.h>

#elif defined(XTOOLS_PLATFORM_OSX)
# include <pthread.h>

#endif

XTOOLS_NAMESPACE_BEGIN

class Event
{
public:
	Event(bool bManualReset, bool bIsSet);
	~Event();

	// Multiple calls to Set() on an event that is already set has no effect
	void Set();

	void Reset();

	// Wait indefinitely for the event to be signaled
	bool Wait();

	// Wait for the event to be signaled or the given amount of time to pass,
	// whichever comes first
	bool WaitTimeout(uint32 timeoutMS);

private:
#if defined(XTOOLS_PLATFORM_WINDOWS_DESKTOP) || defined(XTOOLS_PLATFORM_WINRT)
	HANDLE m_EventHandle;

#elif defined(XTOOLS_PLATFORM_OSX)
    pthread_mutex_t m_mutex;
    pthread_cond_t m_cond;
    bool m_triggered;
#else
# error Event unsupported on this platform
#endif
};

XTOOLS_NAMESPACE_END