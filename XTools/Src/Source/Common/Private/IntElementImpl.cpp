//////////////////////////////////////////////////////////////////////////
// IntElementImpl.cpp
//
// 
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IntElementImpl.h"
#include "ModifyOperation.h"

XTOOLS_NAMESPACE_BEGIN

IntElementPtr IntElement::Cast(const ElementPtr& element)
{
	if (element != NULL && element->GetElementType() == ElementType::Int32Type)
	{
		return static_cast<IntElement*>(element.get());
	}
	else
	{
		return NULL;
	}
}

NAMESPACE_BEGIN(Sync)

IntElementImpl::IntElementImpl(SyncContext* context, const XStringPtr& name, XGuid id, const XValue& startingValue)
	: m_syncContext(context)
	, m_name(name)
	, m_guid(id)
	, m_value(*(startingValue.Get<int32>()))
{
	XTASSERT(name);
}


int32 IntElementImpl::GetValue() const
{
	return m_value;
}


void IntElementImpl::SetValue(int32 newValue)
{
	if (!IsValid())
	{
		LogError("Attempting to set value on invalid IntElement %s", m_name->GetString().c_str());
	}
	else
	{
		m_value = newValue;

		// Create an Operation to represent this change
		ModifyOperationPtr modifyOp = new ModifyOperation(m_guid, newValue, m_syncContext->GetAuthorityLevel(), m_syncContext);

		// Add the op to the list of ops applied since the last sync
		m_syncContext->AddAppliedOperation(modifyOp);
	}
}


ElementType IntElementImpl::GetElementType() const
{
	return ElementType::Int32Type;
}


XGuid IntElementImpl::GetGUID() const
{
	return m_guid;
}


// Returns the text name of the element.  
const XStringPtr& IntElementImpl::GetName() const
{
	return m_name;
}


// Returns a pointer to this Element's parent Element.
// If this is the root element, then it returns NULL
ElementPtr IntElementImpl::GetParent() const
{
	return m_parent;
}


bool IntElementImpl::IsValid() const
{
	return (m_syncContext && m_syncContext->ElementExists(m_guid));
}


XValue IntElementImpl::GetXValue() const
{
	return m_value;
}


void IntElementImpl::SetXValue(const XValue& value)
{
	XTASSERT(value.GetType() == XValue::Int);
	m_value = *value.Get<int32>();
}


// Set the parent of this element
void IntElementImpl::SetParent(Element* parent)
{
	m_parent = parent;
}


void IntElementImpl::SetLocal(int32 newValue)
{
	m_value = newValue;
}

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
