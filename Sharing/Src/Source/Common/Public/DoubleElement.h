//////////////////////////////////////////////////////////////////////////
// DoubleElement.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

DECLARE_PTR_PRE(DoubleElement)

/// Represents a 64-bit double precision floating point element type, and provides functions for getting and setting the value. 
class DoubleElement : public Element
{
public:
	/// If the given Element is a DoubleElement, cast it to the derived type.  Otherwise return null
	static ref_ptr<DoubleElement> Cast(const ElementPtr& element);

	/// Get the current value of the element.  Returns immediately, does not allocate.  
	virtual double GetValue() const = 0;

	/// Sets the value of the element locally and automatically queues up the change to be synced
	/// to all the remote clients.  Returns immediately.  
	virtual void SetValue(double newValue) = 0;
};

DECLARE_PTR_POST(DoubleElement)

XTOOLS_NAMESPACE_END
