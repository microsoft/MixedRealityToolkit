//////////////////////////////////////////////////////////////////////////
// Operation.h
//
// Base class of an object that represents a single change to the shared state
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

class SyncContext;
DECLARE_PTR(SyncContext)

class Operation XTABSTRACT : public RefCounted
{
public:
	enum Type : byte
	{
		Ack = 0,
		Noop,
		Create,
		Modify,
		Delete,
		Replace,

		// Arrays only:
		Insert,
		Update,
		Remove,
		Move,
		TypeCount
	};

	Operation(Type type, AuthorityLevel authLevel);
	Operation(Type type, AuthorityLevel authLevel, const std::vector<XGuid>& hierarchy);

	Type GetType() const;
	const char* GetTypeName() const;

	AuthorityLevel GetAuthorityLevel() const;
	void SetAuthorityLevel(AuthorityLevel authLevel);

	virtual XGuid GetTargetGuid() const = 0;

	// Applies to operation to the data set in the given context.  Does not perform any notification to listeners
	virtual void Apply(const SyncContextPtr& context) = 0;

	virtual void Serialize(const NetworkOutMessagePtr& msg) const = 0;

	virtual void Deserialize(NetworkInMessage& msg) = 0;

	// Notify the appropriate listener about the change that this operation has made.
	virtual void Notify(const SyncContextPtr& context) const = 0;

	virtual std::string GetOpDescription() const = 0;

	// Returns the hierarchy of target element.  The first entry is its parent, then its parent, etc
	// until the root
	const std::vector<XGuid>& GetHierarchy() const;

	// Returns true if the target element is a descendant of the given element
	bool IsDescendantOf(XGuid elementID) const;

protected:
	std::vector<XGuid>		m_hierarchy;

private:
	Type					m_type;
	Sync::AuthorityLevel	m_authLevel;
};

DECLARE_PTR(Operation)

typedef std::vector<OperationPtr> OperationList;

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END