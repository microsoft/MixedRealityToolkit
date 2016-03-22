/// MICROSOFT PROJECT B CHANGES BEGIN

#pragma once

#ifdef _MSC_VER
#include <Windows.h>

/// MICROSOFT PROJECT B CHANGES END

typedef HANDLE pthread_t;
typedef HANDLE pthread_mutex_t;
typedef struct
{
	int waiters_count_;
	// Number of waiting threads.

	CRITICAL_SECTION waiters_count_lock_;
	// Serialize access to <waiters_count_>.

	HANDLE sema_;
	// Semaphore used to queue up threads waiting for the condition to
	// become signaled. 

	HANDLE waiters_done_;
	// An auto-reset event used by the broadcast/signal thread to wait
	// for all the waiting thread(s) to wake up and be released from the
	// semaphore. 

	size_t was_broadcast_;
	// Keeps track of whether we were broadcasting or signaling.  This
	// allows us to optimize the code if we're just signaling.
} pthread_cond_t;

void pthread_mutex_init(pthread_mutex_t *mutex, const void* attr);
void pthread_mutex_destroy(pthread_mutex_t *mutex);
void pthread_mutex_lock(pthread_mutex_t *mutex);
void pthread_mutex_unlock(pthread_mutex_t *mutex);
void sched_yield();
void pthread_cond_init(pthread_cond_t *cv, const void * attr);
void pthread_cond_destroy(pthread_cond_t *cond);
void pthread_cond_wait(pthread_cond_t *cv, pthread_mutex_t *external_mutex);
void pthread_cond_broadcast(pthread_cond_t *cv);
int pthread_create(pthread_t* thread, const void* attr, PTHREAD_START_ROUTINE start_routine, void* arg);
void pthread_join(pthread_t thread, void **value_ptr);
#else
#include <pthread.h>
#include <sched.h>
#endif

/// MICROSOFT PROJECT B CHANGES END