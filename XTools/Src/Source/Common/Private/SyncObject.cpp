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


SyncObject::SyncObject()
{

}


const ObjectElementPtr& SyncObject::GetElement()
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


void SyncObject::OnIntElementChanged(XGuid elementID, int32 newValue)
{
	OnElementValueChanged(elementID, newValue);
}


void SyncObject::OnFloatElementChanged(XGuid elementID, float newValue)
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
		if (XTVERIFY(syncable->GetGUID() == kInvalidXGuid))
		{
			syncable->BindRemote(element);

			// Register the member's new XGuid with the callback maps
			m_memberMap[syncable->GetGUID()] = syncable;
		}
		else
		{
			LogError("Cannot bind remote element %s because it has already been bound", memberIt->first.c_str());
		}
	}
}


void SyncObject::OnElementDeleted(const ElementPtr& )
{
	// TODO: implement
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
	m_element = parent->CreateObjectElement(new XString(name), owner.get());
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
		// Register this object as a listener for change events of its children
		m_element->AddListener(this);

		// NOTE: Expect that OnElementAdded should now get called for each member,
		// so handle the member setup in that function
	}
	else
	{
		LogError("Failed to use remotely created sync element for object %s", element->GetName()->GetString().c_str());
	}
}

XTOOLS_NAMESPACE_END
