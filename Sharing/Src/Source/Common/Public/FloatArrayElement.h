//////////////////////////////////////////////////////////////////////////
// FloatArrayElement.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <Public/Element.h>

XTOOLS_NAMESPACE_BEGIN

DECLARE_PTR_PRE(FloatArrayElement)

/// Represents a array of signed 32-bit floating point values
class FloatArrayElement : public Element
{
public:
	/// If the given Element is an FloatArrayElement, cast it to the derived type.  Otherwise return null
	static ref_ptr<FloatArrayElement> Cast(const ElementPtr& element);

	/// Returns the number of elements in the array
	virtual int32 GetCount() const = 0;

	/// Get the current value of the element at the given index.  Returns immediately, does not allocate.  
	virtual float GetValue(int32 index) const = 0;

	/// Sets the value of the element at the given index.  Returns immediately, does not allocate
	virtual void SetValue(int32 index, float newValue) = 0;

	/// Inserts the given value into the array at the given index
	virtual void InsertValue(int32 index, float value) = 0;

	/// Remove the element at the given index from the array
	virtual void RemoveValue(int32 index) = 0;

	/// Register an object to receive notifications when the elements of this object change.  
	/// Multiple listeners can be registered.  The wrapper class will hold a reference to the listener
	/// to ensure it is not garbage collected until this class is destroyed or the listener is removed. 
	/// \param newListener The listener object that will receive callbacks
	virtual void AddListener(FloatArrayListener* newListener) = 0;

	/// Remove a previously registered listener.  The wrapper class will release its reference to the given listener.  
	/// \param oldListener The listener object that will no longer receive callbacks
	virtual void RemoveListener(FloatArrayListener* oldListener) = 0;
};

DECLARE_PTR_POST(FloatArrayElement)

XTOOLS_NAMESPACE_END