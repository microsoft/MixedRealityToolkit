//////////////////////////////////////////////////////////////////////////
// XThread.h
//
// Cross-platform wrapper for basic thread creation and usage.
// Called XThread rather than Thread to avoid conflicting with platform
// provided headers
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

typedef void (*XThreadFunc)(void*);

class XThread
{
public:
	XThread();
	~XThread();

	bool Fork(XThreadFunc entrypoint, void* parameter);
	void Join();
	bool IsForked() const;

private:

	// Proxy type for holding the real function and parameter.  Gets passed
	// to the platform-specific entry function then called.  
	struct ThreadParam
	{
		XThreadFunc m_function;
		void*		m_parameter;
	};

	ThreadParam		m_param;
	volatile int	m_isForked;
	Mutex			m_mutex;

#if defined(XTOOLS_PLATFORM_WINDOWS_ANY)
	
	static DWORD WINAPI ThreadProc(LPVOID context);

	HANDLE			m_threadHandle;

#elif defined(XTOOLS_PLATFORM_OSX)
	static void* ThreadProc(void* context);
	
	pthread_t       m_thread;
	
#else
# error Unsupported Platform
#endif
};

XTOOLS_NAMESPACE_END