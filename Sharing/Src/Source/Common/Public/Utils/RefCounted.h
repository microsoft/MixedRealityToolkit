// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

/////////////////////////////////////////////////////////
// RefCounted.h
// Base class for types that use internal reference
// counting.  Use with ref_ptr.  
/////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

#if SWIG
%ignore RefCounted;
%feature("ref")   RefCounted "if($this) $this->AddRef();"
%feature("unref") RefCounted "if($this) $this->RemoveRef();"
#endif
class RefCounted
{
public:
	RefCounted() : m_refCount(0) {}
	virtual ~RefCounted() {}

	int AddRef() const { return (++m_refCount); }

	int RemoveRef() const
	{
		if (--m_refCount <= 0)
		{
			delete this;
			return 0;
		}

		return m_refCount;
	}

	int GetRefCount() const { return m_refCount; }

private:
	// Prevent copy construction
	RefCounted(const RefCounted&);
	RefCounted& operator=(const RefCounted&);

	// mutable so that the ref count can still change
	// when the class is referenced as a const type
	mutable int m_refCount;
};

XTOOLS_NAMESPACE_END
