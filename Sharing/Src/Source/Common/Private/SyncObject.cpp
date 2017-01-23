//////////////////////////////////////////////////////////////////////////
// SyncObject.cpp
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SyncObject.h"

XTOOLS_NAMESPACE_BEGIN

XTOOLS_REFLECTION_DEFINE(SyncObject)
.BaseClass<Syncable>();


SyncObject::SyncObject(const std::string& objectType)
	: m_objectType(new XString(objectType))
{

}


SyncObject::~SyncObject()
{
	if (m_element && m_element->IsValid())
	{
		ObjectElementPtr parent = ObjectElement::Cast(m_element->GetParent());
		if (parent)
		{
			parent->RemoveElement(m_element);
		}
	}
}


const ObjectElementPtr& SyncObject::GetObjectElement() const
{
	return m_element;
}


const XStringPtr& SyncObject::GetObjectType() const
{
	return m_objectType;
}


XGuid SyncObject::GetGUID() const
{
	if (m_element) { return m_element->GetGUID(); }
	else { return kInvalidXGuid; }
}


ElementType SyncObject::GetType() const
{
	return ElementType::ObjectType;
}


ElementPtr SyncObject::GetElement() const
{
	return m_element;
}


void SyncObject::AddMember(Syncable* memberSyncable, const std::string& name)
{
	AddMember(memberSyncable, name, MemberCallback());
}


void SyncObject::AddMember(Syncable* memberSyncable, const std::string& name, MemberCallback onChangeCallback)
{
	m_members[name] = memberSyncable;

	if (onChangeCallback.IsBound())
	{
		m_callbackMap[memberSyncable] = onChangeCallback;
	}
}


void SyncObject::OnBoolElementChanged(XGuid elementID, bool newValue)
{
	OnElementValueChanged(elementID, newValue);
}


void SyncObject::OnIntElementChanged(XGuid elementID, int32 newValue)
{
	OnElementValueChanged(elementID, newValue);
}


void SyncObject::OnLongElementChanged(XGuid elementID, int64 newValue)
{
	OnElementValueChanged(elementID, newValue);
}


void SyncObject::OnFloatElementChanged(XGuid elementID, float newValue)
{
	OnElementValueChanged(elementID, newValue);
}


void SyncObject::OnDoubleElementChanged(XGuid elementID, double newValue)
{
	OnElementValueChanged(elementID, newValue);
}


void SyncObject::OnStringElementChanged(XGuid elementID, const XStringPtr& newValue)
{
	OnElementValueChanged(elementID, newValue);
}


void SyncObject::OnElementAdded(const ElementPtr& element)
{
	// Find the element in the list of members
	auto memberIt = m_members.find(element->GetName()->GetString());
	if (memberIt != m_members.end())
	{
		// A member exists for this element; attempt to bind it
		Syncable* syncable = memberIt->second;
		if (syncable->GetGUID() == kInvalidXGuid)
		{
			syncable->BindRemote(element);

			// Register the member's new XGuid with the callback maps
			m_memberMap[syncable->GetGUID()] = syncable;
		}
	}
}


void SyncObject::OnElementDeleted(const ElementPtr& )
{
	// TODO: implement
	XTASSERT(false);
}


void SyncObject::SetValue(const XValue& )
{
	// Object elements should not receive this callback
	XTASSERT(false);
}


template<typename T>
void SyncObject::OnElementValueChanged(XGuid elementID, T newValue)
{
	auto memberMapIter = m_memberMap.find(elementID);
	if (memberMapIter != m_memberMap.end() && memberMapIter->second != nullptr)
	{
		Syncable* syncedElement = memberMapIter->second;
		syncedElement->SetValue(newValue);

		auto callbackMapIter = m_callbackMap.find(syncedElement);
		if (callbackMapIter != m_callbackMap.end() && callbackMapIter->second.IsBound())
		{
			MemberCallback callback = callbackMapIter->second;
			callback.Call(elementID);
		}
	}
}


bool SyncObject::BindLocal(const ObjectElementPtr& parent, const std::string& name, const UserPtr& owner)
{
	m_element = parent->CreateObjectElement(new XString(name), m_objectType, owner.get());
	if (XTVERIFY(m_element != nullptr))
	{
		// Register this object as a listener for change events of its children
		m_element->AddListener(this);

		// Recursively initialize all the members of this class
		for (auto it = m_members.begin(); it != m_members.end(); ++it)
		{
			Syncable* currentSyncable = (*it).second;
			std::string elementName = (*it).first;
			currentSyncable->BindLocal(m_element, elementName, owner);

			// Register the member's new XGuid with the callback maps
			m_memberMap[currentSyncable->GetGUID()] = currentSyncable;
		}

		return true;
	}
	else
	{
		LogError("Failed to create sync element for object %s", name.c_str());
		return false;
	}
}


void SyncObject::BindRemote(const ElementPtr& element)
{
	m_element = ObjectElement::Cast(element);
	if (XTVERIFY(m_element != nullptr))
	{
		XTASSERT(m_element->GetObjectType()->IsEqual(m_objectType));

		// Register this object as a listener for change events of its children
		m_element->AddListener(this);

		int32 childCount = m_element->GetElementCount();
		for (int32 i = 0; i < childCount; ++i)
		{
			ElementPtr childElement = m_element->GetElementAt(i);

			auto memberIt = m_members.find(childElement->GetName()->GetString());
			if (memberIt != m_members.end())
			{
				// A member exists for this element; attempt to bind it
				Syncable* syncable = memberIt->second;
				if (syncable->GetGUID() == kInvalidXGuid)
				{
					syncable->BindRemote(childElement);

					// Register the member's new XGuid with the callback maps
					m_memberMap[syncable->GetGUID()] = syncable;
				}
			}
		}
	}
	else
	{
		LogError("Failed to use remotely created sync element for object %s", element->GetName()->GetString().c_str());
	}
}


void SyncObject::Unbind()
{
	if (m_element != nullptr)
	{
		for (auto childItr = m_memberMap.begin(); childItr != m_memberMap.end(); ++childItr)
		{
			childItr->second->Unbind();
		}

		m_memberMap.clear();

		m_element->RemoveListener(this);
		m_element = nullptr;
	}
}

XTOOLS_NAMESPACE_END
