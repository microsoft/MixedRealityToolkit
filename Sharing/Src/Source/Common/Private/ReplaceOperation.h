//////////////////////////////////////////////////////////////////////////
// ReplaceOperation.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

class ReplaceOperation : public Operation
{
public:
	ReplaceOperation(Sync::AuthorityLevel authLevel);
	ReplaceOperation(const CreateOperationConstPtr& createOp);
	explicit ReplaceOperation(const ReplaceOperation& rhs);

	// Operation Functions:
	virtual XGuid					GetTargetGuid() const XTOVERRIDE;
	virtual void					Apply(const SyncContextPtr& context) XTOVERRIDE;
	virtual void					Serialize(const NetworkOutMessagePtr& msg) const XTOVERRIDE;
	virtual void					Deserialize(NetworkInMessage& msg) XTOVERRIDE;
	virtual void					Notify(const SyncContextPtr& context) const XTOVERRIDE;
	virtual std::string				GetOpDescription() const XTOVERRIDE;

	// Local Functions:
	XGuid							GetParentGUID() const;
	const XValue&					GetValue() const;
	ElementType						GetElementType() const;
	const XStringPtr&				GetName() const;
	UserID							GetOwnerID() const;

private:
	XStringPtr				m_name;
	ElementPtr				m_createdElement;
	ElementPtr				m_deletedElement;
	ElementType				m_elementType;
	XGuid					m_elementGuid;
	UserID					m_ownerID;
	XValue					m_startingValue;
};

DECLARE_PTR(ReplaceOperation)

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
