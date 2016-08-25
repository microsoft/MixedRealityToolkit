// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#ifndef _TASK_SCHEDULER_Z_H
#define _TASK_SCHEDULER_Z_H

#include <BeginDef_Z.h>
#include <Std_Z.h>
#include <Thread_Z.h>

class TaskScheduler_Z;

class  JobGroup_Z
{
public:
	typedef void (*JobProc_Z)( const U16 taskIndex, const U16 tasksCount, void* userData );

	struct JobDesc_Z
	{
		JobGroup_Z *	m_Owner;
		JobProc_Z		m_Proc;
		void *			m_UserData;
		U16				m_Index;
		U16				m_Count;
	};

private:
	Bool					m_bInitialized;
	Bool					m_bAsynchronous;
	volatile S32			m_PendingJobCount;
	Event_Z *				m_CompletionEvent;
	String_Z<16>			m_Name;
	U32						processorsmask;

public :
	JobGroup_Z();
	~JobGroup_Z();

	void Init( const U16* workerAffinity, S32 workerCount, const String_Z<16>& name );
	Bool IsInitialized() const { return m_bInitialized; }
	void SetSynchronousMode(bool bSynchronous) { m_bAsynchronous = !bSynchronous; } //DEBUG purpose only
	void Shut();

	// Submit 'count' job to the scheduler. Specify if you want to sync with the group 'bWaitForCompletion' or just submit a new batch of job to the group
	void Run( U32 count, JobProc_Z task, void* userData, Bool bWaitForCompletion=TRUE );
	//void RunList( JobDesc_Z * jobList, S32 jobCount );

	// Wait for group execution completion
	void WaitForCompletion();
	Bool PollForCompletion(Bool ReleaseIfCompleted = TRUE);

	friend class TaskScheduler_Z;
};

//
// TaskScheduler_Z
//

#define TASK_MANAGER_THREAD_STACK			65536
#define MAX_TASK_MANAGER_PROCESSOR_COUNT	16
#define MAX_TASK_RING_BUFFER_COUNT			4096

template<typename T, U32 MaxSize> struct CommandQueue_Z
{
	SafeArray_Z<T,MaxSize,FALSE,FALSE> m_RingBuffer;
	volatile S32		m_ReadIndex;
	volatile S32		m_WriteIndex;
	SharedResource_Z	m_CritSection;

	void Flush()
	{
		m_ReadIndex = 0;
		m_WriteIndex = 0;
		memset(m_RingBuffer.GetArrayPtr(),0,m_RingBuffer.GetSize());
	}

	void PushBack(T command);
	T & Pop();

	void Lock()
	{
		m_CritSection.Lock();
	}

	void UnLock()
	{
		m_CritSection.Unlock();
	}
};

typedef CommandQueue_Z<JobGroup_Z::JobDesc_Z,MAX_TASK_RING_BUFFER_COUNT>	JobQueue_Z;

class  TaskScheduler_Z
{
public:
	enum CommandQueuePriority {
		//PrioLow,	//NICOB, not supported yet
		PrioNormal,
		//PrioHigh,	//NICOB, not supported yet
		PrioCount
	};

public:
	TaskScheduler_Z();
	~TaskScheduler_Z();

	Bool	Init();
	Bool	InitIfNeeded() { if (!m_Running) return Init(); else return TRUE; }
	void	Shut();

	static U32 ThreadProc( void* data );

	const ProcInfo_Z&	GetProcessorInfo() const { return m_ProcInfo; }
	
	// Get the number of worker thread.
	U32 GetWorkerThreadCount() const { return m_WorkerThreadInfos.GetSize(); }

	static TaskScheduler_Z		TheUnicJobScheduler;
private:
	JobQueue_Z			m_CommandQueue[PrioCount];
	Bool				SafeGetATask(U32 processormask,JobGroup_Z::JobDesc_Z &Task);

	Bool				m_Running;
	struct ThreadInfo_Z
	{
		Thread_Z						thread;
		U32								processormask;
	};
	StaticArray_Z<ThreadInfo_Z,MAX_TASK_MANAGER_PROCESSOR_COUNT,TRUE,TRUE> m_WorkerThreadInfos;
	ProcInfo_Z						m_ProcInfo;
	Semaphore_Z						m_WorkerSemaphore;

	void	Submit( U32 count, JobGroup_Z& group, JobGroup_Z::JobProc_Z task, void* userData );

	friend class JobGroup_Z;
};

#endif //_TASK_SCHEDULER_Z_H
