//////////////////////////////////////////////////////////////////////////
// ObjectElementListener.h
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

// Disable unused parameter warnings.  Pure virtual functions don't play nice with SWIG, but we still want 
// to see the names of the parameters to make reading the code clearer
#pragma warning( push )
#pragma warning( disable : 4100 ) 

XTOOLS_NAMESPACE_BEGIN

XT_LISTENER_DECLARE(ObjectElementListener)

/// Inherit from this class to receive callbacks from Object Elements
/// NOTE: None of the callbacks here are triggered when elements are changed locally; they 
/// are only called when remote changes happen
class ObjectElementListener XTABSTRACT : public Listener
{
public:
	virtual ~ObjectElementListener() {}

	/// Called when the value of an element has changed.  
	/// Pass in the new values in the callback to try to avoid extra allocations of Element wrappers
	virtual void OnBoolElementChanged(XGuid elementID, bool newValue) {}
	virtual void OnIntElementChanged(XGuid elementID, int32 newValue) {}
	virtual void OnLongElementChanged(XGuid elementID, int64 newValue) {}
	virtual void OnFloatElementChanged(XGuid elementID, float newValue) {}
	virtual void OnDoubleElementChanged(XGuid elementID, double newValue) {}
	virtual void OnStringElementChanged(XGuid elementID, const XStringPtr& newValue) {}

	/// Called when a new element of any type is added to the ObjectElement this instance is 
	/// registered with.  
	virtual void OnElementAdded(const ElementPtr& element) {}

	/// Called when an element of any type is removed from the ObjectElement this instance is 
	/// registered with.  
	virtual void OnElementDeleted(const ElementPtr& element) {}
};

XTOOLS_NAMESPACE_END

#pragma warning( pop )