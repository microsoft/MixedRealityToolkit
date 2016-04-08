//////////////////////////////////////////////////////////////////////////
// Callback3.h
//
// Convenience object to wrap callbacks to member functions with three parameters
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <Public/Utils/RefCounted.h>
#include <Public/Utils/RefPtr.h>

XTOOLS_NAMESPACE_BEGIN

template<typename T, typename R, typename S>
class Callback3
{
public:

	Callback3() {}

	template<typename C>
	Callback3(const ref_ptr<C>& object, void (C::*func) (T, R, S))
	{
		m_memberFunction = new MemberFuncTRef<C>(object, func);
	}

	template<typename C>
	Callback3(C* object, void (C::*func) (T, R, S))
	{
		m_memberFunction = new MemberFuncT<C>(object, func);
	}

	void Call(T param1, R param2, S param3) const
	{
		if (m_memberFunction) m_memberFunction->Call(param1, param2, param3);
	}

	bool IsBound() const { return m_memberFunction != NULL; }

	void Clear() { m_memberFunction = NULL; }

private:
	class MemberFunc : public AtomicRefCounted
	{
	public:
		virtual void Call(T param1, R param2, S param3) const = 0;
	};

	template<typename C>
	class MemberFuncTRef : public MemberFunc
	{
	public:
		MemberFuncTRef(const ref_ptr<C>& object, void (C::*func) (T, R, S)) : m_object(object), m_func(func) {}
		virtual void Call(T param1, R param2, S param3) const { ((m_object.get())->*m_func)(param1, param2, param3); }

	private:
		ref_ptr<C> m_object;
		void (C::* m_func) (T, R, S);
	};

	template<typename C>
	class MemberFuncT : public MemberFunc
	{
	public:
		MemberFuncT(C* object, void (C::*func) (T, R, S)) : m_object(object), m_func(func) {}
		virtual void Call(T param1, R param2, S param3) const { (m_object->*m_func)(param1, param2, param3); }

	private:
		C* m_object;
		void (C::* m_func) (T, R, S);
	};

	// Safe for use with automatic copy constructor
    ref_ptr<MemberFunc> m_memberFunction;
};




// Create a callback without the user needing to set any template parameters
template<typename C, typename T, typename R, typename S>
Callback3<T, R, S> CreateCallback3(C* object, void (C::*func) (T, R, S))
{
	return Callback3<T, R, S>(object, func);
}

template<typename C, typename T, typename R, typename S>
Callback3<T, R, S> CreateCallback3(const ref_ptr<C>& object, void (C::*func) (T, R, S))
{
	return Callback3<T, R, S>(object, func);
}


XTOOLS_NAMESPACE_END
