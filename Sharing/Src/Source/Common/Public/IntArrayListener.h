// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// IntArrayListener.h
//////////////////////////////////////////////////////////////////////////

#pragma once

// Disable unused parameter warnings.  Pure virtual functions don't play nice with SWIG, but we still want 
// to see the names of the parameters to make reading the code clearer
#pragma warning( push )
#pragma warning( disable : 4100 ) 

XTOOLS_NAMESPACE_BEGIN

XT_LISTENER_DECLARE(IntArrayListener)

/// Inherit from this class to receive callbacks from an IntArrayElement when it changes
class IntArrayListener XTABSTRACT : public Listener
{
public:
	virtual ~IntArrayListener() {}

	/// Called when the value of an element is changed, but the length of the array stays the same
	virtual void OnValueChanged(int32 index, int32 newValue) {}

	/// Called when a new element is inserted into the array
	virtual void OnValueInserted(int32 index, int32 value) {}

	/// Called when an element is removed from the array
	virtual void OnValueRemoved(int32 index, int32 value) {}
};

XTOOLS_NAMESPACE_END

#pragma warning( pop )
