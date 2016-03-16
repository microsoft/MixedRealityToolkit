//////////////////////////////////////////////////////////////////////////
// Event.h
//
// Platform-independent wrapper for wait handles
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Public/Utils/Event.h>


XTOOLS_NAMESPACE_BEGIN

Event::Event(bool bManualReset, bool bIsSet)
{
#if defined(XTOOLS_PLATFORM_WINDOWS_DESKTOP) || defined(XTOOLS_PLATFORM_WINRT)
	m_EventHandle = CreateEventEx(
		NULL,												// default security attributes
		NULL,												// No need to specify a name
		(bManualReset ? CREATE_EVENT_MANUAL_RESET : 0) |
		(bIsSet ? CREATE_EVENT_INITIAL_SET : 0),			// Flags
		STANDARD_RIGHTS_ALL | EVENT_MODIFY_STATE );			// Default security descriptor
	XTASSERT(GetLastError() != ERROR_ALREADY_EXISTS);
	XTASSERT(m_EventHandle != NULL);

#elif defined(XTOOLS_PLATFORM_OSX)
    pthread_mutex_init(&m_mutex, 0);
    pthread_cond_init(&m_cond, 0);
    m_triggered = bIsSet;

#else
#error Event Unsupported Platform
#endif
}


Event::~Event()
{
#if defined(XTOOLS_PLATFORM_WINDOWS_DESKTOP) || defined(XTOOLS_PLATFORM_WINRT)
	XTASSERT(m_EventHandle != NULL);
	if ( !CloseHandle(m_EventHandle) )
	{
		DWORD errorCode = GetLastError();
		LogError("Event CloseHandle failed, error code %u", errorCode);
		XTASSERT(false);
	}

	m_EventHandle = NULL;

#elif defined(XTOOLS_PLATFORM_OSX)
    pthread_mutex_destroy(&m_mutex);
    pthread_cond_destroy(&m_cond);

#else
#error Event Unsupported Platform
#endif
}


void Event::Set()
{
#if defined(XTOOLS_PLATFORM_WINDOWS_DESKTOP) || defined(XTOOLS_PLATFORM_WINRT)
	XTASSERT(m_EventHandle != NULL);
	if ( !SetEvent(m_EventHandle) )
	{
		DWORD errorCode = GetLastError();
		LogError("SetEvent failed, error code %u", errorCode);
		XTASSERT(false);
	}

#elif defined(XTOOLS_PLATFORM_OSX)
    pthread_mutex_lock(&m_mutex);
    m_triggered = true;
    pthread_cond_broadcast(&m_cond);
    pthread_mutex_unlock(&m_mutex);

#else
#error Event Unsupported Platform
#endif
}


void Event::Reset()
{
#if defined(XTOOLS_PLATFORM_WINDOWS_DESKTOP) || defined(XTOOLS_PLATFORM_WINRT)
	XTASSERT(m_EventHandle != NULL);
	if ( !ResetEvent(m_EventHandle) )
	{
		DWORD errorCode = GetLastError();
		LogError("ResetEvent failed, error code %u", errorCode);
		XTASSERT(false);
	}

#elif defined(XTOOLS_PLATFORM_OSX)
    pthread_mutex_lock(&m_mutex);
    m_triggered = false;
    pthread_mutex_unlock(&m_mutex);

#else
#error Event Unsupported Platform
#endif
}

bool Event::Wait()
{
#if defined(XTOOLS_PLATFORM_WINDOWS_DESKTOP) || defined(XTOOLS_PLATFORM_WINRT)
	XTASSERT(m_EventHandle != NULL);
	DWORD result = WaitForSingleObjectEx(
		m_EventHandle,  // handle to Event
		INFINITE,		// no time-out interval
		false);			// Do not return on IO alerts
    return result == WAIT_OBJECT_0;

#elif defined(XTOOLS_PLATFORM_OSX)
	pthread_mutex_lock(&m_mutex);
	while(!m_triggered)
	{
		pthread_cond_wait(&m_cond, &m_mutex);
	}
	pthread_mutex_unlock(&m_mutex);
	return true;

#else
#error Event Unsupported Platform
#endif
}


bool Event::WaitTimeout(uint32 timeoutMS)
{
#if defined(XTOOLS_PLATFORM_WINDOWS_DESKTOP) || defined(XTOOLS_PLATFORM_WINRT)
	XTASSERT(m_EventHandle != NULL);
	DWORD result = WaitForSingleObjectEx(
		m_EventHandle,  // handle to Event
		timeoutMS,		// time-out interval
		false);			// Do not return on IO alerts
	return result == WAIT_OBJECT_0;

#elif defined(XTOOLS_PLATFORM_OSX)
	struct timeval tv;
	struct timespec ts;

	gettimeofday(&tv, NULL);
	ts.tv_sec = time(NULL) + timeoutMS / 1000;
	ts.tv_nsec = tv.tv_usec * 1000 + 1000 * 1000 * (timeoutMS % 1000);
	ts.tv_sec += ts.tv_nsec / (1000000000);
	ts.tv_nsec %= (1000000000);

	pthread_mutex_lock(&m_mutex);
	int rc = 0;
	while (!m_triggered && rc == 0)
	{
		rc = pthread_cond_timedwait(&m_cond, &m_mutex, &ts);
	}
	pthread_mutex_unlock(&m_mutex);

	return (rc == 0);

#else
#error Event Unsupported Platform
#endif
}

XTOOLS_NAMESPACE_END
