//////////////////////////////////////////////////////////////////////////
// DeleteOperation.h
//
// An operation that removes an element from the shared state
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

class DeleteOperation : public Operation
{
public:
	DeleteOperation(Sync::AuthorityLevel authLevel);
	DeleteOperation(XGuid guid, XGuid parentGuid, const SyncContextPtr& context);
	explicit DeleteOperation(const DeleteOperation& rhs);

	// Operation Functions:
	virtual XGuid			GetTargetGuid() const XTOVERRIDE;
	virtual void			Apply(const SyncContextPtr& context) XTOVERRIDE;
	virtual void			Serialize(const NetworkOutMessagePtr& msg) const XTOVERRIDE;
	virtual void			Deserialize(NetworkInMessage& msg) XTOVERRIDE;
	virtual void			Notify(const SyncContextPtr& context) const XTOVERRIDE;
	virtual std::string		GetOpDescription() const XTOVERRIDE;

private:
	XGuid					m_elementGuid;
	ElementPtr				m_deletedElement;
};

DECLARE_PTR(DeleteOperation)

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END