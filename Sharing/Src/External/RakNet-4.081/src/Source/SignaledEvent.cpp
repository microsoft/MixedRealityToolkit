/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant 
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include "SignaledEvent.h"
#include "RakAssert.h"
#include "RakSleep.h"

#if defined(__GNUC__) 
#include <sys/time.h>
#include <unistd.h>
#endif

using namespace RakNet;





SignaledEvent::SignaledEvent()
{
	/// MICROSOFT PROJECT B CHANGES BEGIN
#if defined(_WIN32) || defined(WINDOWS_PHONE_8) ||  defined(WINDOWS_STORE_RT)
	
	eventList=NULL;
	/// MICROSOFT PROJECT B CHANGES END


#else
	isSignaled=false;
#endif
}
SignaledEvent::~SignaledEvent()
{
	// Intentionally do not close event, so it doesn't close twice on linux
}

void SignaledEvent::InitEvent(void)
{
	/// MICROSOFT PROJECT B CHANGES BEGIN
#if defined(_WIN32) || defined(WINDOWS_PHONE_8) || defined(WINDOWS_STORE_RT)
	
	eventList = CreateEventEx(NULL, NULL, 0, STANDARD_RIGHTS_ALL | EVENT_MODIFY_STATE);
	RakAssert(eventList != NULL);
	/// MICROSOFT PROJECT B CHANGES END







#else

#if !defined(ANDROID)
		pthread_condattr_init( &condAttr );
		pthread_cond_init(&eventList, &condAttr);
#else
		pthread_cond_init(&eventList, 0);
#endif
		pthread_mutexattr_init( &mutexAttr	);
		pthread_mutex_init(&hMutex, &mutexAttr);
#endif
}

void SignaledEvent::CloseEvent(void)
{
	/// MICROSOFT PROJECT B CHANGES BEGIN
#if defined(_WIN32) || defined(WINDOWS_PHONE_8) ||  defined(WINDOWS_STORE_RT)
	
	if (eventList!=NULL)
	{
		CloseHandle(eventList);
		eventList=NULL;
	}

	/// MICROSOFT PROJECT B CHANGES END







#else
	pthread_cond_destroy(&eventList);
	pthread_mutex_destroy(&hMutex);
#if !defined(ANDROID)
	pthread_condattr_destroy( &condAttr );
#endif
	pthread_mutexattr_destroy( &mutexAttr );
#endif
}

void SignaledEvent::SetEvent(void)
{
	/// MICROSOFT PROJECT B CHANGES BEGIN
#if defined(_WIN32) || defined(WINDOWS_PHONE_8) ||  defined(WINDOWS_STORE_RT)
	if (eventList != NULL)
	{
		::SetEvent(eventList);
	}
	/// MICROSOFT PROJECT B CHANGES END










#else
	// Different from SetEvent which stays signaled.
	// We have to record manually that the event was signaled
	isSignaledMutex.Lock();
	isSignaled=true;
	isSignaledMutex.Unlock();

	// Unblock waiting threads
	pthread_cond_broadcast(&eventList);
#endif
}


void SignaledEvent::WaitOnEvent(int timeoutMs)
{
	/// MICROSOFT PROJECT B CHANGES BEGIN
#if defined(_WIN32) ||  defined(WINDOWS_STORE_RT)
	
//	WaitForMultipleObjects(
//		2,
//		eventList,
//		false,
//		timeoutMs);
	DWORD result = WaitForSingleObjectEx(eventList, timeoutMs, FALSE);
	RakAssert(result != WAIT_FAILED);

	// NOTE: ResetEvent() not required because the event is set to auto-reset

	/// MICROSOFT PROJECT B CHANGES END




































#else

	// If was previously set signaled, just unset and return
	isSignaledMutex.Lock();
	if (isSignaled==true)
	{
		isSignaled=false;
		isSignaledMutex.Unlock();
		return;
	}
	isSignaledMutex.Unlock();

	

	//struct timespec   ts;

	// Else wait for SetEvent to be called

















		struct timespec   ts;

		int rc;
		struct timeval    tp;
		rc =  gettimeofday(&tp, NULL);
		ts.tv_sec  = tp.tv_sec;
		ts.tv_nsec = tp.tv_usec * 1000;
// #endif

		while (timeoutMs > 30)
		{
			// Wait 30 milliseconds for the signal, then check again.
			// This is in case we  missed the signal between the top of this function and pthread_cond_timedwait, or after the end of the loop and pthread_cond_timedwait
			ts.tv_nsec += 30*1000000;
			if (ts.tv_nsec >= 1000000000)
			{
			        ts.tv_nsec -= 1000000000;
			        ts.tv_sec++;
			}
			
			// [SBC] added mutex lock/unlock around cond_timedwait.
            // this prevents airplay from generating a whole much of errors.
            // not sure how this works on other platforms since according to
            // the docs you are suppost to hold the lock before you wait
            // on the cond.
            pthread_mutex_lock(&hMutex);
			pthread_cond_timedwait(&eventList, &hMutex, &ts);
            pthread_mutex_unlock(&hMutex);

			timeoutMs-=30;

			isSignaledMutex.Lock();
			if (isSignaled==true)
			{
				isSignaled=false;
				isSignaledMutex.Unlock();
				return;
			}
			isSignaledMutex.Unlock();
		}

		// Wait the remaining time, and turn off the signal in case it was set
		ts.tv_nsec += timeoutMs*1000000;
		if (ts.tv_nsec >= 1000000000)
		{
		        ts.tv_nsec -= 1000000000;
		        ts.tv_sec++;
		}

		pthread_mutex_lock(&hMutex);
		pthread_cond_timedwait(&eventList, &hMutex, &ts);
        pthread_mutex_unlock(&hMutex);

		isSignaledMutex.Lock();
		isSignaled=false;
		isSignaledMutex.Unlock();

#endif
}
