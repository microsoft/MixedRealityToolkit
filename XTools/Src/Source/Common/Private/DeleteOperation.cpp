//////////////////////////////////////////////////////////////////////////
// DeleteOperation.cpp
//
// An operation that removes an element from the shared state
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DeleteOperation.h"
#include "ObjectElementImpl.h"

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

DeleteOperation::DeleteOperation(Sync::AuthorityLevel authLevel)
	: Operation(Operation::Delete, authLevel)
	, m_elementGuid(kInvalidXGuid)
{

}


DeleteOperation::DeleteOperation(XGuid guid, XGuid parentGuid, const SyncContextPtr& context)
	: Operation(Operation::Delete, context->GetAuthorityLevel(), context->GetHierarchy(parentGuid))
	, m_elementGuid(guid)
{

}


DeleteOperation::DeleteOperation(const DeleteOperation& rhs)
	: Operation(Operation::Delete, rhs.GetAuthorityLevel(), rhs.m_hierarchy)
	, m_elementGuid(rhs.m_elementGuid)
{

}


XGuid DeleteOperation::GetTargetGuid() const
{
	return m_elementGuid;
}


void DeleteOperation::Apply(const SyncContextPtr& context)
{
	// Hold on to the pointer so that it can be used in notification later
	m_deletedElement = context->GetElement(m_elementGuid);
	if (XTVERIFY(m_deletedElement))
	{
		// Delete the element and all its children
		context->DeleteElement(m_deletedElement);
	}
}


void DeleteOperation::Serialize(const NetworkOutMessagePtr& msg) const
{
	// Write the GUID of the element to delete
	msg->Write(m_elementGuid);

	// Write the number of ancestors
	msg->Write(static_cast<uint32>(m_hierarchy.size()));

	// Write the GUIDs of the ancestors
	for (size_t i = 0; i < m_hierarchy.size(); ++i)
	{
		msg->Write(m_hierarchy[i]);
	}

	// NOTE: authority level is NOT included in the op when serialized; it changes as the op is sent around the network
}


void DeleteOperation::Deserialize(NetworkInMessage& msg)
{
	// Read the GUID of the element to delete
	m_elementGuid = msg.ReadInt64();

	// Read the number of ancestors
	uint32 numAncestors = msg.ReadUInt32();
	m_hierarchy.reserve(numAncestors);

	// Read the GUIDs of the ancestors
	for (uint32 i = 0; i < numAncestors; ++i)
	{
		m_hierarchy.push_back(msg.ReadInt64());
	}

	// NOTE: authority level is NOT included in the op when serialized; it changes as the op is sent around the network
}


void DeleteOperation::Notify(const SyncContextPtr& context) const
{
	// Don't notify about elements that were already deleted before this op was applied
	if (XTVERIFY(m_deletedElement))
	{
		// Get the parent of the deleted element. 
		// NOTE: the parent may have been deleted by another pending op,
		// so its valid for this to return NULL
		ElementPtr parent = context->GetElement(m_hierarchy[0]);
		if (parent)
		{
			ObjectElementImpl* parentObjectElement = reflection_cast<ObjectElementImpl>(parent);
			if (XTVERIFY(parentObjectElement))
			{
				// Notify the listeners that it a new object has been created
				parentObjectElement->GetListeners()->NotifyListeners(&ObjectElementListener::OnElementDeleted, m_deletedElement);
			}
		}
	}
}


std::string DeleteOperation::GetOpDescription() const
{
	char buffer[512];
	sprintf_s(buffer, sizeof(buffer), "%s, ID: %lli", GetTypeName(), m_elementGuid);
	return buffer;
}

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
