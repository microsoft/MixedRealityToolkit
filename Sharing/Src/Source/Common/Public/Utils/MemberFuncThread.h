//////////////////////////////////////////////////////////////////////////
// MemberFuncThread.h
//
// Class that creates a thread and runs the given member function of the 
// given object on it
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>

XTOOLS_NAMESPACE_BEGIN

class MemberFuncThread : public RefCounted
{
public:

	template <typename T>
	MemberFuncThread(void (T::*function)(void), T *object)
	{
		typedef std::pair<void (T::*)(), T *> CallbackType;
		std::auto_ptr<CallbackType> p(new CallbackType(function, object));

		if ( XTVERIFY(m_thread.Fork(MemberFuncThread::ThreadProc<T>, p.get())) )
		{
			// The ThreadProc now has the responsibility of deleting the pair.
			p.release();
		}
	}

	virtual ~MemberFuncThread();

	void WaitForThreadExit();

private:

	template <typename T>
	static void ThreadProc(void* context)
	{
		typedef std::pair<void (T::*)(), T *> CallbackType;

		std::auto_ptr<CallbackType> p(static_cast<CallbackType *>(context));

		(p->second->*p->first)();
	}

	XThread			m_thread;
};

DECLARE_PTR(MemberFuncThread)

XTOOLS_NAMESPACE_END
