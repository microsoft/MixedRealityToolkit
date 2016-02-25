//////////////////////////////////////////////////////////////////////////
// InsertOperation.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

// Operation to insert a new element at a given index in an array
class InsertOperation : public Operation
{
public:
	InsertOperation(Sync::AuthorityLevel authLevel);
	InsertOperation(XGuid guid, int32 index, XValue newValue, AuthorityLevel authLevel, const SyncContextPtr& context);
	InsertOperation(XGuid guid, int32 index, XValue newValue, AuthorityLevel authLevel, const std::vector<XGuid>& hierarchy);

	// Operation Functions:
	virtual XGuid			GetTargetGuid() const XTOVERRIDE;
	virtual void			Apply(const SyncContextPtr& context) XTOVERRIDE;
	virtual void			Serialize(const NetworkOutMessagePtr& msg) const XTOVERRIDE;
	virtual void			Deserialize(NetworkInMessage& msg) XTOVERRIDE;
	virtual void			Notify(const SyncContextPtr& context) const XTOVERRIDE;
	virtual std::string		GetOpDescription() const XTOVERRIDE;

	int32					GetIndex() const;
	const XValue&			GetValue() const;

private:
	XGuid					m_elementGuid;
	int32					m_index;
	XValue					m_newValue;
};

DECLARE_PTR(InsertOperation)

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
