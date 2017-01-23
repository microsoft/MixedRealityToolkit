//////////////////////////////////////////////////////////////////////////
// FloatArrayListener.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

// Disable unused parameter warnings.  Pure virtual functions don't play nice with SWIG, but we still want 
// to see the names of the parameters to make reading the code clearer
#pragma warning( push )
#pragma warning( disable : 4100 ) 

XTOOLS_NAMESPACE_BEGIN

XT_LISTENER_DECLARE(FloatArrayListener)

/// Inherit from this class to receive callbacks from an FloatArrayElement when it changes
class FloatArrayListener XTABSTRACT : public Listener
{
public:
	virtual ~FloatArrayListener() {}

	/// Called when the value of an element is changed, but the length of the array stays the same
	virtual void OnValueChanged(int32 index, float newValue) {}

	/// Called when a new element is inserted into the array
	virtual void OnValueInserted(int32 index, float value) {}

	/// Called when an element is removed from the array
	virtual void OnValueRemoved(int32 index, float value) {}
};

XTOOLS_NAMESPACE_END

#pragma warning( pop )