//////////////////////////////////////////////////////////////////////////
// RemoveOperation.cpp
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RemoveOperation.h"

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

RemoveOperation::RemoveOperation(Sync::AuthorityLevel authLevel)
	: Operation(Operation::Type::Remove, authLevel)
	, m_elementGuid(kInvalidXGuid)
	, m_index(-1)
{

}


RemoveOperation::RemoveOperation(XGuid guid, int32 index, AuthorityLevel authLevel, const SyncContextPtr& context)
	: Operation(Operation::Type::Remove, authLevel, context->GetHierarchy(context->GetElement(guid)->GetParent()->GetGUID()))
	, m_elementGuid(guid)
	, m_index(index)
{

}


RemoveOperation::RemoveOperation(XGuid guid, int32 index, AuthorityLevel authLevel, const std::vector<XGuid>& hierarchy)
	: Operation(Operation::Type::Remove, authLevel, hierarchy)
	, m_elementGuid(guid)
	, m_index(index)
{

}


XGuid RemoveOperation::GetTargetGuid() const
{
	return m_elementGuid;
}


void RemoveOperation::Apply(const SyncContextPtr& context)
{
	ElementPtr modifiedElement = context->GetElement(m_elementGuid);
	if (XTVERIFY(modifiedElement))
	{
		ArrayElement* arrayElement = reflection_cast<ArrayElement>(modifiedElement);
		if (XTVERIFY(arrayElement))
		{
			m_removedValue = arrayElement->GetXValue(m_index);
			arrayElement->RemoveXValue(m_index);
		}
	}
}


void RemoveOperation::Serialize(const NetworkOutMessagePtr& msg) const
{
	msg->Write(m_elementGuid);
	msg->Write(m_index);

	// Write the number of ancestors
	msg->Write(static_cast<uint32>(m_hierarchy.size()));

	// Write the GUIDs of the ancestors
	for (size_t i = 0; i < m_hierarchy.size(); ++i)
	{
		msg->Write(m_hierarchy[i]);
	}
}


void RemoveOperation::Deserialize(NetworkInMessage& msg)
{
	m_elementGuid = msg.ReadInt64();
	m_index = msg.ReadInt32();

	// Read the number of ancestors
	uint32 numAncestors = msg.ReadUInt32();
	m_hierarchy.reserve(numAncestors);

	// Read the GUIDs of the ancestors
	for (uint32 i = 0; i < numAncestors; ++i)
	{
		m_hierarchy.push_back(msg.ReadInt64());
	}
}


void RemoveOperation::Notify(const SyncContextPtr& context) const
{
	// Check that the element has not been deleted since this op was applied
	ElementPtr modifiedElement = context->GetElement(m_elementGuid);
	if (modifiedElement)
	{
		// Cast to an ArrayElement
		ArrayElement* arrayElement = reflection_cast<ArrayElement>(modifiedElement);
		if (XTVERIFY(arrayElement))
		{
			// Notify the listener that the value was updated
			arrayElement->NotifyRemoved(m_index, m_removedValue);
		}
	}
}


std::string RemoveOperation::GetOpDescription() const
{
	char buffer[512];

	sprintf_s(buffer, sizeof(buffer), "%s, ID: %lli, Index: %i", GetTypeName(), m_elementGuid, m_index);

	return buffer;
}

int32 RemoveOperation::GetIndex() const
{
	return m_index;
}

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
