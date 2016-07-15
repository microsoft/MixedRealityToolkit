// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// CreateOperation.h
// An operation that adds a new element to the shared state
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

class CreateOperation : public Operation
{
public:
	CreateOperation(Sync::AuthorityLevel authLevel);
	CreateOperation(ElementType type, const XStringPtr& name, XGuid guid, XGuid parentGuid, XValue startingValue, Sync::AuthorityLevel authLevel, const SyncContextPtr& context);
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

private:
	XStringPtr				m_name;
	ElementPtr				m_createdElement;
	ElementType				m_elementType;
	XGuid					m_elementGuid;
	XGuid					m_parentGuid;
	XValue					m_startingValue;
	bool					m_bAlreadyExisted;
};

DECLARE_PTR(CreateOperation)

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
