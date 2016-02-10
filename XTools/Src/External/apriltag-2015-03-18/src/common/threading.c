/// MICROSOFT PROJECT B CHANGES BEGIN
#include "threading.h"

#ifdef _MSC_VER
void pthread_mutex_init(pthread_mutex_t *mutex, const void* attr)
{
	*mutex = CreateMutexEx(
		NULL,				// default security attributes
		NULL,				// No need to specify a name
		0,					// Flags
		STANDARD_RIGHTS_ALL);				// Default security descriptor
}

void pthread_mutex_destroy(pthread_mutex_t *mutex)
{
	CloseHandle(*mutex);
}

void pthread_mutex_lock(pthread_mutex_t *mutex)
{
	DWORD result = WaitForSingleObjectEx(
		*mutex,			// handle to mutex
		INFINITE,		// no time-out interval
		FALSE);			// Do not return on IO alerts
}

void pthread_mutex_unlock(pthread_mutex_t *mutex)
{
	ReleaseMutex(*mutex);
}

void sched_yield()
{
#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
	SwitchToThread();
#endif
}

void pthread_cond_init(pthread_cond_t *cv, const void * attr)
{
	cv->waiters_count_ = 0;
	cv->was_broadcast_ = 0;
	cv->sema_ = CreateSemaphoreEx(NULL,       // no security
		0,          // initially 0
		0x7fffffff, // max count
		NULL,		// unnamed 
		0,
		EVENT_ALL_ACCESS);
	InitializeCriticalSectionEx(&cv->waiters_count_lock_, 10, 0);
	cv->waiters_done_ = CreateEventEx(NULL,  // no security
		NULL, // unnamed
		0, // auto-reset, unsignalled
		EVENT_ALL_ACCESS);
		
}

void pthread_cond_destroy(pthread_cond_t *cond)
{
	DeleteCriticalSection(&cond->waiters_count_lock_);
	CloseHandle(cond->sema_);
	CloseHandle(cond->waiters_done_);
}

void pthread_cond_wait(pthread_cond_t *cv,
	pthread_mutex_t *external_mutex)
{
	// TODO: multithreading for apriltag
	//// Avoid race conditions.
	//EnterCriticalSection(&cv->waiters_count_lock_);
	//cv->waiters_count_++;
	//LeaveCriticalSection(&cv->waiters_count_lock_);

	//// This call atomically releases the mutex and waits on the
	//// semaphore until <pthread_cond_signal> or <pthread_cond_broadcast>
	//// are called by another thread.
	//SignalObjectAndWait(*external_mutex, cv->sema_, INFINITE, FALSE);

	//// Reacquire lock to avoid race conditions.
	//EnterCriticalSection(&cv->waiters_count_lock_);

	//// We're no longer waiting...
	//cv->waiters_count_--;

	//// Check to see if we're the last waiter after <pthread_cond_broadcast>.
	//int last_waiter = cv->was_broadcast_ && cv->waiters_count_ == 0;

	//LeaveCriticalSection(&cv->waiters_count_lock_);

	//// If we're the last waiter thread during this particular broadcast
	//// then let all the other threads proceed.
	//if (last_waiter)
	//	// This call atomically signals the <waiters_done_> event and waits until
	//	// it can acquire the <external_mutex>.  This is required to ensure fairness. 
	//	SignalObjectAndWait(cv->waiters_done_, *external_mutex, INFINITE, FALSE);
	//else
	//	// Always regain the external mutex since that's the guarantee we
	//	// give to our callers. 
	//	WaitForSingleObjectEx(*external_mutex, INFINITE, FALSE);
}

void pthread_cond_broadcast(pthread_cond_t *cv)
{
	// TODO: multithreading for apriltag
	//// This is needed to ensure that <waiters_count_> and <was_broadcast_> are
	//// consistent relative to each other.
	//EnterCriticalSection(&cv->waiters_count_lock_);
	//int have_waiters = 0;

	//if (cv->waiters_count_ > 0) {
	//	// We are broadcasting, even if there is just one waiter...
	//	// Record that we are broadcasting, which helps optimize
	//	// <pthread_cond_wait> for the non-broadcast case.
	//	cv->was_broadcast_ = 1;
	//	have_waiters = 1;
	//}

	//if (have_waiters) {
	//	// Wake up all the waiters atomically.
	//	ReleaseSemaphore(cv->sema_, cv->waiters_count_, 0);

	//	LeaveCriticalSection(&cv->waiters_count_lock_);

	//	// Wait for all the awakened threads to acquire the counting
	//	// semaphore. 
	//	WaitForSingleObject(cv->waiters_done_, INFINITE);
	//	// This assignment is okay, even without the <waiters_count_lock_> held 
	//	// because no other waiter threads can wake up to access it.
	//	cv->was_broadcast_ = 0;
	//}
	//else
	//	LeaveCriticalSection(&cv->waiters_count_lock_);
}

int pthread_create(pthread_t* thread, const void* attr, PTHREAD_START_ROUTINE start_routine, void* arg)
{
	*thread = CreateThread(NULL, 0, start_routine, arg, 0, NULL);
	return *thread == NULL;
}

void pthread_join(pthread_t thread, void **value_ptr)
{
	WaitForSingleObjectEx(thread, INFINITE, FALSE);
	CloseHandle(thread);
}
#endif
/// MICROSOFT PROJECT B CHANGES END