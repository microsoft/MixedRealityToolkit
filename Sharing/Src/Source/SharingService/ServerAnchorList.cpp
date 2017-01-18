//////////////////////////////////////////////////////////////////////////
// ServerAnchorList.cpp
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ServerAnchorList.h"

XTOOLS_NAMESPACE_BEGIN

ServerAnchorList::ServerAnchorList()
	: SyncObject("AnchorList")
{
}


BufferPtr ServerAnchorList::GetAnchorData(const std::string& name) const
{
	auto itr = m_anchors.find(name);
	if (itr != m_anchors.end())
	{
		return itr->second.m_data;
	}
	else
	{
		return nullptr;
	}
}


void ServerAnchorList::SetAnchor(const std::string& name, const BufferPtr& data)
{
	// Overwrite the anchor with the given name OR create a new anchor with the name
	XStringPtr nameXString = new XString(name);

	ObjectElementPtr newAnchorElement = ObjectElement::Cast(GetObjectElement()->GetElement(nameXString));
	if (newAnchorElement == nullptr)
	{
		newAnchorElement = GetObjectElement()->CreateObjectElement(nameXString, GetObjectType());
	}

	m_anchors[name] = Anchor(newAnchorElement, data);
}


void ServerAnchorList::RemoveAnchor(const std::string& name)
{
	auto itr = m_anchors.find(name);
	if (itr != m_anchors.end())
	{
		GetObjectElement()->RemoveElement(itr->second.m_element);
		m_anchors.erase(itr);
	}
}

XTOOLS_NAMESPACE_END
