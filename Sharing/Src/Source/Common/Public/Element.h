//////////////////////////////////////////////////////////////////////////
// Element.h
//
// Base class for all element types.   
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

enum ElementType : byte
{
	UnknownType = 0,
	BoolType,
	Int32Type,
	Int64Type,
	FloatType,
	DoubleType,
	StringType,
	ObjectType,
	Int32ArrayType,
	FloatArrayType,
	StringArrayType,

	ElementTypeCount
};

DECLARE_PTR_PRE(Element)

class Element : public AtomicRefCounted, public Reflection::XTObject
{
	XTOOLS_REFLECTION_DECLARE(Element)

public:
	// Returns an ID defining what type of element this is 
	virtual ElementType GetElementType() const = 0;

	// Returns the unique ID for this element.  
	virtual XGuid GetGUID() const = 0;

	// Returns the text name of the element.  
	virtual const XStringPtr& GetName() const = 0;

	// Returns a pointer to this Element's parent Element.
	// If this is the root element, then it returns NULL
	virtual ref_ptr<Element> GetParent() const = 0;

	// Returns false if the element is no longer a part of the sync data set. 
	// ie: The element has been 'deleted', but the user code is still holding on to a reference so
	// it has not been destroyed
	virtual bool IsValid() const = 0;

	//////////////////////////////////////////////////////////////////////////
	// Not exposed to SWIG

	// Returns a generic holder for the value of this element
	// Set to be ignored by SWIG in Common.i
	virtual XValue GetXValue() const = 0;

	// Set the value of this element from a generic holder for the value
	// Set to be ignored by SWIG in Common.i
	virtual void SetXValue(const XValue& value) = 0;

	// Set the parent of this element
	virtual void SetParent(Element* parent) = 0;
};

DECLARE_PTR_POST(Element)

XTOOLS_NAMESPACE_END
