//////////////////////////////////////////////////////////////////////////
// IntArrayElementImpl.cpp
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IntArrayElementImpl.h"

XTOOLS_NAMESPACE_BEGIN

IntArrayElementPtr IntArrayElement::Cast(const ElementPtr& element)
{
	if (element != NULL && element->GetElementType() == ElementType::Int32ArrayType)
	{
		return static_cast<IntArrayElement*>(element.get());
	}
	else
	{
		return NULL;
	}
}

NAMESPACE_BEGIN(Sync)

XTOOLS_REFLECTION_DEFINE(ArrayElement);

XTOOLS_REFLECTION_DEFINE(IntArrayElementImpl)
.BaseClass<Element>()
.BaseClass<ArrayElement>();


IntArrayElementImpl::IntArrayElementImpl(SyncContext* syncContext, const XStringPtr& name, XGuid id, const XValue& startingValue)
	: m_syncContext(syncContext)
	, m_parent(NULL)
	, m_name(name)
	, m_guid(id)
	, m_listenerList(ListenerList::Create())
{
	// The starting value is not used for array creation
	XT_UNREFERENCED_PARAM(startingValue);
}


int32 IntArrayElementImpl::GetCount() const
{
	return static_cast<int32>(m_values.size());
}


int32 IntArrayElementImpl::GetValue(int32 index) const
{
	if (index >= 0 && index < GetCount())
	{
		return m_values[index];
	}
	else
	{
		LogError("GetValue passed invalid index in element %s", m_name->GetString().c_str());
		return -1;
	}
}


void IntArrayElementImpl::SetValue(int32 index, int32 newValue)
{
	if (!IsValid())
	{
		LogError("Attempting to set value on invalid IntArrayElement %s", m_name->GetString().c_str());
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


void IntArrayElementImpl::InsertValue(int32 index, int value)
{
	if (!IsValid())
	{
		LogError("Attempting to set value on invalid IntArrayElement %s", m_name->GetString().c_str());
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


void IntArrayElementImpl::RemoveValue(int32 index)
{
	if (!IsValid())
	{
		LogError("Attempting to set value on invalid IntArrayElement %s", m_name->GetString().c_str());
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


void IntArrayElementImpl::AddListener(IntArrayListener* newListener)
{
	if (!IsValid())
	{
		LogWarning("Attempting to add a listener to invalid element %s", m_name->GetString().c_str());
	}

	m_listenerList->AddListener(newListener);
}


void IntArrayElementImpl::RemoveListener(IntArrayListener* oldListener)
{
	m_listenerList->RemoveListener(oldListener);
}


ElementType IntArrayElementImpl::GetElementType() const
{
	return ElementType::Int32ArrayType;
}


XGuid IntArrayElementImpl::GetGUID() const
{
	return m_guid;
}


const XStringPtr& IntArrayElementImpl::GetName() const
{
	return m_name;
}


ElementPtr IntArrayElementImpl::GetParent() const
{
	return m_parent;
}


bool IntArrayElementImpl::IsValid() const
{
	return (m_syncContext && m_syncContext->ElementExists(m_guid));
}


XValue IntArrayElementImpl::GetXValue() const
{
	return XValue();
}


void IntArrayElementImpl::SetXValue(const XValue& )
{
	XTASSERT(IsValid());

}


void IntArrayElementImpl::SetParent(Element* parent)
{
	m_parent = parent;
}


XValue IntArrayElementImpl::GetXValue(int32 index) const
{
	return GetValue(index);
}


void IntArrayElementImpl::SetXValue(int32 index, const XValue& newValue)
{
	if (index >= 0 && index < GetCount())
	{
		const int32* intValue = newValue.Get<int32>();
		if (intValue != nullptr)
		{
			m_values[index] = *intValue;
		}
	}
	else
	{
		LogError("SetXValue passed invalid index in element %s", m_name->GetString().c_str());
	}
}


void IntArrayElementImpl::InsertXValue(int32 index, const XValue& newValue)
{
	if (index >= 0 && index <= GetCount())
	{
		const int32* intValue = newValue.Get<int32>();
		if (intValue != nullptr)
		{
			m_values.insert(m_values.begin() + index, *intValue);
		}
	}
	else
	{
		LogError("InsertXValue passed invalid index in element %s", m_name->GetString().c_str());
	}
}


void IntArrayElementImpl::RemoveXValue(int32 index)
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


void IntArrayElementImpl::NotifyUpdated(int32 index, const XValue& newValue)
{
	m_listenerList->NotifyListeners(&IntArrayListener::OnValueChanged, index, *(newValue.Get<int32>()));
}


void IntArrayElementImpl::NotifyInserted(int32 index, const XValue& newValue)
{
	m_listenerList->NotifyListeners(&IntArrayListener::OnValueInserted, index, *(newValue.Get<int32>()));
}


void IntArrayElementImpl::NotifyRemoved(int32 index, const XValue& value)
{
	m_listenerList->NotifyListeners(&IntArrayListener::OnValueRemoved, index, *(value.Get<int32>()));
}


NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
