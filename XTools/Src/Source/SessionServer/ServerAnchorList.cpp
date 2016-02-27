//////////////////////////////////////////////////////////////////////////
// ServerAnchorList.cpp
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ServerAnchorList.h"

XTOOLS_NAMESPACE_BEGIN

ServerAnchorList::ServerAnchorList()
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
	ObjectElementPtr newAnchorElement = GetElement()->CreateObjectElement(new XString(name));
	m_anchors[name] = Anchor(newAnchorElement, data);
}


void ServerAnchorList::RemoveAnchor(const std::string& name)
{
	auto itr = m_anchors.find(name);
	if (itr != m_anchors.end())
	{
		GetElement()->RemoveElement(itr->second.m_element);
		m_anchors.erase(itr);
	}
}

XTOOLS_NAMESPACE_END
