//////////////////////////////////////////////////////////////////////////
// Callback.h
//
// Convenience object to wrap callbacks to member functions.  
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <Public/Utils/RefCounted.h>
#include <Public/Utils/RefPtr.h>

XTOOLS_NAMESPACE_BEGIN

template<typename T>
class Callback
{
public:

	Callback() {}

	template<typename C>
	Callback(const ref_ptr<C>& object, void (C::*func) (T))
	{
		m_memberFunction = new MemberFuncTRef<C>(object, func);
	}

	template<typename C>
	Callback(C* object, void (C::*func) (T))
	{
		m_memberFunction = new MemberFuncT<C>(object, func);
	}

	void Call(T param) const
	{
		if (m_memberFunction) m_memberFunction->Call(param);
	}

	bool IsBound() const { return m_memberFunction != NULL; }

	void Clear() { m_memberFunction = NULL; }

private:
	class MemberFunc : public AtomicRefCounted
	{
	public:
		virtual void Call(T param) const = 0;
	};

	template<typename C>
	class MemberFuncTRef : public MemberFunc
	{
	public:
		MemberFuncTRef(const ref_ptr<C>& object, void (C::*func) (T)) : m_object(object), m_func(func) {}
		virtual void Call(T param) const { ((m_object.get())->*m_func)(param); }

	private:
		ref_ptr<C> m_object;
		void (C::* m_func) (T);
	};

	template<typename C>
	class MemberFuncT : public MemberFunc
	{
	public:
		MemberFuncT(C* object, void (C::*func) (T)) : m_object(object), m_func(func) {}
		virtual void Call(T param) const { (m_object->*m_func)(param); }

	private:
		C* m_object;
		void (C::* m_func) (T);
	};

	// Safe for use with automatic copy constructor
	ref_ptr<MemberFunc> m_memberFunction;
};




// Create a callback without the user needing to set any template parameters
template<typename C, typename T>
Callback<T> CreateCallback(C* object, void (C::*func) (T))
{
	return Callback<T>(object, func);
}

template<typename C, typename T>
Callback<T> CreateCallback(const ref_ptr<C>& object, void (C::*func) (T))
{
	return Callback<T>(object, func);
}



XTOOLS_NAMESPACE_END