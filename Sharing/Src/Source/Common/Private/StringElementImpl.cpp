//////////////////////////////////////////////////////////////////////////
// StringElementImpl.cpp
//
// Implementation of the StringElement interface
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StringElementImpl.h"
#include "ModifyOperation.h"

XTOOLS_NAMESPACE_BEGIN

StringElementPtr StringElement::Cast(const ElementPtr& element)
{
	if (element != NULL && element->GetElementType() == ElementType::StringType)
	{
		return static_cast<StringElement*>(element.get());
	}
	else
	{
		return NULL;
	}
}


NAMESPACE_BEGIN(Sync)

StringElementImpl::StringElementImpl(SyncContext* context, const XStringPtr& name, XGuid id, const XValue& startingValue)
	: m_syncContext(context)
	, m_name(name)
	, m_guid(id)
	, m_value(new XString(*(startingValue.Get<std::string>())))
{
	XTASSERT(name);
}


const XStringPtr& StringElementImpl::GetValue() const
{
	return m_value;
}


void StringElementImpl::SetValue(const XStringPtr& newValue)
{
	if (!IsValid())
	{
		LogError("Attempting to set value on invalid StringElement %s", m_name->GetString().c_str());
	}
	else if (!newValue)
	{
		LogError("Null newValue passed to StringElement::SetValue");
	}
	else
	{
		m_value = newValue;

		// Create an Operation to represent this change
		ModifyOperationPtr modifyOp = new ModifyOperation(m_guid, newValue->GetString(), m_syncContext->GetAuthorityLevel(), m_syncContext);

		// Add the op to the list of ops applied since the last sync
		m_syncContext->AddAppliedOperation(modifyOp);
	}
}


ElementType StringElementImpl::GetElementType() const
{
	return ElementType::StringType;
}


XGuid StringElementImpl::GetGUID() const
{
	return m_guid;
}


// Returns the text name of the element.  
const XStringPtr& StringElementImpl::GetName() const
{
	return m_name;
}


// Returns a pointer to this Element's parent Element.
// If this is the root element, then it returns NULL
ElementPtr StringElementImpl::GetParent() const
{
	return m_parent;
}


bool StringElementImpl::IsValid() const
{
	return (m_syncContext && m_syncContext->ElementExists(m_guid));
}


XValue StringElementImpl::GetXValue() const
{
	return m_value->GetString();
}


void StringElementImpl::SetXValue(const XValue& value)
{
	XTASSERT(value.GetType() == XValue::String);
	m_value = new XString(*value.Get<std::string>());
}


void StringElementImpl::SetParent(Element* parent)
{
	m_parent = parent;
}


void StringElementImpl::SetLocal(const XStringPtr& newValue)
{
	XTASSERT(newValue);
	m_value = newValue;
}

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END