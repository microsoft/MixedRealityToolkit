// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// AnchorList.h
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class AnchorList : public SyncObject
{
public:
	AnchorList();

	int32 GetAnchorCount() const;

	const XStringPtr& GetAnchorName(int32 index) const;

	void Clear();

private:
	virtual bool BindLocal(const ObjectElementPtr& parent, const std::string& name, const UserPtr& owner) XTOVERRIDE;
	virtual void BindRemote(const ElementPtr& element) XTOVERRIDE;

	virtual void OnElementAdded(const ElementPtr& element) XTOVERRIDE;
	virtual void OnElementDeleted(const ElementPtr& element) XTOVERRIDE;

	std::vector<ObjectElementPtr> m_anchorNameElements;
};

XTOOLS_NAMESPACE_END
