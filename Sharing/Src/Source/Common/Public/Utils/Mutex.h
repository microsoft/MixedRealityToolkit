// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// Mutex.h
// Platform-independent wrapper for mutexes
//////////////////////////////////////////////////////////////////////////

#pragma once

#if defined(XTOOLS_PLATFORM_WINDOWS_DESKTOP) || defined(XTOOLS_PLATFORM_WINRT)
# include <windows.h>

#elif defined(XTOOLS_PLATFORM_OSX)
# include <pthread.h>

#endif

XTOOLS_NAMESPACE_BEGIN

class Mutex
{
public:
	Mutex();
	~Mutex();

	void Lock();
	void Unlock();

private:
#if defined(XTOOLS_PLATFORM_WINDOWS_DESKTOP) || defined(XTOOLS_PLATFORM_WINRT)
	CRITICAL_SECTION m_criticalSection;
    
#elif defined(XTOOLS_PLATFORM_OSX)
    pthread_mutex_t m_mutex;

#else
#error Mutex Unsupported Platform
#endif
};

XTOOLS_NAMESPACE_END
