//////////////////////////////////////////////////////////////////////////
// StringElement.h
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

DECLARE_PTR_PRE(StringElement)

/// Represents a string element type, and provides functions for getting and setting the value.  
class StringElement : public Element
{
public:
	/// If the given Element is a StringElement, cast it to the derived type.  Otherwise return null
	static ref_ptr<StringElement> Cast(const ElementPtr& element);

	/// Get the current value of the element.  Returns immediately.  XString wrapper is allocated, as well as a C++ XString object.  
	/// The string is not passed across the C++ boundary until XString.GetString() is called.  
	virtual const XStringPtr& GetValue() const = 0;

	/// Sets the value of the element locally and automatically queues up the change to be synced
	/// to all the remote clients.  Returns immediately.  The string is not sent across the boundary;
	/// the XString class has a pointer to the string already existing on the C++ side.  
	virtual void SetValue(const XStringPtr& newString) = 0;
};

DECLARE_PTR_POST(StringElement)

XTOOLS_NAMESPACE_END
