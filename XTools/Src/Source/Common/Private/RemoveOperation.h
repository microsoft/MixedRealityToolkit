//////////////////////////////////////////////////////////////////////////
// RemoveOperation.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

// Operation to remove an element at a given index in an array
class RemoveOperation : public Operation
{
public:
	RemoveOperation(Sync::AuthorityLevel authLevel);
	RemoveOperation(XGuid guid, int32 index, AuthorityLevel authLevel, const SyncContextPtr& context);
	RemoveOperation(XGuid guid, int32 index, AuthorityLevel authLevel, const std::vector<XGuid>& hierarchy);

	// Operation Functions:
	virtual XGuid			GetTargetGuid() const XTOVERRIDE;
	virtual void			Apply(const SyncContextPtr& context) XTOVERRIDE;
	virtual void			Serialize(const NetworkOutMessagePtr& msg) const XTOVERRIDE;
	virtual void			Deserialize(NetworkInMessage& msg) XTOVERRIDE;
	virtual void			Notify(const SyncContextPtr& context) const XTOVERRIDE;
	virtual std::string		GetOpDescription() const XTOVERRIDE;

	int32					GetIndex() const;

private:
	XGuid					m_elementGuid;
	int32					m_index;
	XValue					m_removedValue;
};

DECLARE_PTR(RemoveOperation)

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
