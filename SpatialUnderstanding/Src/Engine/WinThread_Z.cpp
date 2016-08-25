// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include <Types_Z.h>	//
#include <Assert_Z.h>	// Ces deux includes sont l� pour Kera.

#include <pch.h>
#include <Thread_Z.h>
#include <windows.h>

#if defined(_WINSTORE) && !defined(_UAP)
	#error "windows store targets should use WinStoreThread_Z.cpp"
#endif

// Code needed to set the debug name for the thread.
// This is weird, but it's in the official doc: http://msdn.microsoft.com/en-us/library/xcb2z8hs.aspx
#ifndef _SUBMISSION
	const DWORD MS_VC_EXCEPTION=0x406D1388;

	#pragma pack(push,8)
	typedef struct tagTHREADNAME_INFO
	{
		DWORD dwType; // Must be 0x1000.
		LPCSTR szName; // Pointer to name (in user addr space).
		DWORD dwThreadID; // Thread ID (-1=caller thread).
		DWORD dwFlags; // Reserved for future use, must be zero.
	} THREADNAME_INFO;
	#pragma pack(pop)

	void Thread_Z::SetDebugName( U32 threadID, const Char* threadName)
	{
		THREADNAME_INFO info;
		info.dwType = 0x1000;
		info.szName = threadName;
		info.dwThreadID = (DWORD)threadID;
		info.dwFlags = 0;

		__try
		{
			RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info );
		}
		__except(EXCEPTION_EXECUTE_HANDLER)
		{
		}
	}
#endif

DWORD __CDECL	PlatformThreadProc(LPVOID lpThreadParameter)
{
#ifndef _SUBMISSION
	Thread_Z::SetDebugName(-1, ((Thread_Z*)lpThreadParameter)->GetName());
#endif

	((Thread_Z*)lpThreadParameter)->ThreadProc();
	ExitThread(0);
	return 0;
}

class PlatformThreadObject_Z
{
public:
	PlatformThreadObject_Z()		{threadHandle=INVALID_HANDLE_VALUE;}
	HANDLE		threadHandle;
	U32			threadId;
};

static inline U16 CountSetBits( ULONG_PTR bitMask )
{
	const DWORD offset = sizeof(ULONG_PTR) * 8 - 1;
	U16 bitSetCount ( 0 );
	ULONG_PTR bitTest = (ULONG_PTR)1 << offset;    
	for( DWORD i = 0; i <= offset; ++i )
	{
		if( bitMask & bitTest )
			++bitSetCount;
		bitTest >>= 1;
	}
	return bitSetCount;
}

S32 Thread_Z::SafeIncrement( volatile S32* addend )
{
	return InterlockedIncrement( addend );
}
S32 Thread_Z::SafeDecrement( volatile S32* addend )
{
	return InterlockedDecrement( addend );
}
S32	Thread_Z::SafeAdd( volatile S32* addend, S32 value )
{
	return InterlockedExchangeAdd(addend,value);
}

U32 Thread_Z::GetThreadId()
{
	return GetCurrentThreadId();
}

U32 Thread_Z::GetProcessorId()
{
	return 0;//GetCurrentProcessorNumber() only available from Vista version;
}
Bool Thread_Z::GetProcessorInfo( ProcInfo_Z& info )
{
	SYSTEM_INFO processorInfo;
	GetNativeSystemInfo( &processorInfo );

	memset(&info, 0, sizeof(info));
	info.logical = processorInfo.dwNumberOfProcessors;
	info.physical = 0;
	info.scalar = 0;

	info.cpuCacheLineSize = 0;
	info.cpuCacheSizeL1 = 0;
	info.cpuCacheSizeL2 = 0;
	info.cpuCacheSizeL3 = 0;

	return TRUE;
}

void Thread_Z::Init(const ThreadParam_Z& _ThParam)
{
	EXCEPTION_Z(!IsInited());
	m_ThreadObject = New_Z PlatformThreadObject_Z();
	m_ThreadParam = _ThParam;
	m_Flags       = _ThParam.Flags;
	RegisterRunningThread( this );
}

S32 GetWinThreadPriority(S32 Priority)
{
	switch(Priority){
	case THREAD_PRIO_VERYLOW:	return THREAD_PRIORITY_LOWEST;
	case THREAD_PRIO_LOW:		return THREAD_PRIORITY_BELOW_NORMAL;
	case THREAD_PRIO_HIGH:		return THREAD_PRIORITY_ABOVE_NORMAL;
	case THREAD_PRIO_VERYHIGH:	return THREAD_PRIORITY_HIGHEST;
	case THREAD_PRIO_NORMAL:
	default:					return THREAD_PRIORITY_NORMAL;
	}
}

Bool Thread_Z::Start()
{
	EXCEPTIONC_Z(m_ThreadObject != 0,"Call Init Before");
	EXCEPTIONC_Z(m_ThreadObject->threadHandle==INVALID_HANDLE_VALUE,"Already Started");
	
	const S32 MaxRetry(3);
	S32 Retry(MaxRetry);

	while(Retry--)
	{
		m_ThreadObject->threadHandle = CreateThread(NULL,
			m_ThreadParam.StackSize,
			(LPTHREAD_START_ROUTINE)&PlatformThreadProc,
			this,
			0,
			&m_ThreadObject->threadId);

		if(!m_ThreadObject->threadHandle)
		{
			DWORD	dw=::GetLastError();
			String_Z<1024>	ErrorStr;

#ifdef _UNICODE
			WCHAR wstr[ErrorStr.ArraySize];
			wstr[ErrorStr.ArraySize - 1]=0;
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), wstr, ErrorStr.ArraySize - 1, NULL);

			CHAR str[ErrorStr.ArraySize];
			size_t strNumChars;
			wcstombs_s(&strNumChars, str, ErrorStr.ArraySize, wstr, ErrorStr.ArraySize - 1);
			str[ErrorStr.ArraySize - 1]=0;
			ErrorStr.StrCpy(str);
#else
			ErrorStr[ErrorStr.ArraySize - 1]=0;
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)ErrorStr.Get(), ErrorStr.ArraySize - 1, NULL);
#endif

			OUTPUT_Z("Thread_Z::Start failed name=%s\n\tAttempt %d/%d\n\tLastError=%s",m_ThreadParam.Name.Get(),MaxRetry-Retry,MaxRetry,(const Char *)ErrorStr);
			Sleep(500);
		}
		else
			break;
	}
	EXCEPTIONC_Z( m_ThreadObject->threadHandle != 0, "Failed to start thread %s 0x%x", m_ThreadParam.Name.Get(), GetLastError() );

	// Concurrency visualizer does not show thread names but only IDs, use this as a help
	OUTPUT_Z("[Thread_Z] ID#%d '%s'",m_ThreadObject->threadId,m_ThreadParam.Name.Get());

	ChangePriority(m_ThreadParam.Priority);
	return TRUE;
}

ThreadWait_Z Thread_Z::Join(S64 uTimeOut) /* Micro-seconds */
{
	DWORD	Ret=WAIT_OBJECT_0;
	EXCEPTIONC_Z(m_ThreadObject != 0,"Trying to join an uninitialized thread! Thread_Z::Init");
	EXCEPTIONC_Z(m_ThreadObject->threadHandle!=INVALID_HANDLE_VALUE,"Trying to join an non started thread! Thread_Z::Start");
	if(m_ThreadObject && m_ThreadObject->threadHandle!=INVALID_HANDLE_VALUE)
		if( uTimeOut == -1 )
		{
			Ret=WaitForSingleObject(m_ThreadObject->threadHandle,INFINITE);
		}
		else
		{
			Ret=WaitForSingleObject(m_ThreadObject->threadHandle,(DWORD)(uTimeOut/1000));
		}

	switch(Ret)
	{
		case WAIT_FAILED: 		return THREAD_WAIT_FAILED;
		case WAIT_ABANDONED:	return THREAD_WAIT_ABANDONNED;
		case WAIT_TIMEOUT:		return THREAD_WAIT_TIMEOUT;
		default:
		{
			if(m_ThreadObject && m_ThreadObject->threadHandle!=INVALID_HANDLE_VALUE)
			{
				CloseHandle(m_ThreadObject->threadHandle);
				m_ThreadObject->threadHandle=INVALID_HANDLE_VALUE;
			}
			return THREAD_WAIT_SIGNALED;
		}
	}
}

void Thread_Z::Suspend()
{
	if(m_ThreadObject && m_ThreadObject->threadHandle!=INVALID_HANDLE_VALUE)
		SuspendThread(m_ThreadObject->threadHandle);
}

void Thread_Z::Resume()
{
	if(m_ThreadObject && m_ThreadObject->threadHandle!=INVALID_HANDLE_VALUE)
		ResumeThread(m_ThreadObject->threadHandle);
}

void Thread_Z::ChangePriority(S32 Priority)
{
	m_ThreadParam.Priority = Priority;
	if(m_ThreadObject && m_ThreadObject->threadHandle!=INVALID_HANDLE_VALUE)
	{
		int prio = GetWinThreadPriority(m_ThreadParam.Priority);
		SetThreadPriority(m_ThreadObject->threadHandle,prio);
	}
}

void Thread_Z::Shut()
{
	if(m_ThreadObject)
	{
		if( m_ThreadObject->threadHandle!=INVALID_HANDLE_VALUE )
			Join();
		Delete_Z m_ThreadObject;
		m_ThreadObject = NULL;
	}

	UnregisterRunningThread( this );
}

Thread_Z::~Thread_Z()
{
	Shut();
}

void Thread_Z::Sleep(U64 uTime) /* Micro-seconds */
{
	::Sleep((DWORD)(uTime/1000));	// ! les :: sont indispensables ici, sinon il rappelle la methode de Thread_Z
}
