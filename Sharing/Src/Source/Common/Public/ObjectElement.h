//////////////////////////////////////////////////////////////////////////
// ObjectElement.h
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

DECLARE_PTR_PRE(ObjectElement)

/// Represents an Object element type, and provides functions for getting 
/// the elements that it contains.  To add elements use the ElementFactory's
/// functions and pass a pointer to one of these as the parent.  
class ObjectElement : public Element
{
	XTOOLS_REFLECTION_DECLARE(ObjectElement)

public:
	/// If the given element is an Object, cast it to the derived type.  Otherwise return null
	static ref_ptr<ObjectElement> Cast(const ElementPtr& element);

	/// Create a BoolElement as a child of this object.  The element will have the given starting value.  
	/// Creating this element will cause the same element to automatically be created on all the remote systems
	virtual BoolElementPtr CreateBoolElement(const XStringPtr& name, bool value) = 0;

	/// Create an IntElement as a child of this object.  The element will have the given starting value.  
	/// Creating this element will cause the same element to automatically be created on all the remote systems
	virtual IntElementPtr CreateIntElement(const XStringPtr& name, int32 value) = 0;

	/// Create a LongElement as a child of this object.  The element will have the given starting value.  
	/// Creating this element will cause the same element to automatically be created on all the remote systems
	virtual LongElementPtr CreateLongElement(const XStringPtr& name, int64 value) = 0;

	/// Create a FloatElement as a child of this object.  The element will have the given starting value.  
	/// Creating this element will cause the same element to automatically be created on all the remote systems
	virtual FloatElementPtr CreateFloatElement(const XStringPtr& name, float value) = 0;

	/// Create a DoubleElement as a child of this object.  The element will have the given starting value.  
	/// Creating this element will cause the same element to automatically be created on all the remote systems
	virtual DoubleElementPtr CreateDoubleElement(const XStringPtr& name, double value) = 0;

	/// Create a StringElement as a child of this object.  The element will have the given starting value.  
	/// Creating this element will cause the same element to automatically be created on all the remote systems
	virtual StringElementPtr CreateStringElement(const XStringPtr& name, const XStringPtr& value) = 0;

	/// Create an ObjectElement as a child of this object.   
	/// Creating this element will cause the same element to automatically be created on all the remote systems.
	/// Set the User parameter to your local User ID to have this object removed from remote systems when you disconnect
	virtual ref_ptr<ObjectElement> CreateObjectElement(const XStringPtr& name, const XStringPtr& objectType, const User* owner = NULL) = 0;

	/// Create an IntArrayElement as a child of this object.    
	/// Creating this element will cause the same element to automatically be created on all the remote systems
	virtual IntArrayElementPtr CreateIntArrayElement(const XStringPtr& name) = 0;

	/// Create a FloatArrayElement as a child of this object.    
	/// Creating this element will cause the same element to automatically be created on all the remote systems
	virtual FloatArrayElementPtr CreateFloatArrayElement(const XStringPtr& name) = 0;

	/// Create a StringArrayElement as a child of this object.    
	/// Creating this element will cause the same element to automatically be created on all the remote systems
	virtual StringArrayElementPtr CreateStringArrayElement(const XStringPtr& name) = 0;

	/// Returns the number of elements that are immediate children of this element
	virtual int32 GetElementCount() const = 0;

	/// Get the element by GUID
	virtual ElementPtr GetElement(XGuid id) const = 0;

	/// Get the element by name
	virtual ElementPtr GetElement(const XStringPtr& name) const = 0;

	/// Get the element by index
	virtual ElementPtr GetElementAt(int32 index) const = 0;

	/// Remove the given element, which must be an immediate child of this object.  
	/// The element will also be automatically removed on all remote systems.  
	virtual void RemoveElement(const ElementPtr& element) = 0;

	/// Remove the child element with the given XGuid
	/// The element will also be automatically removed on all remote systems.  
	virtual void RemoveElement(XGuid id) = 0;

	/// Remove the child element at the given index.  
	/// NOTE: the index of child elements will not be the same across all systems
	/// The element will also be automatically removed on all remote systems.  
	virtual void RemoveElementAt(int32 index) = 0;

	/// Register an object to receive notifications when the elements of this object change.  
	/// Multiple listeners can be registered.  The wrapper class will hold a reference to the listener
	/// to ensure it is not garbage collected until this class is destroyed or the listener is removed. 
	/// \param newListener The listener object that will receive callbacks 
	virtual void AddListener(ObjectElementListener* newListener) = 0;

	/// Remove a previously registered listener.  The wrapper class will release its reference to the given listener.  
	/// \param oldListener The listener object that will no longer receive callbacks  
	virtual void RemoveListener(ObjectElementListener* oldListener) = 0;

	/// Return the ID of the user who 'owns' this element.  Owned elements will be deleted from the 
	/// set of synced data when their owner disconnects
	virtual UserID GetOwnerID() const = 0;

	/// Returns the type of this Object element, ie: the type of the class that this Object Element represents
	/// in the synced data set.  
	virtual const XStringPtr& GetObjectType() const = 0;
};

DECLARE_PTR_POST(ObjectElement)

XTOOLS_NAMESPACE_END
