//////////////////////////////////////////////////////////////////////////
// FloatElement.h 
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <Public/Element.h>

XTOOLS_NAMESPACE_BEGIN

DECLARE_PTR_PRE(FloatElement)

/// Represents a float element type, and provides functions for getting and setting the value. 
class FloatElement : public Element
{
public:
	/// If the given Element is a FloatElement, cast it to the derived type.  Otherwise return null
	static ref_ptr<FloatElement> Cast(const ElementPtr& element);

	/// Get the current value of the element.  Returns immediately, does not allocate.  
	virtual float GetValue() const = 0;

	/// Sets the value of the element locally and automatically queues up the change to be synced
	/// to all the remote clients.  Returns immediately.  
	virtual void SetValue(float newValue) = 0;
};

DECLARE_PTR_POST(FloatElement)

XTOOLS_NAMESPACE_END
