//////////////////////////////////////////////////////////////////////////
// XThread.cpp
//
// Cross-platform wrapper for basic thread creation and usage.
// Called XThread rather than Thread to avoid conflicting with platform
// provided headers
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Public/Utils/XThread.h>

XTOOLS_NAMESPACE_BEGIN

XThread::XThread()
: m_isForked(0)
{
#if defined(XTOOLS_PLATFORM_WINDOWS_ANY)
	m_threadHandle = NULL;
	
#elif defined(XTOOLS_PLATFORM_OSX)
	m_thread = NULL;
	
#endif
}


XThread::~XThread()
{
	if (m_isForked)
	{
		Join();
	}
}

bool XThread::Fork(XThreadFunc entrypoint, void* parameter)
{
	ScopedLock lock(m_mutex);

	// Can't fork another thread if one is already running
	if (m_isForked)
	{
		return false;
	}

	// Set the values for the parameter that will be passed to the proxy function
	m_param.m_function = entrypoint;
	m_param.m_parameter = parameter;

	// Create and start the thread
#if defined(XTOOLS_PLATFORM_WINDOWS_ANY)
	m_threadHandle = ::CreateThread(NULL, 0, XThread::ThreadProc, &m_param, 0, NULL);
	if ( XTVERIFY(m_threadHandle != NULL) )
	{
		m_isForked = 1;
	}
	
#elif defined(XTOOLS_PLATFORM_OSX)
	pthread_attr_t attr;
	
	// Initialize and set thread detached attribute
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);
	
	XTVERIFY(pthread_create(&m_thread, &attr, XThread::ThreadProc, &m_param) == 0);
	
	pthread_attr_destroy(&attr);
	
	m_isForked = 1;

#endif

	return (m_isForked != 0);
}


void XThread::Join()
{
	ScopedLock lock(m_mutex);

	if (m_isForked)
	{
#if defined(XTOOLS_PLATFORM_WINDOWS_ANY)

		// Make sure that the handle is valid
		XTVERIFY(m_threadHandle);
		
		// Block and wait for the thread to exit
		XTVERIFY(WaitForSingleObjectEx(m_threadHandle, INFINITE, false) == WAIT_OBJECT_0);

		// Close the thread handle
		XTVERIFY(CloseHandle(m_threadHandle));
		m_threadHandle = NULL;
		
#elif defined(XTOOLS_PLATFORM_OSX)
		XTVERIFY(pthread_join(m_thread, nullptr) == 0);
		m_thread = NULL;
		
#endif

		m_isForked = 0;
	}
}


bool XThread::IsForked() const
{
	return (m_isForked != 0);
}


#if defined(XTOOLS_PLATFORM_WINDOWS_ANY)
//static 
DWORD WINAPI XThread::ThreadProc(LPVOID context)
{
	ThreadParam* threadParam = (ThreadParam*)context;

	// Run the user-provided function with the user-provided parameter
	threadParam->m_function(threadParam->m_parameter);

	return 0;
}

#elif defined(XTOOLS_PLATFORM_OSX)

void* XThread::ThreadProc(void* context)
{
	ThreadParam* threadParam = (ThreadParam*)context;
	
	threadParam->m_function(threadParam->m_parameter);
	
	return nullptr;
}

#endif

XTOOLS_NAMESPACE_END
