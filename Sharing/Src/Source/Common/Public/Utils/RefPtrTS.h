//////////////////////////////////////////////////////////////////////////
// RefPtrTS.h
//
// A version of ref_ptr that makes access to its contents thread-safe
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

template <class T>
class ref_ptr_ts
{
public:
	typedef T element_type;

	template <class R>
	class LockingProxy
	{
	public:
		LockingProxy(R* pObj) : m_ptr(pObj) { m_ptr->Lock(); }

		~LockingProxy() { m_ptr->Unlock(); }

		R* operator->() const { return m_ptr; }

	private:
		LockingProxy& operator=(const LockingProxy&);
		R* m_ptr;
	};



	ref_ptr_ts(T* pRefCounted = NULL) : m_pRefCounted(pRefCounted)
	{
		static_assert(std::is_base_of<AtomicRefCounted, T>::value, "type parameter of this class must derive from AtomicRefCounted");

		AddRef(m_pRefCounted);
	}

	template <class OtherT>
	ref_ptr_ts(const ref_ptr_ts<OtherT>& rhs) : m_pRefCounted(rhs.get())
	{
		AddRef(m_pRefCounted);
	}

	ref_ptr_ts(const ref_ptr_ts& rhs) : m_pRefCounted(rhs.get())
	{
		AddRef(m_pRefCounted);
	}

	~ref_ptr_ts()
	{
		SubRef(m_pRefCounted);
	}

	template <class OtherT>
	ref_ptr_ts& operator=(const ref_ptr_ts<OtherT>& rhs)
	{
		return (*this = rhs.get());
	}

	ref_ptr_ts& operator=(const ref_ptr_ts& rhs)
	{
		return (*this = rhs.get());
	}

	ref_ptr_ts& operator=(T* rhs)
	{
		AddRef(rhs);
		SubRef(m_pRefCounted);
		m_pRefCounted = rhs;
		return *this;
	}

	T* get() const
	{
		return m_pRefCounted;
	}

	LockingProxy<T> operator->() const
	{
		return LockingProxy<T>(m_pRefCounted);
	}

	class empty_type;
	operator const empty_type*() const
	{
		return reinterpret_cast<const empty_type*>(m_pRefCounted);
	}

	void swap(ref_ptr_ts& rhs)
	{
		T* tmp = m_pRefCounted;
		m_pRefCounted = rhs.m_pRefCounted;
		rhs.m_pRefCounted = tmp;
	}

private:

	static inline void AddRef(T* pRefCounted)
	{
		if (pRefCounted != nullptr)
			pRefCounted->AddRef();
	}

	static inline void SubRef(T* pRefCounted)
	{
		if (pRefCounted != nullptr)
			pRefCounted->RemoveRef();
	}

	T* m_pRefCounted;
};


template <class T, class U>
inline bool operator==(const ref_ptr_ts<T>& lhs, const ref_ptr_ts<U>& rhs)
{
	return lhs.get() == rhs.get();
}

template <class T, class U>
inline bool operator!=(const ref_ptr_ts<T>& lhs, const ref_ptr_ts<U>& rhs)
{
	return lhs.get() != rhs.get();
}

template <class T>
inline bool operator==(const ref_ptr_ts<T>& lhs, const T* rhs)
{
	return lhs.get() == rhs;
}

template <class T>
inline bool operator!=(const ref_ptr_ts<T>& lhs, const T* rhs)
{
	return lhs.get() != rhs;
}

template <class T>
inline bool operator==(const T* lhs, const ref_ptr_ts<T>& rhs)
{
	return lhs == rhs.get();
}

template <class T>
inline bool operator!=(const T* lhs, const ref_ptr_ts<T>& rhs)
{
	return lhs != rhs.get();
}

template <class T, class U>
inline bool operator==(const ref_ptr_ts<T>& lhs, const U* rhs)
{
	return lhs.get() == rhs;
}

template <class T, class U>
inline bool operator!=(const ref_ptr_ts<T>& lhs, const U* rhs)
{
	return lhs.get() != rhs;
}

template <class T>
inline bool operator<(const ref_ptr_ts<T>& lhs, const ref_ptr_ts<T>& rhs)
{
	return lhs.get() < rhs.get();
}

template <class T>
void swap(ref_ptr_ts<T>& lhs, ref_ptr_ts<T>& rhs)
{
	lhs.swap(rhs);
}


XTOOLS_NAMESPACE_END


#if !defined(SWIG)
# define DECLARE_THREADSAFE_PTR(X) \
	typedef ref_ptr_ts<X> X##Ptr; \
	typedef ref_ptr_ts<const X> X##ConstPtr;
#else 
# define DECLARE_THREADSAFE_PTR(X)
#endif
