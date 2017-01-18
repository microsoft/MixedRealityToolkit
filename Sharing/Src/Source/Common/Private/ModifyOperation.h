//////////////////////////////////////////////////////////////////////////
// ModifyOperation.h
//
// An operation that changes the value of an element that exists in the shared state
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

class ModifyOperation : public Operation
{
public:
	ModifyOperation(AuthorityLevel authLevel);
	ModifyOperation(XGuid guid, XValue newValue, AuthorityLevel authLevel, const SyncContextPtr& context);
	ModifyOperation(XGuid guid, XValue newValue, AuthorityLevel authLevel, const std::vector<XGuid>& hierarchy);
	explicit ModifyOperation(const ModifyOperation& rhs);
	explicit ModifyOperation(const CreateOperationConstPtr& createOp);

	// Operation Functions:
	virtual XGuid				GetTargetGuid() const XTOVERRIDE;
	virtual void				Apply(const SyncContextPtr& context) XTOVERRIDE;
	virtual void				Serialize(const NetworkOutMessagePtr& msg) const XTOVERRIDE;
	virtual void				Deserialize(NetworkInMessage& msg) XTOVERRIDE;
	virtual void				Notify(const SyncContextPtr& context) const XTOVERRIDE;
	virtual std::string			GetOpDescription() const XTOVERRIDE;

	const XValue&				GetValue() const;

private:
	XGuid					m_elementGuid;
	XValue					m_newValue;
};

DECLARE_PTR(ModifyOperation)

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END