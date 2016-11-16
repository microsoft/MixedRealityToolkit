//////////////////////////////////////////////////////////////////////////
// StringArrayElementImpl.cpp
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "StringArrayElementImpl.h"

XTOOLS_NAMESPACE_BEGIN

StringArrayElementPtr StringArrayElement::Cast(const ElementPtr& element)
{
	if (element != NULL && element->GetElementType() == ElementType::StringArrayType)
	{
		return static_cast<StringArrayElement*>(element.get());
	}
	else
	{
		return NULL;
	}
}

NAMESPACE_BEGIN(Sync)

XTOOLS_REFLECTION_DEFINE(StringArrayElementImpl)
.BaseClass<Element>()
.BaseClass<ArrayElement>();


StringArrayElementImpl::StringArrayElementImpl(SyncContext* syncContext, const XStringPtr& name, XGuid id, const XValue& startingValue)
	: m_syncContext(syncContext)
	, m_parent(NULL)
	, m_name(name)
	, m_guid(id)
	, m_listenerList(ListenerList::Create())
{
	// The starting value is not used for array creation
	XT_UNREFERENCED_PARAM(startingValue);
}


int32 StringArrayElementImpl::GetCount() const
{
	return static_cast<int32>(m_values.size());
}


XStringPtr StringArrayElementImpl::GetValue(int32 index) const
{
	if (index >= 0 && index < GetCount())
	{
		return m_values[index];
	}
	else
	{
		LogError("GetValue passed invalid index in element %s", m_name->GetString().c_str());
		return nullptr;
	}
}


void StringArrayElementImpl::SetValue(int32 index, const XStringPtr& newValue)
{
	if (!IsValid())
	{
		LogError("Attempting to set value on invalid StringArrayElement %s", m_name->GetString().c_str());
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


void StringArrayElementImpl::InsertValue(int32 index, const XStringPtr& value)
{
	if (!IsValid())
	{
		LogError("Attempting to set value on invalid StringArrayElement %s", m_name->GetString().c_str());
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


void StringArrayElementImpl::RemoveValue(int32 index)
{
	if (!IsValid())
	{
		LogError("Attempting to set value on invalid StringArrayElement %s", m_name->GetString().c_str());
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


void StringArrayElementImpl::AddListener(StringArrayListener* newListener)
{
	if (!IsValid())
	{
		LogWarning("Attempting to add a listener to invalid element %s", m_name->GetString().c_str());
	}

	m_listenerList->AddListener(newListener);
}


void StringArrayElementImpl::RemoveListener(StringArrayListener* oldListener)
{
	m_listenerList->RemoveListener(oldListener);
}


ElementType StringArrayElementImpl::GetElementType() const
{
	return ElementType::StringArrayType;
}


XGuid StringArrayElementImpl::GetGUID() const
{
	return m_guid;
}


const XStringPtr& StringArrayElementImpl::GetName() const
{
	return m_name;
}


ElementPtr StringArrayElementImpl::GetParent() const
{
	return m_parent;
}


bool StringArrayElementImpl::IsValid() const
{
	return (m_syncContext && m_syncContext->ElementExists(m_guid));
}


XValue StringArrayElementImpl::GetXValue() const
{
	return XValue();
}


void StringArrayElementImpl::SetXValue(const XValue&)
{
	XTASSERT(IsValid());

}


void StringArrayElementImpl::SetParent(Element* parent)
{
	m_parent = parent;
}


XValue StringArrayElementImpl::GetXValue(int32 index) const
{
	return GetValue(index);
}


void StringArrayElementImpl::SetXValue(int32 index, const XValue& newValue)
{
	if (index >= 0 && index < GetCount())
	{
		const std::string* rawValue = newValue.Get<std::string>();
		if (rawValue != nullptr)
		{
			m_values[index] = new XString(*rawValue);
		}
	}
	else
	{
		LogError("SetXValue passed invalid index in element %s", m_name->GetString().c_str());
	}
}


void StringArrayElementImpl::InsertXValue(int32 index, const XValue& newValue)
{
	if (index >= 0 && index <= GetCount())
	{
		const std::string* rawValue = newValue.Get<std::string>();
		if (rawValue != nullptr)
		{
			m_values.insert(m_values.begin() + index, new XString(*rawValue));
		}
	}
	else
	{
		LogError("InsertXValue passed invalid index in element %s", m_name->GetString().c_str());
	}
}


void StringArrayElementImpl::RemoveXValue(int32 index)
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


void StringArrayElementImpl::NotifyUpdated(int32 index, const XValue& newValue)
{
	XStringPtr stringValue = new XString(*(newValue.Get<std::string>()));
	m_listenerList->NotifyListeners(&StringArrayListener::OnValueChanged, index, stringValue);
}


void StringArrayElementImpl::NotifyInserted(int32 index, const XValue& newValue)
{
	XStringPtr stringValue = new XString(*(newValue.Get<std::string>()));
	m_listenerList->NotifyListeners(&StringArrayListener::OnValueInserted, index, stringValue);
}


void StringArrayElementImpl::NotifyRemoved(int32 index, const XValue& value)
{
	XStringPtr stringValue = new XString(*(value.Get<std::string>()));
	m_listenerList->NotifyListeners(&StringArrayListener::OnValueRemoved, index, stringValue);
}


NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
