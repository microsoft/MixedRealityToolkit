//////////////////////////////////////////////////////////////////////////
// LongElementImpl.cpp
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LongElementImpl.h"
#include "ModifyOperation.h"

XTOOLS_NAMESPACE_BEGIN

LongElementPtr LongElement::Cast(const ElementPtr& element)
{
	if (element != NULL && element->GetElementType() == ElementType::Int64Type)
	{
		return static_cast<LongElement*>(element.get());
	}
	else
	{
		return NULL;
	}
}


NAMESPACE_BEGIN(Sync)

LongElementImpl::LongElementImpl(SyncContext* context, const XStringPtr& name, XGuid id, const XValue& startingValue)
	: m_syncContext(context)
	, m_name(name)
	, m_guid(id)
	, m_value(*(startingValue.Get<int64>()))
{
	XTASSERT(name);
}


int64 LongElementImpl::GetValue() const
{
	return m_value;
}


void LongElementImpl::SetValue(int64 newValue)
{
	if (!IsValid())
	{
		LogError("Attempting to set value on invalid LongElement %s", m_name->GetString().c_str());
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


ElementType LongElementImpl::GetElementType() const
{
	return ElementType::Int64Type;
}


XGuid LongElementImpl::GetGUID() const
{
	return m_guid;
}


const XStringPtr& LongElementImpl::GetName() const
{
	return m_name;
}


// Returns a pointer to this Element's parent Element.
// If this is the root element, then it returns NULL
ElementPtr LongElementImpl::GetParent() const
{
	return m_parent;
}


bool LongElementImpl::IsValid() const
{
	return (m_syncContext && m_syncContext->ElementExists(m_guid));
}


XValue LongElementImpl::GetXValue() const
{
	return m_value;
}


void LongElementImpl::SetXValue(const XValue& value)
{
	XTASSERT(value.GetType() == XValue::Int64);
	m_value = *value.Get<int64>();
}


void LongElementImpl::SetParent(Element* parent)
{
	m_parent = parent;
}


void LongElementImpl::SetLocal(int64 newValue)
{
	m_value = newValue;
}

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
