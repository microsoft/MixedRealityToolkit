//////////////////////////////////////////////////////////////////////////
// FloatArrayElementImpl.cpp
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "FloatArrayElementImpl.h"

XTOOLS_NAMESPACE_BEGIN

FloatArrayElementPtr FloatArrayElement::Cast(const ElementPtr& element)
{
	if (element != NULL && element->GetElementType() == ElementType::FloatArrayType)
	{
		return static_cast<FloatArrayElement*>(element.get());
	}
	else
	{
		return NULL;
	}
}

NAMESPACE_BEGIN(Sync)

XTOOLS_REFLECTION_DEFINE(FloatArrayElementImpl)
.BaseClass<Element>()
.BaseClass<ArrayElement>();


FloatArrayElementImpl::FloatArrayElementImpl(SyncContext* syncContext, const XStringPtr& name, XGuid id, const XValue& startingValue)
	: m_syncContext(syncContext)
	, m_parent(NULL)
	, m_name(name)
	, m_guid(id)
	, m_listenerList(ListenerList::Create())
{
	// The starting value is not used for array creation
	XT_UNREFERENCED_PARAM(startingValue);
}


int32 FloatArrayElementImpl::GetCount() const
{
	return static_cast<int32>(m_values.size());
}


float FloatArrayElementImpl::GetValue(int32 index) const
{
	if (index >= 0 && index < GetCount())
	{
		return m_values[index];
	}
	else
	{
		LogError("GetValue passed invalid index in element %s", m_name->GetString().c_str());
		return -1.0f;
	}
}


void FloatArrayElementImpl::SetValue(int32 index, float newValue)
{
	if (!IsValid())
	{
		LogError("Attempting to set value on invalid FloatArrayElement %s", m_name->GetString().c_str());
	}
	else
	{
		if (index >= 0 && index < GetCount())
		{
			m_values[index] = newValue;

			// Create an Operation to represent this change
			UpdateOperationPtr updateOp = new UpdateOperation(m_guid, index, newValue, m_syncContext->GetAuthorityLevel(), m_syncContext);

			// Add the op to the list of ops applied since the last sync
			m_syncContext->AddAppliedOperation(updateOp);
		}
		else
		{
			LogError("SetValue passed invalid index in element %s", m_name->GetString().c_str());
		}
	}
}


void FloatArrayElementImpl::InsertValue(int32 index, float value)
{
	if (!IsValid())
	{
		LogError("Attempting to set value on invalid FloatArrayElement %s", m_name->GetString().c_str());
	}
	else
	{
		if (index >= 0 && index <= GetCount())
		{
			m_values.insert(m_values.begin() + index, value);

			// Create an Operation to represent this change
			InsertOperationPtr insertOp = new InsertOperation(m_guid, index, value, m_syncContext->GetAuthorityLevel(), m_syncContext);

			// Add the op to the list of ops applied since the last sync
			m_syncContext->AddAppliedOperation(insertOp);
		}
		else
		{
			LogError("InsertValue passed invalid index in element %s", m_name->GetString().c_str());
		}
	}
}


void FloatArrayElementImpl::RemoveValue(int32 index)
{
	if (!IsValid())
	{
		LogError("Attempting to set value on invalid FloatArrayElement %s", m_name->GetString().c_str());
	}
	else
	{
		if (index >= 0 && index < GetCount())
		{
			m_values.erase(m_values.begin() + index);

			// Create an Operation to represent this change
			RemoveOperationPtr removeOp = new RemoveOperation(m_guid, index, m_syncContext->GetAuthorityLevel(), m_syncContext);

			// Add the op to the list of ops applied since the last sync
			m_syncContext->AddAppliedOperation(removeOp);
		}
		else
		{
			LogError("RemoveValue passed invalid index in element %s", m_name->GetString().c_str());
		}
	}
}


void FloatArrayElementImpl::AddListener(FloatArrayListener* newListener)
{
	if (!IsValid())
	{
		LogWarning("Attempting to add a listener to invalid element %s", m_name->GetString().c_str());
	}

	m_listenerList->AddListener(newListener);
}


void FloatArrayElementImpl::RemoveListener(FloatArrayListener* oldListener)
{
	m_listenerList->RemoveListener(oldListener);
}


ElementType FloatArrayElementImpl::GetElementType() const
{
	return ElementType::FloatArrayType;
}


XGuid FloatArrayElementImpl::GetGUID() const
{
	return m_guid;
}


const XStringPtr& FloatArrayElementImpl::GetName() const
{
	return m_name;
}


ElementPtr FloatArrayElementImpl::GetParent() const
{
	return m_parent;
}


bool FloatArrayElementImpl::IsValid() const
{
	return (m_syncContext && m_syncContext->ElementExists(m_guid));
}


XValue FloatArrayElementImpl::GetXValue() const
{
	return XValue();
}


void FloatArrayElementImpl::SetXValue(const XValue&)
{
	XTASSERT(IsValid());

}


void FloatArrayElementImpl::SetParent(Element* parent)
{
	m_parent = parent;
}


XValue FloatArrayElementImpl::GetXValue(int32 index) const
{
	return GetValue(index);
}


void FloatArrayElementImpl::SetXValue(int32 index, const XValue& newValue)
{
	if (index >= 0 && index < GetCount())
	{
		const float* floatValue = newValue.Get<float>();
		if (floatValue != nullptr)
		{
			m_values[index] = *floatValue;
		}
	}
	else
	{
		LogError("SetXValue passed invalid index in element %s", m_name->GetString().c_str());
	}
}


void FloatArrayElementImpl::InsertXValue(int32 index, const XValue& newValue)
{
	if (index >= 0 && index <= GetCount())
	{
		const float* floatValue = newValue.Get<float>();
		if (floatValue != nullptr)
		{
			m_values.insert(m_values.begin() + index, *floatValue);
		}
	}
	else
	{
		LogError("InsertXValue passed invalid index in element %s", m_name->GetString().c_str());
	}
}


void FloatArrayElementImpl::RemoveXValue(int32 index)
{
	if (index >= 0 && index < GetCount())
	{
		m_values.erase(m_values.begin() + index);
	}
	else
	{
		LogError("RemoveXValue passed invalid index in element %s", m_name->GetString().c_str());
	}
}


void FloatArrayElementImpl::NotifyUpdated(int32 index, const XValue& newValue)
{
	m_listenerList->NotifyListeners(&FloatArrayListener::OnValueChanged, index, *(newValue.Get<float>()));
}


void FloatArrayElementImpl::NotifyInserted(int32 index, const XValue& newValue)
{
	m_listenerList->NotifyListeners(&FloatArrayListener::OnValueInserted, index, *(newValue.Get<float>()));
}


void FloatArrayElementImpl::NotifyRemoved(int32 index, const XValue& value)
{
	m_listenerList->NotifyListeners(&FloatArrayListener::OnValueRemoved, index, *(value.Get<float>()));
}


NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
