// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include <pch.h>
#include <Mutex_Z.h>

// A critical section guards access to a shared resource
// private data of the shared resource to keep windows out of our header files
class SharedResource_Z::Private
{
public:
	// the windows critical section object
	CRITICAL_SECTION criticalSection;
};

// create a shared resource
SharedResource_Z::SharedResource_Z() : p(New_Z Private)
{
	InitializeCriticalSectionEx(&p->criticalSection, 0, 0);
}

// destroy a shared resource
SharedResource_Z::~SharedResource_Z()
{
	DeleteCriticalSection(&p->criticalSection);
	Delete_Z p;
}

void SharedResource_Z::Lock()
{
	EnterCriticalSection(&p->criticalSection);
}

void SharedResource_Z::Unlock()
{
	LeaveCriticalSection(&p->criticalSection);
}

Bool SharedResource_Z::TryLock()
{
	return TryEnterCriticalSection(&p->criticalSection);
}

//
//
// The SharedResourceGuard guards a shared resource by 
// preventing simultaneous entry to a guarded region
//
//

// enter the shared area
SharedResourceGuard_Z::SharedResourceGuard_Z(SharedResource_Z &sharedResource) : sharedResource(sharedResource)
{
	sharedResource.Lock();
}

// leave the shared resource area
SharedResourceGuard_Z::~SharedResourceGuard_Z()
{
	sharedResource.Unlock();
}

//
//
typedef union EventHandle_Z {
	HANDLE HDL;
	U64	 eventHandle;
	EventHandle_Z() {}
	EventHandle_Z(HANDLE hdl) : HDL(hdl) {}
	EventHandle_Z(U64 hdl) : eventHandle(hdl) {}
} EventHandle_Z;

//
// Event::Create
//
Bool Event_Z::Create()
{
	EventHandle_Z hdl;
	hdl.HDL = CreateEventEx(NULL, NULL, CREATE_EVENT_MANUAL_RESET, EVENT_ALL_ACCESS); // Create a manual-reset event, initially not signaled
	eventHandle = hdl.eventHandle;
	return eventHandle != NULL ? TRUE : FALSE;
}


//
// Event::Delete
//
void Event_Z::Delete()
{
	ASSERT_Z(eventHandle);
	EventHandle_Z hdl(eventHandle);
	CloseHandle(hdl.HDL);
	eventHandle = NULL;
}


//
// Event::Signal
//
void Event_Z::Signal()
{
	ASSERT_Z(eventHandle);
	EventHandle_Z hdl(eventHandle);
	SetEvent(hdl.HDL);
}

void Event_Z::Reset()
{
	ASSERT_Z(eventHandle);
	EventHandle_Z hdl(eventHandle);
	ResetEvent(hdl.HDL);
}


//
// Event::Wait
//
Bool Event_Z::Wait(S32 time, Bool reset)
{
	ASSERT_Z(eventHandle);
	EventHandle_Z hdl(eventHandle);
	U32 rc = WaitForSingleObjectEx(hdl.HDL, time < 0 ? INFINITE : time, FALSE);
	if( reset )
		ResetEvent(hdl.HDL);
	return rc == WAIT_TIMEOUT ? FALSE : TRUE;
}


//
// Event::Poll
//
Bool Event_Z::Poll()
{
	ASSERT_Z(eventHandle);
	EventHandle_Z hdl(eventHandle);
	U32 rc = WaitForSingleObjectEx(hdl.HDL, 0, FALSE);
	return rc == WAIT_OBJECT_0 ? TRUE : FALSE;
}

  
//
// Event::WaitEvents
//
S32 Event_Z::WaitEvents( const Event_Z* events, const U8 count, const Bool waitAll, const S32 time )
{
	DWORD returnCode;
	HANDLE handles[MAXIMUM_WAIT_OBJECTS];

	EXCEPTION_Z( count > 0 && count < MAXIMUM_WAIT_OBJECTS );

	for( U8 i = 0; i < count; ++i )
	{
		EventHandle_Z hdl(events[i].eventHandle);
		handles[i] = hdl.HDL;
		ASSERT_Z( handles[i] );
	}

	returnCode = WaitForMultipleObjectsEx( count, handles, waitAll, time < 0 ? INFINITE : time, FALSE);

	for( U8 i = 0; i < count; ++i )
		ResetEvent( handles[i] );

	const DWORD maxSuccessValue( WAIT_OBJECT_0 + count );
	if( returnCode >= WAIT_OBJECT_0 && returnCode < maxSuccessValue )
		return waitAll ? THREAD_WAIT_SIGNALED : maxSuccessValue-returnCode;

	const DWORD maxAbandonValue( WAIT_ABANDONED_0 + count );
	if( returnCode >= WAIT_ABANDONED_0 && returnCode < maxAbandonValue )
		return THREAD_WAIT_ABANDONNED;

	if( returnCode == WAIT_TIMEOUT )
		return THREAD_WAIT_TIMEOUT;

	return THREAD_WAIT_FAILED;
}

S32 Event_Z::WaitEvents( Event_Z*const* events, const U8 count, const Bool waitAll, const S32 time )
{
	DWORD returnCode;
	HANDLE handles[MAXIMUM_WAIT_OBJECTS];

	EXCEPTION_Z( count > 0 && count < MAXIMUM_WAIT_OBJECTS );

	for( U8 i = 0; i < count; ++i )
	{
		EventHandle_Z hdl(events[i]->eventHandle);
		handles[i] = hdl.HDL;
		ASSERT_Z( handles[i] );
	}

	returnCode = WaitForMultipleObjectsEx( count, handles, waitAll, time < 0 ? INFINITE : time, FALSE);

	for( U8 i = 0; i < count; ++i )
		ResetEvent( handles[i] );

	const DWORD maxSuccessValue( WAIT_OBJECT_0 + count );
	if( returnCode >= WAIT_OBJECT_0 && returnCode < maxSuccessValue )
		return waitAll ? THREAD_WAIT_SIGNALED : maxSuccessValue-returnCode;

	const DWORD maxAbandonValue( WAIT_ABANDONED_0 + count );
	if( returnCode >= WAIT_ABANDONED_0 && returnCode < maxAbandonValue )
		return THREAD_WAIT_ABANDONNED;

	if( returnCode == WAIT_TIMEOUT )
		return THREAD_WAIT_TIMEOUT;

	return THREAD_WAIT_FAILED;
}
 
//
// Event::PollEvents
//
Bool Event_Z::PollEvents( const Event_Z* events, const U8 count )
{
	HANDLE handles[MAXIMUM_WAIT_OBJECTS];
	EXCEPTION_Z( count > 0 && count < MAXIMUM_WAIT_OBJECTS );

	for( U8 i = 0; i < count; ++i )
	{
		EventHandle_Z hdl(events[i].eventHandle);
		handles[i] = hdl.HDL;
		ASSERT_Z( handles[i] );
	}

	DWORD returnCode = WaitForMultipleObjectsEx( count, handles, TRUE, 0 , FALSE);

	return returnCode >= WAIT_OBJECT_0 && returnCode < ( WAIT_OBJECT_0 + count );
}

Bool Event_Z::PollEvents( Event_Z*const* events, const U8 count )
{
	HANDLE handles[MAXIMUM_WAIT_OBJECTS];
	EXCEPTION_Z( count > 0 && count < MAXIMUM_WAIT_OBJECTS );

	for( U8 i = 0; i < count; ++i )
	{
		EventHandle_Z hdl(events[i]->eventHandle);
		handles[i] = hdl.HDL;
		ASSERT_Z( handles[i] );
	}

	DWORD returnCode = WaitForMultipleObjectsEx( count, handles, TRUE, 0, FALSE );

	return returnCode >= WAIT_OBJECT_0 && returnCode < ( WAIT_OBJECT_0 + count );
}

//
// Semaphore
//

void Semaphore_Z::Create(U32 initialCount, U32 maxCount)
{
	EventHandle_Z hdl;
	hdl.HDL = CreateSemaphoreExW(NULL, initialCount, maxCount, NULL, 0, SEMAPHORE_ALL_ACCESS);
	handle = hdl.eventHandle;
	EXCEPTION_Z(handle);
}

void Semaphore_Z::Destroy()
{
	ASSERT_Z(handle);
	EventHandle_Z hdl(handle);
	CloseHandle(hdl.HDL);
	handle = NULL;
}

Bool Semaphore_Z::Wait(S32 time)
{
	ASSERT_Z(handle);
	EventHandle_Z hdl(handle);
	U32 rc = WaitForSingleObjectEx(hdl.HDL, time < 0 ? INFINITE : time, FALSE);
	return rc == WAIT_TIMEOUT ? FALSE : TRUE;
}

void Semaphore_Z::Release(U32 addValue)
{
	ASSERT_Z(handle);
	EventHandle_Z hdl(handle);
	ReleaseSemaphore(hdl.HDL,addValue,NULL);
}
