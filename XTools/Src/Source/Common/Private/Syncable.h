//////////////////////////////////////////////////////////////////////////
// Syncable.h
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

// Base class for types that can by synchronized with the XTools sync system
class Syncable XTABSTRACT : public Reflection::XTObject
{
	XTOOLS_REFLECTION_DECLARE(Syncable)
		
public:
	virtual ~Syncable() {}

	virtual XGuid GetGUID() const = 0;

	virtual ElementType GetType() const = 0;

	virtual ElementPtr GetElement() const = 0;

	// Create a new element for this instance in the sync system
	virtual bool BindLocal(const ObjectElementPtr& parent, const std::string& name, const UserPtr& owner) = 0;

	// Bind this instance to an element that already exists in the sync system
	virtual void BindRemote(const ElementPtr& element) = 0;

	// Remove the binding information from this instance
	virtual void Unbind() = 0;

protected:
	Syncable() {}

	// Set the value of this instance in response to the value being changed remotely
	virtual void SetValue(const XValue& newValue) = 0;

	friend class SyncObject;
};

XTOOLS_NAMESPACE_END
