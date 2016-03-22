//////////////////////////////////////////////////////////////////////////
// IntElement.h
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <Public/Element.h>

XTOOLS_NAMESPACE_BEGIN

DECLARE_PTR_PRE(IntElement)

/// Represents a signed 32-bit integer element type, and provides functions for getting and setting the value.
class IntElement : public Element
{
public:
	/// If the given Element is an IntElement, cast it to the derived type.  Otherwise return null
	static ref_ptr<IntElement> Cast(const ElementPtr& element);

	/// Get the current value of the element.  Returns immediately, does not allocate.  
	virtual int32 GetValue() const = 0;

	/// Sets the value of the element locally and automatically queues up the change to be synced
	/// to all the remote clients.  Returns immediately.  
	virtual void SetValue(int32 newValue) = 0;
};

DECLARE_PTR_POST(IntElement)

XTOOLS_NAMESPACE_END
