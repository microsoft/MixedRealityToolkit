// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef _THREAD_Z_H
#define _THREAD_Z_H

#define THREAD_BASE_STACK		65536
#define THREAD_BASE_PRIORITY	THREAD_PRIO_NORMAL

#define Sleep_Z(x)				Thread_Z::Sleep((x))

#include <BeginDef_Z.h>
#include <Assert_Z.h>
#include <String_Z.h>

enum ThreadPriority_Z
{
	THREAD_PRIO_VERYLOW = 0,
	THREAD_PRIO_LOW,
	THREAD_PRIO_NORMAL,
	THREAD_PRIO_HIGH,
	THREAD_PRIO_VERYHIGH
};

enum ThreadCreationFlag
{
	THREAD_CREATE_NONE      = 0,
	THREAD_CREATE_JOINABLE  = 1 << 0,	// The process can be joined by another thread
	THREAD_CREATE_INTERRUPT = 1 << 1,	// The process can be interrupted
	THREAD_CREATE_RENDERING = 1 << 2	// The process is somehow used for rendering
};

enum ThreadWait_Z
{
	THREAD_WAIT_TIMEOUT    = -3,
	THREAD_WAIT_ABANDONNED = -2,
	THREAD_WAIT_FAILED     = -1,
	THREAD_WAIT_SIGNALED   =  0
};

//
// Implement thread object
typedef U32 (*ThreadProc_Z)(void* userdata);

class PlatformThreadObject_Z;

struct  ThreadParam_Z {
	ThreadProc_Z	ThreadProc;
	void*			UserData;
	S32				StackSize;
	S32				X360ProcessorAffinity;
	S32				Priority;
	U32             Flags;
	String_Z<64>	Name;
	ThreadParam_Z() :
		ThreadProc(NULL),
		UserData(NULL),
		StackSize(THREAD_BASE_STACK),
		X360ProcessorAffinity(0),
		Priority(THREAD_BASE_PRIORITY),
		Flags(THREAD_CREATE_JOINABLE),
		Name("ACE: Thread_Z")
	{
	}
	ThreadParam_Z(ThreadProc_Z _ThreadProc,void* _UserData) :
		ThreadProc(_ThreadProc),
		UserData(_UserData),
		StackSize(THREAD_BASE_STACK),
		X360ProcessorAffinity(0),
		Priority(THREAD_BASE_PRIORITY),
		Flags(THREAD_CREATE_JOINABLE),
		Name("ACE: Thread_Z")
	{
	}
};

struct  ProcInfo_Z
{
	U16 physical;			// Number of physical processors (cores)
	U16 logical;			// Number of logical processors (e.g. twice the number of cores on hyperthreaded CPUs)
	U16 scalar;				// Number of scalar processors (SPU-like)
	U32 cpuCacheSizeL1;		// Size of the L1 cache, in bytes 
	U32 cpuCacheSizeL2;		// Size of the L2 cache, in bytes
	U32 cpuCacheSizeL3;		// Size of the L3 cache, in bytes
	U32 cpuCacheLineSize;	// Size of a CPU cache line (L2), in bytes
};

class  Thread_Z
{
public:

	Thread_Z() : m_ThreadObject(NULL) {}
	Thread_Z(const ThreadParam_Z& _ThParam);
	Thread_Z(ThreadProc_Z _ThreadProc,void* _UserData);

	virtual ~Thread_Z();

	virtual Bool			IsInited() { return m_ThreadObject?TRUE:FALSE; }
	virtual void			Init(const ThreadParam_Z& _ThParam);
			 void			Init(ThreadProc_Z _ThreadProc,void* _UserData);
	virtual Bool			Start();
	virtual ThreadWait_Z	Join(S64 uTimeOut=-1);											// The time-out interval, default is INFINITE, return FALSE if failed
	virtual void			Suspend();
	virtual void			Resume();
	virtual void            SynchStop() {}													// Custom function to stop safely the thread processing
	virtual void            SynchRestart() {}												// Custom function to restart safely the thread processing
	virtual void			Shut();

	virtual void			ChangePriority(S32 Priority);

	const Char*				GetName() const	{ return m_ThreadParam.Name.Get(); }

	S32						ThreadProc();

	static  S32				SafeIncrement( volatile S32* addend );
	static  S32				SafeDecrement( volatile S32* addend );
	static  S32				SafeAdd( volatile S32* addend, S32 value );

	static  U32				GetThreadId();									// Return the id of the current thread
	static  U32				GetProcessorId();								// Return the id of the logical processor on which the code runs
	static  Bool			GetProcessorInfo( ProcInfo_Z& info );			// Retrieve information about the various processors available on the platform. Return a success code
	static	void			Sleep(U64 uTime);								//!\\ uTime is in µs!
	static	void			SuspendThreads( ThreadCreationFlag flag );		// Safely stops the threads that match the flag
	static	void			ResumeThreads( ThreadCreationFlag flag );		// Safely restarts the threads that match the flag
	static	void			SetDebugName( U32 threadId, const Char* name );

	PlatformThreadObject_Z*	GetThreadObject() { return m_ThreadObject; }

private:

	PlatformThreadObject_Z*	m_ThreadObject;
	ThreadParam_Z			m_ThreadParam;
	U32						m_Flags;

	static Bool RegisterRunningThread( Thread_Z* thread );
	static Bool UnregisterRunningThread( Thread_Z* thread );
private:

	Thread_Z(const Thread_Z &);
	const Thread_Z & operator=(const Thread_Z &);
};

#endif
