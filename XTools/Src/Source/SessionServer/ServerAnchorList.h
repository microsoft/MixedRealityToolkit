//////////////////////////////////////////////////////////////////////////
// ServerAnchorList.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <Private/Buffer.h>

XTOOLS_NAMESPACE_BEGIN

class ServerAnchorList : public SyncObject
{
public:
	ServerAnchorList();

	BufferPtr GetAnchorData(const std::string& name) const;

	void SetAnchor(const std::string& name, const BufferPtr& data);
	void RemoveAnchor(const std::string& name);

private:

	struct Anchor
	{
		Anchor() {}
		Anchor(const ObjectElementPtr& element, const BufferPtr& data)
			: m_element(element)
			, m_data(data) {}

		ObjectElementPtr m_element;
		BufferPtr m_data;
	};

	std::map<std::string, Anchor> m_anchors;
};

XTOOLS_NAMESPACE_END
