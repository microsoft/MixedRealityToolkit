// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#include <pch.h>
#include <Assert_Z.h>	// Ces deux includes sont l� pour Kera.
#include <Thread_Z.h>
#include <Memory_Z.h>
#include <StaticArray_Z.h>
#include <System_Z.h>

namespace Thread
{
	// D�fini en dehors de la classe pour ne pas avoir d'includes suppl�mentaires dans le header
	typedef	SafeArrayPtr_Z<Thread_Z,256> ThreadPtrBuffer;
	static ThreadPtrBuffer	s_RegisteredThreads;
	static U16				s_ThreadCount(0);	
}

Thread_Z::Thread_Z(const ThreadParam_Z& _ThParam)
	: m_ThreadObject(NULL)
{
	Init(_ThParam);
}

Thread_Z::Thread_Z(ThreadProc_Z _ThreadProc,void* _UserData)
	: m_ThreadObject(NULL) 
{
	Init(_ThreadProc,_UserData);
}

void Thread_Z::Init(ThreadProc_Z _ThreadProc,void* _UserData)
{
	ThreadParam_Z	ThParam(_ThreadProc,_UserData);
	Init(ThParam);
}

S32 Thread_Z::ThreadProc()
{
	SetAndRestoreFloatControlDownward floatControlDownward;
	CHECKFTOLMODE();
	SetFloatControlExceptions();

	S32 ret;
	while((ret=m_ThreadParam.ThreadProc(m_ThreadParam.UserData)))
	{
		Sleep_Z(0);
	}
	return ret;
}

void Thread_Z::SuspendThreads( ThreadCreationFlag flag )
{
	Thread_Z* thread;
	for( U16 i = 0; i < Thread::s_ThreadCount; ++i )
	{
		thread = Thread::s_RegisteredThreads[i];
		if( thread->m_Flags & flag )
			thread->SynchStop();
	}
}

void Thread_Z::ResumeThreads( ThreadCreationFlag flag )
{
	Thread_Z* thread;
	for( U16 i = 0; i < Thread::s_ThreadCount; ++i )
	{
		thread = Thread::s_RegisteredThreads[i];
		if( thread->m_Flags & flag )
			thread->SynchRestart();
	}
}

Bool Thread_Z::RegisterRunningThread( Thread_Z* thread )
{
	Thread::s_RegisteredThreads[Thread::s_ThreadCount++] = thread;
	return TRUE;
}

Bool Thread_Z::UnregisterRunningThread( Thread_Z* thread )
{
	Thread_Z** arrayPtr = Thread::s_RegisteredThreads.GetArrayPtr();

	for( U16 i = 0; i < Thread::s_ThreadCount; ++i, ++arrayPtr )
	{
		if( *arrayPtr == thread )
		{
			--Thread::s_ThreadCount;
			memmove( arrayPtr, arrayPtr+1, (Thread::s_ThreadCount-i)*sizeof(Thread_Z*) );
			return TRUE;
		}
	}
	return FALSE;
}
