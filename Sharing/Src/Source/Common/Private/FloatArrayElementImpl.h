//////////////////////////////////////////////////////////////////////////
// FloatArrayElementImpl.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

/// Represents a array of signed 32-bit floats
class FloatArrayElementImpl : public FloatArrayElement, public ArrayElement
{
	XTOOLS_REFLECTION_DECLARE(FloatArrayElementImpl)

public:
	typedef ListenerList<FloatArrayListener> ListenerList;
	DECLARE_PTR(ListenerList);

	FloatArrayElementImpl(SyncContext* syncContext, const XStringPtr& name, XGuid id, const XValue& startingValue);

	/// Returns the number of elements in the array
	virtual int32 GetCount() const XTOVERRIDE;

	/// Get the current value of the element at the given index.  Returns immediately, does not allocate.  
	virtual float GetValue(int32 index) const XTOVERRIDE;

	/// Sets the value of the element at the given index.  Returns immediately, does not allocate
	virtual void SetValue(int32 index, float newValue) XTOVERRIDE;

	/// Inserts the given value into the array at the given index
	virtual void InsertValue(int32 index, float value) XTOVERRIDE;

	/// Remove the element at the given index from the array
	virtual void RemoveValue(int32 index) XTOVERRIDE;

	/// Register an object to receive notifications when the elements of this object change.  
	/// Multiple listeners can be registered.  The wrapper class will hold a reference to the listener
	/// to ensure it is not garbage collected until this class is destroyed or the listener is removed. 
	/// \param newListener The listener object that will receive callbacks
	virtual void AddListener(FloatArrayListener* newListener) XTOVERRIDE;

	/// Remove a previously registered listener.  The wrapper class will release its reference to the given listener.  
	/// \param newListener The listener object that will no longer receive callbacks
	virtual void RemoveListener(FloatArrayListener* oldListener) XTOVERRIDE;

	// Returns an ID defining what type of element this is 
	virtual ElementType GetElementType() const XTOVERRIDE;

	// Returns the session-wide unique ID for this element.  
	virtual XGuid GetGUID() const XTOVERRIDE;

	// Returns the text name of the element.  
	virtual const XStringPtr& GetName() const XTOVERRIDE;

	// Returns a pointer to this Element's parent Element.
	// If this is the root element, then it returns NULL
	virtual ElementPtr GetParent() const XTOVERRIDE;

	// Returns false if the element is no longer a part of the sync data set. 
	// ie: The element has been 'deleted', but the user code is still holding on to a reference so
	// it has not been destroyed
	virtual bool IsValid() const XTOVERRIDE;

	// Returns a generic holder for the value of this element
	// Set to be ignored by SWIG in Common.i
	virtual XValue GetXValue() const XTOVERRIDE;

	// Set the value of this element from a generic holder for the value
	// Set to be ignored by SWIG in Common.i
	virtual void SetXValue(const XValue& value) XTOVERRIDE;

	// Set the parent of this element
	virtual void SetParent(Element* parent) XTOVERRIDE;

	//////////////////////////////////////////////////////////////////////////
	// ArrayElement Functions
	virtual XValue GetXValue(int32 index) const XTOVERRIDE;

	virtual void SetXValue(int32 index, const XValue& newValue) XTOVERRIDE;

	virtual void InsertXValue(int32 index, const XValue& newValue) XTOVERRIDE;

	virtual void RemoveXValue(int32 index) XTOVERRIDE;

	virtual void NotifyUpdated(int32 index, const XValue& newValue) XTOVERRIDE;

	virtual void NotifyInserted(int32 index, const XValue& newValue) XTOVERRIDE;

	virtual void NotifyRemoved(int32 index, const XValue& value) XTOVERRIDE;

private:
	SyncContext*		m_syncContext;
	Element*			m_parent;
	XStringPtr			m_name;
	XGuid				m_guid;
	std::vector<float>	m_values;
	ListenerListPtr		m_listenerList;
};

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
