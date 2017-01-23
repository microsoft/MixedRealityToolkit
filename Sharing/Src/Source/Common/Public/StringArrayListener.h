//////////////////////////////////////////////////////////////////////////
// StringArrayListener.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

// Disable unused parameter warnings.  Pure virtual functions don't play nice with SWIG, but we still want 
// to see the names of the parameters to make reading the code clearer
#pragma warning( push )
#pragma warning( disable : 4100 ) 

XTOOLS_NAMESPACE_BEGIN

XT_LISTENER_DECLARE(StringArrayListener)

/// Inherit from this class to receive callbacks from a StringArrayElement when it changes
class StringArrayListener XTABSTRACT : public Listener
{
public:
	virtual ~StringArrayListener() {}

	/// Called when the value of an element is changed, but the length of the array stays the same
	virtual void OnValueChanged(int32 index, const XStringPtr& newValue) {}

	/// Called when a new element is inserted into the array
	virtual void OnValueInserted(int32 index, const XStringPtr& value) {}

	/// Called when an element is removed from the array
	virtual void OnValueRemoved(int32 index, const XStringPtr& value) {}
};

XTOOLS_NAMESPACE_END

#pragma warning( pop )