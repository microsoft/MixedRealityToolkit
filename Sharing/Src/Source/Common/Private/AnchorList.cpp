//////////////////////////////////////////////////////////////////////////
// AnchorList.cpp
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AnchorList.h"

XTOOLS_NAMESPACE_BEGIN

AnchorList::AnchorList()
	: SyncObject("AnchorList")
{
}

int32 AnchorList::GetAnchorCount() const
{
	return (int32)m_anchorNameElements.size();
}


const XStringPtr& AnchorList::GetAnchorName(int32 index) const
{
	return m_anchorNameElements[index]->GetName();
}


void AnchorList::Clear()
{
	ObjectElementPtr myElement = GetObjectElement();
	if (myElement)
	{
		for (size_t i = 0; i < m_anchorNameElements.size(); ++i)
		{
			myElement->RemoveElement(m_anchorNameElements[i]);
		}
	}

	m_anchorNameElements.clear();
}


bool AnchorList::BindLocal(const ObjectElementPtr& parent, const std::string& name, const UserPtr& owner)
{
	if (SyncObject::BindLocal(parent, name, owner))
	{
		GetObjectElement()->AddListener(this);
		return true;
	}
	else
	{
		return false;
	}
}


void AnchorList::BindRemote(const ElementPtr& element)
{
	SyncObject::BindRemote(element);
	GetObjectElement()->AddListener(this);
}


void AnchorList::OnElementAdded(const ElementPtr& element)
{
	SyncObject::OnElementAdded(element);

	ObjectElementPtr objElement = ObjectElement::Cast(element);
	if (objElement)
	{
		m_anchorNameElements.push_back(objElement);
	}
}


void AnchorList::OnElementDeleted(const ElementPtr& element)
{
	SyncObject::OnElementDeleted(element);

	for (size_t i = 0; i < m_anchorNameElements.size(); ++i)
	{
		if (m_anchorNameElements[i]->GetGUID() == element->GetGUID())
		{
			m_anchorNameElements.erase(m_anchorNameElements.begin() + i);
			break;
		}
	}
}

XTOOLS_NAMESPACE_END
