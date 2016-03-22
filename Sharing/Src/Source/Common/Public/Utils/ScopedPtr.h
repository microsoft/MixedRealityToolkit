//////////////////////////////////////////////////////////////////////////
// ScopedPtr.h
//
// Smart pointer to an object that deletes the object when it goes out of scope
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

template <class T>
class scoped_ptr
{
public:
	typedef T element_type;

	scoped_ptr(T* pRefCounted = NULL) : m_pObject(pRefCounted)
	{
	}

	~scoped_ptr()
	{
		if (m_pObject)
		{
			delete m_pObject;
		}
	}

	scoped_ptr& operator=(T* rhs)
	{
		this->~scoped_ptr();
		m_pObject = rhs;
		return *this;
	}

	T* get() const
	{
		return m_pObject;
	}

	T* operator->() const
	{
		return m_pObject;
	}

	T& operator*() const
	{
		return *m_pObject;
	}

	class empty_type;
	operator const empty_type*() const
	{
		return reinterpret_cast<const empty_type*>(m_pObject);
	}

	void swap(scoped_ptr& rhs)
	{
		T* tmp = m_pObject;
		m_pObject = rhs.m_pObject;
		rhs.m_pObject = tmp;
	}

	T* release()
	{
		T* tmp = m_pObject;
		m_pObject = NULL;
		return tmp;
	}

private:
	// Prevent default copy construction
	scoped_ptr(const scoped_ptr& rhs);
	scoped_ptr& operator=(const scoped_ptr& rhs);

	T* m_pObject;
};


template <class T, class U>
inline bool operator==(const scoped_ptr<T>& lhs, const scoped_ptr<U>& rhs)
{
	return lhs.get() == rhs.get();
}

template <class T, class U>
inline bool operator!=(const scoped_ptr<T>& lhs, const scoped_ptr<U>& rhs)
{
	return lhs.get() != rhs.get();
}

template <class T>
inline bool operator==(const scoped_ptr<T>& lhs, const T* rhs)
{
	return lhs.get() == rhs;
}

template <class T>
inline bool operator!=(const scoped_ptr<T>& lhs, const T* rhs)
{
	return lhs.get() != rhs;
}

template <class T>
inline bool operator==(const T* lhs, const scoped_ptr<T>& rhs)
{
	return lhs == rhs.get();
}

template <class T>
inline bool operator!=(const T* lhs, const scoped_ptr<T>& rhs)
{
	return lhs != rhs.get();
}

template <class T, class U>
inline bool operator==(const scoped_ptr<T>& lhs, const U* rhs)
{
	return lhs.get() == rhs;
}

template <class T, class U>
inline bool operator!=(const scoped_ptr<T>& lhs, const U* rhs)
{
	return lhs.get() != rhs;
}

template <class T>
inline bool operator<(const scoped_ptr<T>& lhs, const scoped_ptr<T>& rhs)
{
	return lhs.get() < rhs.get();
}

template <class T>
void swap(scoped_ptr<T>& lhs, scoped_ptr<T>& rhs)
{
	lhs.swap(rhs);
}

template <class T>
T* get_pointer(scoped_ptr<T> const & p)
{
	return p.get();
}

XTOOLS_NAMESPACE_END
