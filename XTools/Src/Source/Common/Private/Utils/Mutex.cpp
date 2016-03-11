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
	// Note: choosing a loop count of 4000 before doing an expensive WaitForSingleObject.  
	// From MSDN: "The heap manager uses a spin count of roughly 4000 for its per-heap critical sections. This gives great performance and scalability in almost all worst-case scenarios."
	if (!InitializeCriticalSectionEx(&m_criticalSection, 4000, CRITICAL_SECTION_NO_DEBUG_INFO))
	{
		DWORD errorCode = GetLastError();
		LogError("InitializeCriticalSectionEx failed, error code %u", errorCode);
		XTASSERT(false);
	}
    
#elif defined(XTOOLS_PLATFORM_OSX)
    XTVERIFY(pthread_mutex_init(&m_mutex, NULL) == 0);
    
#else
#error Mutex Unsupported Platform
#endif
}


Mutex::~Mutex()
{
#if defined(XTOOLS_PLATFORM_WINDOWS_DESKTOP) || defined(XTOOLS_PLATFORM_WINRT)
	DeleteCriticalSection(&m_criticalSection);
    
#elif defined(XTOOLS_PLATFORM_OSX)
    XTVERIFY(pthread_mutex_destroy(&m_mutex) == 0);
    
#else
#error Mutex Unsupported Platform
#endif
}


void Mutex::Lock()
{
#if defined(XTOOLS_PLATFORM_WINDOWS_DESKTOP) || defined(XTOOLS_PLATFORM_WINRT)
	EnterCriticalSection(&m_criticalSection);
    
#elif defined(XTOOLS_PLATFORM_OSX)
    XTVERIFY(pthread_mutex_lock(&m_mutex) == 0);
    
#else
#error Mutex Unsupported Platform
#endif
}


void Mutex::Unlock()
{
#if defined(XTOOLS_PLATFORM_WINDOWS_DESKTOP) || defined(XTOOLS_PLATFORM_WINRT)
	LeaveCriticalSection(&m_criticalSection);
    
#elif defined(XTOOLS_PLATFORM_OSX)
    XTVERIFY(pthread_mutex_unlock(&m_mutex) == 0);
    
#else
#error Mutex Unsupported Platform
#endif
}

XTOOLS_NAMESPACE_END
