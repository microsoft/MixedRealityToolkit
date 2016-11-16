//////////////////////////////////////////////////////////////////////////
// ReplaceOperation.cpp
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ReplaceOperation.h"
#include "CreateOperation.h"
#include "ObjectElementImpl.h"
#include "ModifyOperation.h"
#include "NoopOperation.h"

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

ReplaceOperation::ReplaceOperation(Sync::AuthorityLevel authLevel)
	: Operation(Operation::Replace, authLevel)
	, m_elementType(ElementType::UnknownType)
	, m_elementGuid(kInvalidXGuid)
	, m_ownerID(User::kInvalidUserID)
{

}


ReplaceOperation::ReplaceOperation(const CreateOperationConstPtr& createOp)
	: Operation(Operation::Replace, createOp->GetAuthorityLevel(), createOp->GetHierarchy())
	, m_name(createOp->GetName())
	, m_elementType(createOp->GetElementType())
	, m_elementGuid(createOp->GetTargetGuid())
	, m_ownerID(createOp->GetOwnerID())
	, m_startingValue(createOp->GetValue())
{
	
}


ReplaceOperation::ReplaceOperation(const ReplaceOperation& rhs)
	: Operation(Operation::Replace, rhs.GetAuthorityLevel(), rhs.m_hierarchy)
	, m_name(rhs.m_name)
	, m_elementType(rhs.m_elementType)
	, m_elementGuid(rhs.m_elementGuid)
	, m_ownerID(rhs.m_ownerID)
	, m_startingValue(rhs.m_startingValue)
{

}


XGuid ReplaceOperation::GetTargetGuid() const
{
	return m_elementGuid;
}


void ReplaceOperation::Apply(const SyncContextPtr& context)
{
	// Hold on to the pointer so that it can be used in notification later
	m_deletedElement = context->GetElement(m_elementGuid);
	if (XTVERIFY(m_deletedElement))
	{
		// Delete the element and all its children
		context->DeleteElement(m_deletedElement);
	}

	// Create the new element.  
	// We have to ensure that the added element is not destroyed before the listener is notified, so hold on to a reference
	m_createdElement = context->CreateElement(m_elementType, m_name, m_elementGuid, m_hierarchy[0], m_ownerID, m_startingValue);
	XTASSERT(m_createdElement);
}


void ReplaceOperation::Serialize(const NetworkOutMessagePtr& msg) const
{
	msg->Write((byte)m_elementType);
	msg->Write(m_name);
	msg->Write(m_elementGuid);
	msg->Write(m_ownerID);
	m_startingValue.Serialize(msg);

	// Write the number of ancestors
	msg->Write(static_cast<uint32>(m_hierarchy.size()));

	// Write the GUIDs of the ancestors
	for (size_t i = 0; i < m_hierarchy.size(); ++i)
	{
		msg->Write(m_hierarchy[i]);
	}

	// NOTE: authority level is NOT included in the op when serialized; it changes as the op is sent around the network
}


void ReplaceOperation::Deserialize(NetworkInMessage& msg)
{
	m_elementType = (ElementType)msg.ReadByte();
	m_name = msg.ReadString();
	m_elementGuid = msg.ReadInt64();
	m_ownerID = msg.ReadInt32();
	m_startingValue.Deserialize(msg);

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


void ReplaceOperation::Notify(const SyncContextPtr& context) const
{
	// First send an ElementDeleted event for the old element, then send an ElementAdded event for the new element

	// Should have been created in Apply before this function gets called
	XTASSERT(m_createdElement);

	// Get the parent of the deleted element. 
	// NOTE: the parent may have been deleted by another pending op,
	// so its valid for this to return NULL

	ElementPtr parentElement = context->GetElement(m_hierarchy[0]);
	if (parentElement != NULL)
	{
		ObjectElementImpl* parentObjectElement = reflection_cast<ObjectElementImpl>(parentElement);
		if (XTVERIFY(parentObjectElement))
		{
			// Don't notify about elements that were already deleted before this op was applied
			if (XTVERIFY(m_deletedElement))
			{
				// Notify the listeners that it a new object has been created
				parentObjectElement->GetListeners()->NotifyListeners(&ObjectElementListener::OnElementDeleted, m_deletedElement);
			}

			// Notify the listeners that a new object has been created
			parentObjectElement->GetListeners()->NotifyListeners(&ObjectElementListener::OnElementAdded, m_createdElement);
		}
	}
}


std::string ReplaceOperation::GetOpDescription() const
{
	char buffer[512];
	sprintf_s(buffer, sizeof(buffer), "%s, ID: %lli, Name: %s, Value: %s", GetTypeName(), m_elementGuid, m_name->GetString().c_str(), m_startingValue.ToString().c_str());
	return buffer;
}


XGuid ReplaceOperation::GetParentGUID() const
{
	return m_hierarchy[0];
}


const XValue& ReplaceOperation::GetValue() const
{
	return m_startingValue;
}


ElementType ReplaceOperation::GetElementType() const
{
	return m_elementType;
}


const XStringPtr& ReplaceOperation::GetName() const
{
	return m_name;
}


UserID ReplaceOperation::GetOwnerID() const
{
	return m_ownerID;
}


NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
