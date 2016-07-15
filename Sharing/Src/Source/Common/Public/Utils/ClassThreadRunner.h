// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// ClassThreadRunner.h
// Creates a class on its own thread and updates it until the thread exits
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class ClassThreadRunner : public RefCounted
{
public:

	template <typename T, typename ParamType>
	ClassThreadRunner(const ParamType& param)
	{
		std::auto_ptr<ParamType> p(new ParamType(param));

		if (XTVERIFY(m_thread.Fork(ClassThreadRunner::ThreadProc<T, ParamType>, p.get())))
		{
			// The ThreadProc now has the responsibility of deleting the pair.
			p.release();
		}
	}

	virtual ~ClassThreadRunner();

	void WaitForThreadExit();

private:

	template <typename T, typename ParamType>
	static void ThreadProc(void* context)
	{
		std::auto_ptr<ParamType> p(static_cast<ParamType *>(context));

		T myObject(*p);

		myObject.Update();
	}

	XThread			m_thread;
};

DECLARE_PTR(ClassThreadRunner)

XTOOLS_NAMESPACE_END
