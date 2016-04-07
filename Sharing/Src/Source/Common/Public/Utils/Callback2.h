//////////////////////////////////////////////////////////////////////////
// Callback2.h
//
// Convenience object to wrap callbacks to member functions with two parameters
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <Public/Utils/RefCounted.h>
#include <Public/Utils/RefPtr.h>

XTOOLS_NAMESPACE_BEGIN

template<typename T, typename R>
class Callback2
{
public:

	Callback2() {}

	template<typename C>
	Callback2(const ref_ptr<C>& object, void (C::*func) (T, R))
	{
		m_memberFunction = new MemberFuncTRef<C>(object, func);
	}

	template<typename C>
	Callback2(C* object, void (C::*func) (T, R))
	{
		m_memberFunction = new MemberFuncT<C>(object, func);
	}

	void Call(T param1, R param2) const
	{
		if (m_memberFunction) m_memberFunction->Call(param1, param2);
	}

	bool IsBound() const { return m_memberFunction != NULL; }

	void Clear() { m_memberFunction = NULL; }

private:
	class MemberFunc : public AtomicRefCounted
	{
	public:
		virtual void Call(T param1, R param2) const = 0;
	};

	template<typename C>
	class MemberFuncTRef : public MemberFunc
	{
	public:
		MemberFuncTRef(const ref_ptr<C>& object, void (C::*func) (T, R)) : m_object(object), m_func(func) {}
		virtual void Call(T param1, R param2) const { ((m_object.get())->*m_func)(param1, param2); }

	private:
		ref_ptr<C> m_object;
		void (C::* m_func) (T, R);
	};

	template<typename C>
	class MemberFuncT : public MemberFunc
	{
	public:
		MemberFuncT(C* object, void (C::*func) (T, R)) : m_object(object), m_func(func) {}
		virtual void Call(T param1, R param2) const { (m_object->*m_func)(param1, param2); }

	private:
		C* m_object;
		void (C::* m_func) (T, R);
	};

	// Safe for use with automatic copy constructor
	ref_ptr<MemberFunc> m_memberFunction;
};




// Create a callback without the user needing to set any template parameters
template<typename C, typename T, typename R>
Callback2<T, R> CreateCallback2(C* object, void (C::*func) (T, R))
{
	return Callback2<T, R>(object, func);
}

template<typename C, typename T, typename R>
Callback2<T, R> CreateCallback2(const ref_ptr<C>& object, void (C::*func) (T, R))
{
	return Callback2<T, R>(object, func);
}


XTOOLS_NAMESPACE_END
