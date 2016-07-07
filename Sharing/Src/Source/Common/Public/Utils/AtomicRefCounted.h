// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// AtomicRefCounted.h
// Thread-save version of RefCounted
//////////////////////////////////////////////////////////////////////////

#pragma once

#if !defined(SWIG)
# include <atomic>
#endif

XTOOLS_NAMESPACE_BEGIN

#if SWIG
%ignore AtomicRefCounted;
%feature("ref")   AtomicRefCounted "if($this) $this->AddRef();"
%feature("unref") AtomicRefCounted "if($this) $this->RemoveRef();"
#endif
class AtomicRefCounted
{
public:
	AtomicRefCounted() : m_refCount(0) {}
	virtual ~AtomicRefCounted() {}

	void AddRef() const { (++m_refCount); }

	void RemoveRef() const
	{
		if (m_refCount.fetch_sub(1) <= 1)
		{
			delete this;
		}
	}

	int GetRefCount() const { return m_refCount; }

private:
	// Prevent copy construction
	AtomicRefCounted(const RefCounted&);
	AtomicRefCounted& operator=(const AtomicRefCounted&);

	// mutable so that the ref count can still change
	// when the class is referenced as a const type
	mutable std::atomic<int> m_refCount;

};

XTOOLS_NAMESPACE_END
