//////////////////////////////////////////////////////////////////////////
// BoolElementImpl.cpp
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BoolElementImpl.h"
#include "ModifyOperation.h"

XTOOLS_NAMESPACE_BEGIN

BoolElementPtr BoolElement::Cast(const ElementPtr& element)
{
	if (element != NULL && element->GetElementType() == ElementType::BoolType)
	{
		return static_cast<BoolElement*>(element.get());
	}
	else
	{
		return NULL;
	}
}


NAMESPACE_BEGIN(Sync)

BoolElementImpl::BoolElementImpl(SyncContext* context, const XStringPtr& name, XGuid id, const XValue& startingValue)
	: m_syncContext(context)
	, m_name(name)
	, m_guid(id)
	, m_value(*(startingValue.Get<bool>()))
{
	XTASSERT(name);
}


bool BoolElementImpl::GetValue() const
{
	return m_value;
}


void BoolElementImpl::SetValue(bool newValue)
{
	if (!IsValid())
	{
		LogError("Attempting to set value on invalid BoolElement %s", m_name->GetString().c_str());
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


ElementType BoolElementImpl::GetElementType() const
{
	return ElementType::BoolType;
}


XGuid BoolElementImpl::GetGUID() const
{
	return m_guid;
}


const XStringPtr& BoolElementImpl::GetName() const
{
	return m_name;
}


// Returns a pointer to this Element's parent Element.
// If this is the root element, then it returns NULL
ElementPtr BoolElementImpl::GetParent() const
{
	return m_parent;
}


bool BoolElementImpl::IsValid() const
{
	return (m_syncContext && m_syncContext->ElementExists(m_guid));
}


XValue BoolElementImpl::GetXValue() const
{
	return m_value;
}


void BoolElementImpl::SetXValue(const XValue& value)
{
	XTASSERT(value.GetType() == XValue::Bool);
	m_value = *value.Get<bool>();
}


void BoolElementImpl::SetParent(Element* parent)
{
	m_parent = parent;
}


void BoolElementImpl::SetLocal(bool newValue)
{
	m_value = newValue;
}

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END