//////////////////////////////////////////////////////////////////////////
// DoubleElementImpl.cpp
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DoubleElementImpl.h"
#include "ModifyOperation.h"

XTOOLS_NAMESPACE_BEGIN

DoubleElementPtr DoubleElement::Cast(const ElementPtr& element)
{
	if (element != NULL && element->GetElementType() == ElementType::DoubleType)
	{
		return static_cast<DoubleElement*>(element.get());
	}
	else
	{
		return NULL;
	}
}


NAMESPACE_BEGIN(Sync)

DoubleElementImpl::DoubleElementImpl(SyncContext* context, const XStringPtr& name, XGuid id, const XValue& startingValue)
	: m_syncContext(context)
	, m_name(name)
	, m_guid(id)
	, m_value(*(startingValue.Get<double>()))
{
	XTASSERT(name);
}


double DoubleElementImpl::GetValue() const
{
	return m_value;
}


void DoubleElementImpl::SetValue(double newValue)
{
	if (!IsValid())
	{
		LogError("Attempting to set value on invalid DoubleElement %s", m_name->GetString().c_str());
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


ElementType DoubleElementImpl::GetElementType() const
{
	return ElementType::DoubleType;
}


XGuid DoubleElementImpl::GetGUID() const
{
	return m_guid;
}


const XStringPtr& DoubleElementImpl::GetName() const
{
	return m_name;
}


// Returns a pointer to this Element's parent Element.
// If this is the root element, then it returns NULL
ElementPtr DoubleElementImpl::GetParent() const
{
	return m_parent;
}


bool DoubleElementImpl::IsValid() const
{
	return (m_syncContext && m_syncContext->ElementExists(m_guid));
}


XValue DoubleElementImpl::GetXValue() const
{
	return m_value;
}


void DoubleElementImpl::SetXValue(const XValue& value)
{
	XTASSERT(value.GetType() == XValue::Double);
	m_value = *value.Get<double>();
}


void DoubleElementImpl::SetParent(Element* parent)
{
	m_parent = parent;
}


void DoubleElementImpl::SetLocal(double newValue)
{
	m_value = newValue;
}

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END

