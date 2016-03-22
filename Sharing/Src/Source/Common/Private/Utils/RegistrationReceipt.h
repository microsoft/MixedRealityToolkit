//////////////////////////////////////////////////////////////////////////
// RegistrationReceipt.h
//
// Receipt that automatically unregisters a callback when it is destroyed.
// Also serves to keep a reference to the callback emitter until all its listeners
// are released.  
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <Public/Utils/Receipt.h>
#include <Public/Utils/Callback.h>
#include <Public/Utils/Callback2.h>

XTOOLS_NAMESPACE_BEGIN

typedef uint32 RegistrationKey;

//////////////////////////////////////////////////////////////////////////
// Returns a new key each time its called.  Thread-safe
RegistrationKey GetNewRegistrationKey();


//////////////////////////////////////////////////////////////////////////
// Base class for registration receipts.  
class RegistrationReceipt : public Receipt
{
public:
	RegistrationReceipt(RegistrationKey key) : m_key(key) {}

	RegistrationKey GetKey() const { return m_key; }

private:
	RegistrationKey m_key;
};
DECLARE_PTR(RegistrationReceipt)

//////////////////////////////////////////////////////////////////////////
template<typename T>
class RegistrationReceipt1 : public RegistrationReceipt
{
public:
	RegistrationReceipt1(RegistrationKey key, const Callback<T>& unregisterCallback, T callbackParam)
		: RegistrationReceipt(key)
		, m_callback(unregisterCallback)
		, m_callbackParam(callbackParam)	{}

	virtual ~RegistrationReceipt1()
	{
		if (m_callback.IsBound())
		{
			m_callback.Call(m_callbackParam);
			m_callback.Clear();
		}
	}

	virtual void Clear() { m_callback.Clear(); }

private:
	Callback<T>		m_callback;
	T				m_callbackParam;
};


//////////////////////////////////////////////////////////////////////////
template<typename T, typename R>
class RegistrationReceipt2 : public RegistrationReceipt
{
public:
	RegistrationReceipt2(RegistrationKey key, const Callback2<T, R>& unregisterCallback, T callbackParam1, R callbackParam2)
		: RegistrationReceipt(key)
		, m_callback(unregisterCallback)
		, m_callbackParam1(callbackParam1)	
		, m_callbackParam2(callbackParam2)
	{}

	virtual ~RegistrationReceipt2()
	{
		if (m_callback.IsBound())
		{
			m_callback.Call(m_callbackParam1, m_callbackParam2);
			m_callback.Clear();
		}
	}

	virtual void Clear() { m_callback.Clear(); }

private:
	Callback2<T,R>	m_callback;
	T				m_callbackParam1;
	R				m_callbackParam2;
};


// Utility function that makes the appropriate type of registration receipt automatically
// without the need for the programmer to manually set the template parameters
template<typename C, typename T>
RegistrationReceiptPtr CreateRegistrationReceipt(const ref_ptr<C>& callbackObject, void (C::*callbackFunction) (T), T callbackParameter)
{
	return new RegistrationReceipt1<T>(GetNewRegistrationKey(), CreateCallback(callbackObject, callbackFunction), callbackParameter);
}

template<typename C, typename T, typename R, typename P, typename Q>
RegistrationReceiptPtr CreateRegistrationReceipt(const ref_ptr<C>& callbackObject, void (C::*callbackFunction) (T, R), P callbackParameter1, Q callbackParameter2)
{
	return new RegistrationReceipt2<T, R>(GetNewRegistrationKey(), CreateCallback2(callbackObject, callbackFunction), callbackParameter1, callbackParameter2);
}

XTOOLS_NAMESPACE_END
