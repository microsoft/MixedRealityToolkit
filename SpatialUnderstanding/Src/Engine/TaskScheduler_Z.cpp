// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include <pch.h>
#include <TaskScheduler_Z.h>

//#define _DEBUG_TASK_SCHEDULER

#ifdef _DEBUG_TASK_SCHEDULER
	#define SCHEDULER_MESSAGE_Z(...)	OUTPUT_Z(__VA_ARGS__)
#else
	#define SCHEDULER_MESSAGE_Z(...)
#endif

TaskScheduler_Z		TaskScheduler_Z::TheUnicJobScheduler;

JobGroup_Z::JobGroup_Z() : m_bAsynchronous(FALSE), m_PendingJobCount(0), m_CompletionEvent(NULL)
{
	SCHEDULER_MESSAGE_Z("[JobGroup_Z] Create JobGroup_Z %x",this);
	m_CompletionEvent = New_Z Event_Z();
	m_bInitialized = FALSE;
}

JobGroup_Z::~JobGroup_Z()
{
	SCHEDULER_MESSAGE_Z("[JobGroup_Z] Destroy JobGroup_Z 0x%x %s",this,m_Name.Get());
	// Don't Remove !
	// Have to wait End Task before to destroy JobGroup_Z.
	Shut();
	Delete_Z m_CompletionEvent;
}

void JobGroup_Z::Init( const U16* workerAffinity, S32 workerCount, const String_Z<16>& name )
{
	SCHEDULER_MESSAGE_Z("[JobGroup_Z] (%s) Init maxWorker=%d", name.Get(), workerCount);

	m_Name = name;
	m_bInitialized = TRUE;

	EXCEPTIONC_Z(m_CompletionEvent!=NULL,"JobGroup must have a valid Event_Z pointer");
	m_CompletionEvent->Reset();
}

void JobGroup_Z::Shut()
{
	SCHEDULER_MESSAGE_Z("[JobGroup_Z] SHUT JobGroup_Z %x",this);

	// Don't Remove ! Have to test and reset Wait Sate.
	WaitForCompletion();
}

void JobGroup_Z::Run( U32 count, JobProc_Z task, void* userData, Bool bWaitForCompletion )
{
	SCHEDULER_MESSAGE_Z("[JobGroup_Z] Run(%d,0x%x,0x%x)", count, task, userData);
	PROFILER_SCOPED_CPU_MARKER_L1(COLOR_GREEN,"Run");

	if(bWaitForCompletion)
		WaitForCompletion();

	// Add Jobs.
	m_CompletionEvent->Reset();
	Thread_Z::SafeAdd(&m_PendingJobCount,count);
	TaskScheduler_Z::TheUnicJobScheduler.Submit(count,*this,task,userData);

	if(m_bAsynchronous)
		WaitForCompletion();
}

void JobGroup_Z::WaitForCompletion()
{
	SCHEDULER_MESSAGE_Z("[JobGroup_Z] WaitForCompletion()");

	// Spin loop over job counter for this group
	while( m_PendingJobCount != 0 )
	{
		Sleep_Z(0);
	}

	//SCHEDULER_MESSAGE_Z("[JobGroup_Z] (%d %s) WaitEvents", taskCount, m_Threads[0].name.Get() );
	EXCEPTIONC_Z(m_PendingJobCount==0,"Job counter mismatch error");
	m_CompletionEvent->Reset();
	return;
}

// Test If Task is finished.
Bool JobGroup_Z::PollForCompletion(Bool ReleaseIfCompleted)
{
	SCHEDULER_MESSAGE_Z("[JobGroup_Z] PollForCompletion()");

	if( m_PendingJobCount != 0 )
		return FALSE;

	if (ReleaseIfCompleted)
		WaitForCompletion();

	return TRUE;
}

//
// TaskManager
// 

TaskScheduler_Z::TaskScheduler_Z()
{
	m_Running = FALSE;
}

TaskScheduler_Z::~TaskScheduler_Z()
{
}

Bool TaskScheduler_Z::Init()
{
	Bool bSuccess = Thread_Z::GetProcessorInfo( m_ProcInfo );
	EXCEPTION_Z( bSuccess );
	
	// Reset RingBuffer
	for( int i=0; i<_countof(m_CommandQueue); i++)
		m_CommandQueue[i].Flush();

	m_Running = TRUE;
	m_WorkerSemaphore.Create(0,16384);

	U32 numThreads = Min<U32>(m_ProcInfo.logical,MAX_TASK_MANAGER_PROCESSOR_COUNT);
	MESSAGE_Z("Initializing TaskManager with %d processor.", numThreads);
	m_WorkerThreadInfos.SetSize(numThreads);
	for(U32 i=0;i<numThreads;i++)
	{
		ThreadInfo_Z& threadInfo = m_WorkerThreadInfos[i];
		threadInfo.processormask=1<<i;

		ThreadParam_Z param;
		param.Name.Sprintf("ACE: WorkerThread %d", i );
		param.StackSize = TASK_MANAGER_THREAD_STACK;
		param.Priority = THREAD_PRIO_NORMAL;
		param.ThreadProc = &TaskScheduler_Z::ThreadProc;
		param.UserData = &threadInfo;
		param.X360ProcessorAffinity = i;
		threadInfo.thread.Init( param );
		threadInfo.thread.Start();
	}

	return TRUE;
}

void TaskScheduler_Z::Shut()
{
	if(m_Running)
	{
		m_Running = FALSE;
		U32 numThreads = Min<U32>(m_ProcInfo.logical,MAX_TASK_MANAGER_PROCESSOR_COUNT);
		m_WorkerSemaphore.Release(numThreads);
		for(U32 i=0;i<numThreads;i++)
		{
			ThreadInfo_Z& threadInfo = m_WorkerThreadInfos[i];
			threadInfo.thread.Join();
			threadInfo.thread.Shut();
		}
	}

	// Reset RingBuffers
	for( int i=0; i<_countof(m_CommandQueue); i++)
		m_CommandQueue[i].Flush();
}

void TaskScheduler_Z::Submit( U32 count, JobGroup_Z& group, JobGroup_Z::JobProc_Z task, void* userData )
{
	JobQueue_Z & jobQueue = m_CommandQueue[PrioNormal];

	// Lock the queue
	jobQueue.Lock();
	
	for(U32 i=0; i<count; i++)
	{
		// Ring Buffer Full ?
		//   It's prohibited to set m_WorkloadWriteIndex to a value containing a task.
		S32 FuturNewWrite = jobQueue.m_WriteIndex + 1;
		if(FuturNewWrite>=MAX_TASK_RING_BUFFER_COUNT)
			FuturNewWrite = 0;

		while (FuturNewWrite == jobQueue.m_ReadIndex)	// Wait task pop for avoid task OverWrite (I know, it's possible to write one more task... but it's the simple way).
			Thread_Z::Sleep(0);

		// Add Task to global ring buffer.
		JobGroup_Z::JobDesc_Z & aJob = jobQueue.m_RingBuffer[jobQueue.m_WriteIndex];
		aJob.m_Owner = &group;
		aJob.m_Proc = task;
		aJob.m_UserData = userData;
		aJob.m_Index = i;
		aJob.m_Count = count;

		// We must gurantee that aJob is completely flushed to memory before we increment writeIndex
		READWRITEBARRIER_Z();

		// Really PushIT !
		jobQueue.m_WriteIndex = FuturNewWrite;

		SCHEDULER_MESSAGE_Z("[TaskScheduler_Z] (%s) SUBMIT cb(0x%x) index(%d), count(%d)",group.m_Name.Get(),aJob.m_Proc,aJob.m_Index,aJob.m_Count);
	}

	// Unlock queue for other threads.
	jobQueue.UnLock();

	// Wakeup worker threads.
	m_WorkerSemaphore.Release( m_WorkerThreadInfos.GetSize() );
}

Bool	TaskScheduler_Z::SafeGetATask(U32 processormask,JobGroup_Z::JobDesc_Z &Task)
{
	JobQueue_Z & jobQueue = m_CommandQueue[PrioNormal];
	SharedResourceGuard_Z lock(jobQueue.m_CritSection);

	// Find a task.
	S32 NumTask = -1;
	S32 i = jobQueue.m_ReadIndex;
	while (i != jobQueue.m_WriteIndex)
	{
		// Check!
		if (1)
		{
			// Find it !
			NumTask = i;
			break;
		}
		// 
		i++;
		if(i>=MAX_TASK_RING_BUFFER_COUNT)
			i = 0;
	}
	if (NumTask<0)
	{
		// No Task.
		return FALSE;
	}

	// invert.
	Task = jobQueue.m_RingBuffer[NumTask];
	jobQueue.m_RingBuffer[NumTask] = jobQueue.m_RingBuffer[jobQueue.m_ReadIndex];
	jobQueue.m_RingBuffer[jobQueue.m_ReadIndex] = Task;

	jobQueue.m_ReadIndex = (jobQueue.m_ReadIndex + 1) % (MAX_TASK_RING_BUFFER_COUNT);

	return TRUE;
}

U32 TaskScheduler_Z::ThreadProc( void* data )
{
	TaskScheduler_Z	*pJobSched = &TaskScheduler_Z::TheUnicJobScheduler;

	ThreadInfo_Z* aThread = reinterpret_cast <ThreadInfo_Z*> ( data );
	while( pJobSched->m_Running )
	{
		// Wait until there is at least one task to execute on this particular processor/thread
		pJobSched->m_WorkerSemaphore.Wait();

		// Grab job from command queue.
		JobGroup_Z::JobDesc_Z Job;
		while (pJobSched->SafeGetATask(aThread->processormask,Job))
		{
			// Execute current task and signal completion
			// DON'T ADD PROFILER MARKER IN TASKSCHEDULER_Z ... too heavy and lead to crashes on some occasions
			if( Job.m_Proc )
			{
				SCHEDULER_MESSAGE_Z("[TaskScheduler_Z] (%s) RUN JOB %d / %d", Job.m_Owner->m_Name.Get(), Job.m_Index, Job.m_Count);
				(*Job.m_Proc)(Job.m_Index, Job.m_Count, Job.m_UserData);
				Job.m_Proc = NULL; //Reset proc pointer for safety (don't leave junk or dead link in ringbuffer)
				Thread_Z::SafeDecrement(&Job.m_Owner->m_PendingJobCount);
				if (Job.m_Owner->m_PendingJobCount == 0)
				{
					Job.m_Owner->m_CompletionEvent->Signal();
					SCHEDULER_MESSAGE_Z("[TaskScheduler_Z] (%s) GROUP COMPLETE", Job.m_Owner->m_Name.Get());
				}
			}
			else
			{
				SCHEDULER_MESSAGE_Z("[TaskScheduler_Z] EMPTY JOB... WEIRD");
			}
		}
	}
	return 0;
}

