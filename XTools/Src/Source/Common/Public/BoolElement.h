//////////////////////////////////////////////////////////////////////////
// BoolElement.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <Public/Element.h>

XTOOLS_NAMESPACE_BEGIN

DECLARE_PTR_PRE(BoolElement)

/// Represents a boolean element type, and provides functions for getting and setting the value. 
class BoolElement : public Element
{
public:
	/// If the given Element is a BoolElement, cast it to the derived type.  Otherwise return null
	static ref_ptr<BoolElement> Cast(const ElementPtr& element);

	/// Get the current value of the element.  Returns immediately, does not allocate.  
	virtual bool GetValue() const = 0;

	/// Sets the value of the element locally and automatically queues up the change to be synced
	/// to all the remote clients.  Returns immediately.  
	virtual void SetValue(bool newValue) = 0;
};

DECLARE_PTR_POST(BoolElement)

XTOOLS_NAMESPACE_END
