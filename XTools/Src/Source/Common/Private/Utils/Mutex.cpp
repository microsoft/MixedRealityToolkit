//////////////////////////////////////////////////////////////////////////
// Mutex.cpp
//
// Platform-independent wrapper for mutexes
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Public/Utils/Mutex.h>


XTOOLS_NAMESPACE_BEGIN

Mutex::Mutex()
{
#if defined(XTOOLS_PLATFORM_WINDOWS_DESKTOP) || defined(XTOOLS_PLATFORM_WINRT)
	m_mutexHandle = CreateMutexEx(
		NULL,				// default security attributes
		NULL,				// No need to specify a name
		0,					// Flags
		STANDARD_RIGHTS_ALL);				// Default security descriptor
	XTASSERT(GetLastError() != ERROR_ALREADY_EXISTS);
	XTASSERT(m_mutexHandle != NULL);
    
#elif defined(XTOOLS_PLATFORM_OSX)
    XTVERIFY(pthread_mutex_init(&m_mutex, NULL) == 0);
    
#else
#error Mutex Unsupported Platform
#endif
}


Mutex::~Mutex()
{
#if defined(XTOOLS_PLATFORM_WINDOWS_DESKTOP) || defined(XTOOLS_PLATFORM_WINRT)
	XTASSERT(m_mutexHandle != NULL);
	if (!CloseHandle(m_mutexHandle))
	{
		DWORD errorCode = GetLastError();
		LogError("Mutex CloseHandle failed, error code %u", errorCode);
		XTASSERT(false);
	}

	m_mutexHandle = NULL;
    
#elif defined(XTOOLS_PLATFORM_OSX)
    XTVERIFY(pthread_mutex_destroy(&m_mutex) == 0);
    
#else
#error Mutex Unsupported Platform
#endif
}


void Mutex::Lock()
{
#if defined(XTOOLS_PLATFORM_WINDOWS_DESKTOP) || defined(XTOOLS_PLATFORM_WINRT)
	XTASSERT(m_mutexHandle != NULL);
	DWORD result = WaitForSingleObjectEx(
		m_mutexHandle,  // handle to mutex
		INFINITE,		// no time-out interval
		false);			// Do not return on IO alerts
	XTASSERT(result == WAIT_OBJECT_0);
    
#elif defined(XTOOLS_PLATFORM_OSX)
    XTVERIFY(pthread_mutex_lock(&m_mutex) == 0);
    
#else
#error Mutex Unsupported Platform
#endif
}


void Mutex::Unlock()
{
#if defined(XTOOLS_PLATFORM_WINDOWS_DESKTOP) || defined(XTOOLS_PLATFORM_WINRT)
	XTASSERT(m_mutexHandle != NULL);
	if (!ReleaseMutex(m_mutexHandle))
	{
		DWORD errorCode = GetLastError();
		LogError("ReleaseMutex failed, error code %u", errorCode);
		XTASSERT(false);
	}
    
#elif defined(XTOOLS_PLATFORM_OSX)
    XTVERIFY(pthread_mutex_unlock(&m_mutex) == 0);
    
#else
#error Mutex Unsupported Platform
#endif
}

XTOOLS_NAMESPACE_END
