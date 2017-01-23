//////////////////////////////////////////////////////////////////////////
// SyncContext.h
//
// Contains common data used by multiple parts of the sync process
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector>

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

// Define a type to store unique IDs for each system
typedef int32 SystemID;
static const SystemID kUnknownSystemID = 0xFFFFFFFF;

class ObjectElementImpl;
DECLARE_PTR(ObjectElementImpl)

class SyncContext : public AtomicRefCounted
{
public:
	SyncContext(AuthorityLevel authorityLevel, SystemID localSystemID, const UserPtr& localUser);

	ObjectElementPtr			GetRootObject() const;

	const TransformManager&		GetTransformManager() const;

	const OperationFactory&		GetOpFactory() const;

	void						AddAppliedOperation(const OperationPtr& op);

	const OperationList&		GetAppliedOperations() const;

	// clear out the accumulated history of applied operations
	void						ClearAppliedOperations();

	bool						ElementExists(XGuid id) const;

	ElementPtr					GetElement(XGuid id) const;

	SystemID					GetLocalSystemID() const;
	AuthorityLevel				GetAuthorityLevel() const;

	ElementPtr					CreateElement(ElementType type, const XStringPtr& name, XGuid guid, XGuid parentGuid, UserID owner, XValue startingValue);
	void						DeleteElement(const ElementPtr& element);

	// Return the full path string of the given element
	std::string					GetElementPath(XGuid id) const;

	XGuid						CreateGUID(const XStringPtr& name, XGuid parent) const;

	std::vector<XGuid>			GetHierarchy(XGuid startingID) const;

	const UserPtr&				GetLocalUser() const;

	void						PrintSyncDataTree() const;

private:
	void						PrintElementRecurs(const ElementConstPtr& element, int depth) const;
	void						GetElementPathRecurs(XGuid id, std::string& pathOut) const;
	void						DeleteElementRecurs(const ElementPtr& element);

	ObjectElementImplPtr			m_rootObject;
	TransformManager				m_transformMgr;
	OperationList					m_appliedChanges;
	std::map<XGuid, ElementPtr>		m_guidMap;
	AuthorityLevel					m_authorityLevel;
	SystemID						m_localSystemID;
	OperationFactory				m_opFactory;
	ElementFactory					m_elementFactory;
	UserPtr							m_localUser;
};

DECLARE_PTR(SyncContext)

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END