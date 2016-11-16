//////////////////////////////////////////////////////////////////////////
// ObjectElementImpl.h
//
// Implements the ObjectElement interface
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

class ObjectElementImpl : public ObjectElement
{
	XTOOLS_REFLECTION_DECLARE(ObjectElementImpl)

public:
	typedef ListenerList<ObjectElementListener> ListenerList;
	DECLARE_PTR(ListenerList);


	ObjectElementImpl(SyncContext* syncContext, const XStringPtr& name, XGuid id, UserID ownerID, const XValue& typeValue);

	// ObjectElement Functions:

	/// Create a BoolElement as a child of this object.  The element will have the given starting value.  
	/// Creating this element will cause the same element to automatically be created on all the remote systems
	virtual BoolElementPtr CreateBoolElement(const XStringPtr& name, bool value) XTOVERRIDE;

	/// Create an IntElement as a child of this object.  The element will have the given starting value.  
	/// Creating this element will cause the same element to automatically be created on all the remote systems
	virtual IntElementPtr CreateIntElement(const XStringPtr& name, int value) XTOVERRIDE;

	/// Create a LongElement as a child of this object.  The element will have the given starting value.  
	/// Creating this element will cause the same element to automatically be created on all the remote systems
	virtual LongElementPtr CreateLongElement(const XStringPtr& name, int64 value) XTOVERRIDE;

	/// Create a FloatElement as a child of this object.  The element will have the given starting value.  
	/// Creating this element will cause the same element to automatically be created on all the remote systems
	virtual FloatElementPtr CreateFloatElement(const XStringPtr& name, float value) XTOVERRIDE;

	/// Create a DoubleElement as a child of this object.  The element will have the given starting value.  
	/// Creating this element will cause the same element to automatically be created on all the remote systems
	virtual DoubleElementPtr CreateDoubleElement(const XStringPtr& name, double value) XTOVERRIDE;

	/// Create a StringElement as a child of this object.  The element will have the given starting value.  
	/// Creating this element will cause the same element to automatically be created on all the remote systems
	virtual StringElementPtr CreateStringElement(const XStringPtr& name, const XStringPtr& value) XTOVERRIDE;

	/// Create an ObjectElement as a child of this object.   
	/// Creating this element will cause the same element to automatically be created on all the remote systems
	virtual ObjectElementPtr CreateObjectElement(const XStringPtr& name, const XStringPtr& objectType, const User* owner) XTOVERRIDE;

	/// Create an IntArrayElement as a child of this object.    
	/// Creating this element will cause the same element to automatically be created on all the remote systems
	virtual IntArrayElementPtr CreateIntArrayElement(const XStringPtr& name) XTOVERRIDE;

	/// Create a FloatArrayElement as a child of this object.    
	/// Creating this element will cause the same element to automatically be created on all the remote systems
	virtual FloatArrayElementPtr CreateFloatArrayElement(const XStringPtr& name) XTOVERRIDE;

	/// Create a StringArrayElement as a child of this object.    
	/// Creating this element will cause the same element to automatically be created on all the remote systems
	virtual StringArrayElementPtr CreateStringArrayElement(const XStringPtr& name) XTOVERRIDE;

	/// Returns the number of elements that are immediate children of this element
	virtual int32 GetElementCount() const XTOVERRIDE;

	/// Get the element by GUID
	virtual ElementPtr GetElement(XGuid id) const XTOVERRIDE;

	/// Get the element by name
	virtual ElementPtr GetElement(const XStringPtr& name) const XTOVERRIDE;

	/// Get the element by index
	virtual ElementPtr GetElementAt(int32 index) const XTOVERRIDE;

	/// Remove the given element, which must be an immediate child of this object.  
	/// The element will also be automatically removed on all remote systems.  
	virtual void RemoveElement(const ElementPtr& element) XTOVERRIDE;

	/// Remove the child element with the given XGuid
	/// The element will also be automatically removed on all remote systems.  
	virtual void RemoveElement(XGuid id) XTOVERRIDE;

	/// Remove the child element at the given index.  
	/// NOTE: the index of child elements will not be the same across all systems
	/// The element will also be automatically removed on all remote systems.  
	virtual void RemoveElementAt(int32 index) XTOVERRIDE;

	/// Register an object to receive notifications when the elements of this object change.  
	/// Multiple listeners can be registered.  The wrapper class will hold a reference to the listener
	/// to ensure it is not garbage collected until this class is destroyed or the listener is removed. 
	/// \param newListener The listener object that will receive callbacks from this SessionManager
	virtual void AddListener(ObjectElementListener* newListener) XTOVERRIDE;

	/// Remove a previously registered listener.  The wrapper class will release its reference to the given listener.  
	/// \param newListener The listener object that will no longer receive callbacks from this SessionManager.  
	virtual void RemoveListener(ObjectElementListener* oldListener) XTOVERRIDE;

	/// Return the ID of the user who 'owns' this element.  Owned elements will be deleted from the 
	/// set of synced data when their owner disconnects
	virtual UserID GetOwnerID() const XTOVERRIDE;

	/// Returns the type of this Object element, ie: the type of the class that this Object Element represents
	/// in the synced data set.  
	virtual const XStringPtr& GetObjectType() const XTOVERRIDE;

	/// Returns an ID defining what type of element this is 
	virtual ElementType GetElementType() const XTOVERRIDE;

	/// Returns the session-wide unique ID for this element.  
	virtual XGuid GetGUID() const XTOVERRIDE;

	/// Returns the text name of the element.  
	virtual const XStringPtr& GetName() const XTOVERRIDE;

	/// Returns a pointer to this Element's parent Element.
	/// If this is the root element, then it returns NULL
	virtual ElementPtr GetParent() const XTOVERRIDE;

	/// Returns false if the element is no longer a part of the sync data set. 
	/// ie: The element has been 'deleted', but the user code is still holding on to a reference so
	/// it has not been destroyed
	virtual bool IsValid() const XTOVERRIDE;

	/// Returns a generic holder for the value of this element
	/// Set to be ignored by SWIG in Common.i
	virtual XValue GetXValue() const XTOVERRIDE;

	/// Set the value of this element from a generic holder for the value
	/// Set to be ignored by SWIG in Common.i
	virtual void SetXValue(const XValue& value) XTOVERRIDE;

	/// Set the parent of this element
	virtual void SetParent(Element* parent) XTOVERRIDE;

	// Local Functions:
	const ListenerListPtr& GetListeners() const;

	void AddChild(const ElementPtr& newElement);
	void RemoveChild(const ElementPtr& newElement);

private:

	ElementPtr CreateElement(ElementType type, const XStringPtr& name, UserID ownerID, const XValue& value);

	SyncContext*						m_syncContext;
	Element*							m_parent;
	std::vector<ElementPtr>				m_children;
	XStringPtr							m_name;
	XGuid								m_guid;
	ListenerListPtr						m_listenerList;
	UserID								m_ownerID;
	XStringPtr							m_objectType;
};

DECLARE_PTR(ObjectElementImpl)

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END