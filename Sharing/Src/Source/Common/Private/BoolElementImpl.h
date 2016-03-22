//////////////////////////////////////////////////////////////////////////
// BoolElementImpl.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once


XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

class BoolElementImpl : public BoolElement
{
public:
	BoolElementImpl(SyncContext* syncContext, const XStringPtr& name, XGuid id, const XValue& startingValue);

	// BoolElement Functions:

	// Get the current value of the element.  Returns immediately, does not allocate.  
	virtual bool GetValue() const XTOVERRIDE;

	// Sets the value of the element locally and automatically queues up the change to be synced
	// to all the remote clients.  Returns immediately.  
	virtual void SetValue(bool newValue) XTOVERRIDE;

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

	// Local Functions:
	void SetLocal(bool newValue);

private:
	SyncContext*		m_syncContext;
	Element*			m_parent;
	XStringPtr			m_name;
	XGuid				m_guid;
	bool				m_value;
};

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
