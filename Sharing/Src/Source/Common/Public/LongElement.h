//////////////////////////////////////////////////////////////////////////
// LongElement.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

DECLARE_PTR_PRE(LongElement)

/// Represents a signed 64-bit integer element type, and provides functions for getting and setting the value. 
class LongElement : public Element
{
public:
	/// If the given Element is a LongElement, cast it to the derived type.  Otherwise return null
	static ref_ptr<LongElement> Cast(const ElementPtr& element);

	/// Get the current value of the element.  Returns immediately, does not allocate.  
	virtual int64 GetValue() const = 0;

	/// Sets the value of the element locally and automatically queues up the change to be synced
	/// to all the remote clients.  Returns immediately.  
	virtual void SetValue(int64 newValue) = 0;
};

DECLARE_PTR_POST(LongElement)

XTOOLS_NAMESPACE_END
