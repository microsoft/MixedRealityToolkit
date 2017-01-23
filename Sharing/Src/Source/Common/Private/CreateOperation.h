//////////////////////////////////////////////////////////////////////////
// CreateOperation.h
//
// An operation that adds a new element to the shared state
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

class CreateOperation : public Operation
{
public:
	CreateOperation(Sync::AuthorityLevel authLevel);
	CreateOperation(ElementType type, const XStringPtr& name, XGuid guid, XGuid parentGuid, UserID ownerID, XValue startingValue, Sync::AuthorityLevel authLevel, const SyncContextPtr& context);
	CreateOperation(ElementType type, const XStringPtr& name, XGuid guid, UserID ownerID, XValue startingValue, Sync::AuthorityLevel authLevel, const std::vector<XGuid>& hierarchy);
	explicit CreateOperation(const CreateOperation& rhs);

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
	ElementType				m_elementType;
	XGuid					m_elementGuid;
	UserID					m_ownerID;
	XValue					m_startingValue;
};

DECLARE_PTR(CreateOperation)

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END