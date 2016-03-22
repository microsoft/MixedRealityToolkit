//////////////////////////////////////////////////////////////////////////
// FloatElementImpl.cpp
//
// 
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FloatElementImpl.h"
#include "ModifyOperation.h"

XTOOLS_NAMESPACE_BEGIN

FloatElementPtr FloatElement::Cast(const ElementPtr& element)
{
	if (element != NULL && element->GetElementType() == ElementType::FloatType)
	{
		return static_cast<FloatElement*>(element.get());
	}
	else
	{
		return NULL;
	}
}


NAMESPACE_BEGIN(Sync)

FloatElementImpl::FloatElementImpl(SyncContext* context, const XStringPtr& name, XGuid id, const XValue& startingValue)
	: m_syncContext(context)
	, m_name(name)
	, m_guid(id)
	, m_value(*(startingValue.Get<float>()))
{
	XTASSERT(name);
}


float FloatElementImpl::GetValue() const
{
	return m_value;
}


void FloatElementImpl::SetValue(float newValue)
{
	if (!IsValid())
	{
		LogError("Attempting to set value on invalid FloatElement %s", m_name->GetString().c_str());
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


ElementType FloatElementImpl::GetElementType() const
{
	return ElementType::FloatType;
}


XGuid FloatElementImpl::GetGUID() const
{
	return m_guid;
}


const XStringPtr& FloatElementImpl::GetName() const
{
	return m_name;
}


// Returns a pointer to this Element's parent Element.
// If this is the root element, then it returns NULL
ElementPtr FloatElementImpl::GetParent() const
{
	return m_parent;
}


bool FloatElementImpl::IsValid() const
{
	return (m_syncContext && m_syncContext->ElementExists(m_guid));
}


XValue FloatElementImpl::GetXValue() const
{
	return m_value;
}


void FloatElementImpl::SetXValue(const XValue& value)
{
	XTASSERT(value.GetType() == XValue::Float);
	m_value = *value.Get<float>();
}


void FloatElementImpl::SetParent(Element* parent)
{
	m_parent = parent;
}


void FloatElementImpl::SetLocal(float newValue)
{
	m_value = newValue;
}

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END