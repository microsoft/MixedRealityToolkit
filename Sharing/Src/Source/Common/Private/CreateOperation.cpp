//////////////////////////////////////////////////////////////////////////
// CreateOperation.cpp
//
// An operation that adds a new element to the shared state
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CreateOperation.h"
#include "ObjectElementImpl.h"
#include "ModifyOperation.h"
#include "NoopOperation.h"

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

CreateOperation::CreateOperation(Sync::AuthorityLevel authLevel)
	: Operation(Operation::Create, authLevel)
	, m_elementType(ElementType::UnknownType)
	, m_elementGuid(kInvalidXGuid)
	, m_ownerID(User::kInvalidUserID)
{
	
}


CreateOperation::CreateOperation(ElementType type, const XStringPtr& name, XGuid guid, XGuid parentGuid, UserID ownerID, XValue startingValue, Sync::AuthorityLevel authLevel, const SyncContextPtr& context)
	: Operation(Operation::Create, authLevel, context->GetHierarchy(parentGuid))
	, m_name(name)
	, m_elementType(type)
	, m_elementGuid(guid)
	, m_ownerID(ownerID)
	, m_startingValue(startingValue)
{
	XTASSERT(name);
}


CreateOperation::CreateOperation(ElementType type, const XStringPtr& name, XGuid guid, UserID ownerID, XValue startingValue, Sync::AuthorityLevel authLevel, const std::vector<XGuid>& hierarchy)
	: Operation(Operation::Create, authLevel, hierarchy)
	, m_name(name)
	, m_elementType(type)
	, m_elementGuid(guid)
	, m_ownerID(ownerID)
	, m_startingValue(startingValue)
{
	XTASSERT(name);
}


CreateOperation::CreateOperation(const CreateOperation& rhs)
	: Operation(Operation::Create, rhs.GetAuthorityLevel(), rhs.m_hierarchy)
	, m_name(rhs.m_name)
	, m_elementType(rhs.m_elementType)
	, m_elementGuid(rhs.m_elementGuid)
	, m_ownerID(rhs.m_ownerID)
	, m_startingValue(rhs.m_startingValue)
{

}


XGuid CreateOperation::GetTargetGuid() const
{
	return m_elementGuid;
}


void CreateOperation::Apply(const SyncContextPtr& context)
{
	// Element should not exist before we create it
	XTASSERT(context->GetElement(m_elementGuid) == NULL);

	// Create the new element.  
	// We have to ensure that the added element is not destroyed before the listener is notified.  
	m_createdElement = context->CreateElement(m_elementType, m_name, m_elementGuid, GetParentGUID(), m_ownerID, m_startingValue);
	XTASSERT(m_createdElement);
}


void CreateOperation::Serialize(const NetworkOutMessagePtr& msg) const
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


void CreateOperation::Deserialize(NetworkInMessage& msg)
{
	m_elementType	= (ElementType)msg.ReadByte();
	m_name			= msg.ReadString();
	m_elementGuid	= msg.ReadInt64();
	m_ownerID		= msg.ReadInt32();
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


void CreateOperation::Notify(const SyncContextPtr& context) const
{
	// Should have been set in Apply
	XTASSERT(m_createdElement);

	ElementPtr parentElement = context->GetElement(GetParentGUID());

	// The parent might have been deleted since the element was added, so be sure to check
	if (parentElement != NULL)
	{
		ObjectElementImpl* parentObjectElement = reflection_cast<ObjectElementImpl>(parentElement);
		if (XTVERIFY(parentObjectElement))
		{
			// Notify the listeners that it a new object has been created
			parentObjectElement->GetListeners()->NotifyListeners(&ObjectElementListener::OnElementAdded, m_createdElement);
		}
	}
}


std::string CreateOperation::GetOpDescription() const
{
	char buffer[512];
	sprintf_s(buffer, sizeof(buffer), "%s, ID: %lli, Name: %s, Value: %s", GetTypeName(), m_elementGuid, m_name->GetString().c_str(), m_startingValue.ToString().c_str());
	return buffer;
}


XGuid CreateOperation::GetParentGUID() const
{
	return m_hierarchy[0];
}


const XValue& CreateOperation::GetValue() const
{
	return m_startingValue;
}


ElementType CreateOperation::GetElementType() const
{
	return m_elementType;
}


const XStringPtr& CreateOperation::GetName() const
{
	return m_name;
}


UserID CreateOperation::GetOwnerID() const
{
	return m_ownerID;
}


NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
