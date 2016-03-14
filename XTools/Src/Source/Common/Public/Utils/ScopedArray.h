//////////////////////////////////////////////////////////////////////////
// ScopedArray.h
//
// Smart pointer to an array that deletes the array when it goes out of scope
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

template <class T>
class scoped_array
{
public:
	typedef T element_type;

	scoped_array(T* pRefCounted = NULL) : m_pArray(pRefCounted)
	{
	}

	~scoped_array()
	{
		if (m_pArray)
		{
			delete [] m_pArray;
		}
	}

	scoped_array& operator=(T* rhs)
	{
		this->~scoped_array();
		m_pArray = rhs;
		return *this;
	}

	T* get() const
	{
		return m_pArray;
	}

	T& operator[](int index)
	{
		return m_pArray[index];
	}

	const T& operator[](int index) const
	{
		return m_pArray[index];
	}

	class empty_type;
	operator const empty_type*() const
	{
		return reinterpret_cast<const empty_type*>(m_pArray);
	}

	void swap(scoped_array& rhs)
	{
		T* tmp = m_pArray;
		m_pArray = rhs.m_pArray;
		rhs.m_pArray = tmp;
	}

	T* release()
	{
		T* tmp = m_pArray;
		m_pArray = NULL;
		return tmp;
	}

private:
	// Prevent default copy construction
	scoped_array(const scoped_array& rhs);
	scoped_array& operator=(const scoped_array& rhs);

	T* m_pArray;
};


template <class T, class U>
inline bool operator==(const scoped_array<T>& lhs, const scoped_array<U>& rhs)
{
	return lhs.get() == rhs.get();
}

template <class T, class U>
inline bool operator!=(const scoped_array<T>& lhs, const scoped_array<U>& rhs)
{
	return lhs.get() != rhs.get();
}

template <class T>
inline bool operator==(const scoped_array<T>& lhs, const T* rhs)
{
	return lhs.get() == rhs;
}

template <class T>
inline bool operator!=(const scoped_array<T>& lhs, const T* rhs)
{
	return lhs.get() != rhs;
}

template <class T>
inline bool operator==(const T* lhs, const scoped_array<T>& rhs)
{
	return lhs == rhs.get();
}

template <class T>
inline bool operator!=(const T* lhs, const scoped_array<T>& rhs)
{
	return lhs != rhs.get();
}

template <class T, class U>
inline bool operator==(const scoped_array<T>& lhs, const U* rhs)
{
	return lhs.get() == rhs;
}

template <class T, class U>
inline bool operator!=(const scoped_array<T>& lhs, const U* rhs)
{
	return lhs.get() != rhs;
}

template <class T>
inline bool operator<(const scoped_array<T>& lhs, const scoped_array<T>& rhs)
{
	return lhs.get() < rhs.get();
}

template <class T>
void swap(scoped_array<T>& lhs, scoped_array<T>& rhs)
{
	lhs.swap(rhs);
}


XTOOLS_NAMESPACE_END
