/////////////////////////////////////////////////////////
// RefPtr.h
//
// Smart pointer for classes that support internal reference counting
//
// Copyright (C) 2014 Microsoft Inc, All rights reserved
/////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

template <class T>
class ref_ptr
{
public:
	typedef T element_type;

#if !defined(SWIG)
	ref_ptr(T* pRefCounted = NULL) : m_pRefCounted(pRefCounted)
	{
		AddRef(m_pRefCounted);
	}
#endif

	template <class OtherT>
	ref_ptr(const ref_ptr<OtherT>& rhs) : m_pRefCounted(rhs.get())
	{
		AddRef(m_pRefCounted);
	}

	ref_ptr(const ref_ptr& rhs) : m_pRefCounted(rhs.get())
	{
		AddRef(m_pRefCounted);
	}

	~ref_ptr()
	{
		SubRef(m_pRefCounted);
	}

	template <class OtherT>
	ref_ptr& operator=(const ref_ptr<OtherT>& rhs)
	{
		return (*this = rhs.get());
	}

#if !defined(SWIG)
	ref_ptr& operator=(const ref_ptr& rhs)
	{
		return (*this = rhs.get());
	}

	ref_ptr& operator=(T* rhs)
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
#endif

	T* operator->() const
	{
		return m_pRefCounted;
	}

	T& operator*() const
	{
		return *m_pRefCounted;
	}

#if !defined(SWIG)
	class empty_type;
	operator const empty_type*() const
	{
		return reinterpret_cast<const empty_type*>(m_pRefCounted);
	}
#endif

	void swap(ref_ptr& rhs)
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
inline bool operator==(const ref_ptr<T>& lhs, const ref_ptr<U>& rhs)
{
	return lhs.get() == rhs.get();
}

template <class T, class U>
inline bool operator!=(const ref_ptr<T>& lhs, const ref_ptr<U>& rhs)
{
	return lhs.get() != rhs.get();
}

template <class T>
inline bool operator==(const ref_ptr<T>& lhs, const T* rhs)
{
	return lhs.get() == rhs;
}

template <class T>
inline bool operator!=(const ref_ptr<T>& lhs, const T* rhs)
{
	return lhs.get() != rhs;
}

template <class T>
inline bool operator==(const T* lhs, const ref_ptr<T>& rhs)
{
	return lhs == rhs.get();
}

template <class T>
inline bool operator!=(const T* lhs, const ref_ptr<T>& rhs)
{
	return lhs != rhs.get();
}

template <class T, class U>
inline bool operator==(const ref_ptr<T>& lhs, const U* rhs)
{
	return lhs.get() == rhs;
}

template <class T, class U>
inline bool operator!=(const ref_ptr<T>& lhs, const U* rhs)
{
	return lhs.get() != rhs;
}

template <class T>
inline bool operator<(const ref_ptr<T>& lhs, const ref_ptr<T>& rhs)
{
	return lhs.get() < rhs.get();
}

template <class T>
void swap(ref_ptr<T>& lhs, ref_ptr<T>& rhs)
{
	lhs.swap(rhs);
}

template <typename T>
T* get_pointer(ref_ptr<T> const & p)
{
	return p.get();
}

template <typename T>
const T* get_pointer(ref_ptr<const T> const & p)
{
	return p.get();
}

XTOOLS_NAMESPACE_END


#if !defined(SWIG)
# define DECLARE_PTR_PRE(X) 

# define DECLARE_PTR_POST(X) \
	typedef XTools::ref_ptr<X> X##Ptr; \
	typedef XTools::ref_ptr<const X> X##ConstPtr; 

# define DECLARE_PTR(X) \
	DECLARE_PTR_PRE(X) \
	DECLARE_PTR_POST(X)
#else 
# define DECLARE_PTR_PRE(X) \
	%ref_ptr(X);

# define DECLARE_PTR_POST(X) \
	typedef XTools::ref_ptr<X> X##Ptr; \
	typedef XTools::ref_ptr<const X> X##ConstPtr; \
	%feature("novaluewrapper") ref_ptr<X>; \
	%template() ref_ptr<X>;

# define DECLARE_PTR(X) \
	DECLARE_PTR_PRE(X) \
	DECLARE_PTR_POST(X)

#endif
