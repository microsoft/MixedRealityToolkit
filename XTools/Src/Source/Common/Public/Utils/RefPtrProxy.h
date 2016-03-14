//////////////////////////////////////////////////////////////////////////
// RefPtrProxy.h
//
// 
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

template <class T>
class ref_ptr_proxy
{
public:
	typedef T element_type;

#if !defined(SWIG)
	ref_ptr_proxy(T* pRefCounted = NULL) : m_pRefCounted(pRefCounted)
	{
		AddRef(m_pRefCounted);
	}
#endif

	template <class OtherT>
	ref_ptr_proxy(const ref_ptr_proxy<OtherT>& rhs) : m_pRefCounted(rhs.get())
	{
		AddRef(m_pRefCounted);
	}

	ref_ptr_proxy(const ref_ptr_proxy& rhs) : m_pRefCounted(rhs.get())
	{
		AddRef(m_pRefCounted);
	}

	~ref_ptr_proxy()
	{
		SubRef(m_pRefCounted);
	}

	template <class OtherT>
	ref_ptr_proxy& operator=(const ref_ptr_proxy<OtherT>& rhs)
	{
		return (*this = rhs.get());
	}

	ref_ptr_proxy& operator=(const ref_ptr_proxy& rhs)
	{
		return (*this = rhs.get());
	}

	ref_ptr_proxy& operator=(T* rhs)
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

	T& operator->()
	{
		return *m_pRefCounted;
	}

	const T& operator->() const
	{
		return *m_pRefCounted;
	}

	T& operator*() const
	{
		return *m_pRefCounted;
	}


	class empty_type;
	operator const empty_type*() const
	{
		return reinterpret_cast<const empty_type*>(m_pRefCounted);
	}

	void swap(ref_ptr_proxy& rhs)
	{
		T* tmp = m_pRefCounted;
		m_pRefCounted = rhs.m_pRefCounted;
		rhs.m_pRefCounted = tmp;
	}

	int get_ref_count() const
	{
		return m_pRefCounted ? m_pRefCounted->GetRefCount() : 0;
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
inline bool operator==(const ref_ptr_proxy<T>& lhs, const ref_ptr_proxy<U>& rhs)
{
	return lhs.get() == rhs.get();
}

template <class T, class U>
inline bool operator!=(const ref_ptr_proxy<T>& lhs, const ref_ptr_proxy<U>& rhs)
{
	return lhs.get() != rhs.get();
}

template <class T>
inline bool operator==(const ref_ptr_proxy<T>& lhs, const T* rhs)
{
	return lhs.get() == rhs;
}

template <class T>
inline bool operator!=(const ref_ptr_proxy<T>& lhs, const T* rhs)
{
	return lhs.get() != rhs;
}

template <class T>
inline bool operator==(const T* lhs, const ref_ptr_proxy<T>& rhs)
{
	return lhs == rhs.get();
}

template <class T>
inline bool operator!=(const T* lhs, const ref_ptr_proxy<T>& rhs)
{
	return lhs != rhs.get();
}

template <class T, class U>
inline bool operator==(const ref_ptr_proxy<T>& lhs, const U* rhs)
{
	return lhs.get() == rhs;
}

template <class T, class U>
inline bool operator!=(const ref_ptr_proxy<T>& lhs, const U* rhs)
{
	return lhs.get() != rhs;
}

template <class T>
inline bool operator<(const ref_ptr_proxy<T>& lhs, const ref_ptr_proxy<T>& rhs)
{
	return lhs.get() < rhs.get();
}

template <class T>
void swap(ref_ptr_proxy<T>& lhs, ref_ptr_proxy<T>& rhs)
{
	lhs.swap(rhs);
}


XTOOLS_NAMESPACE_END


# define DECLARE_PTR_PROXY(X) \
	typedef XTools::ref_ptr_proxy<X> X##Ptr; \
	typedef XTools::ref_ptr_proxy<const X> X##ConstPtr;


