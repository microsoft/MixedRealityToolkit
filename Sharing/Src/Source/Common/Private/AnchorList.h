//////////////////////////////////////////////////////////////////////////
// AnchorList.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
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
