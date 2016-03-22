//////////////////////////////////////////////////////////////////////////
// InsertOperation.cpp
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "InsertOperation.h"

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

InsertOperation::InsertOperation(Sync::AuthorityLevel authLevel)
	: Operation(Operation::Type::Insert, authLevel)
	, m_elementGuid(kInvalidXGuid)
	, m_index(-1)
{

}


InsertOperation::InsertOperation(XGuid guid, int32 index, XValue newValue, AuthorityLevel authLevel, const SyncContextPtr& context)
	: Operation(Operation::Type::Insert, authLevel, context->GetHierarchy(context->GetElement(guid)->GetParent()->GetGUID()))
	, m_elementGuid(guid)
	, m_index(index)
	, m_newValue(newValue)
{

}


InsertOperation::InsertOperation(XGuid guid, int32 index, XValue newValue, AuthorityLevel authLevel, const std::vector<XGuid>& hierarchy)
	: Operation(Operation::Type::Insert, authLevel, hierarchy)
	, m_elementGuid(guid)
	, m_index(index)
	, m_newValue(newValue)
{

}


XGuid InsertOperation::GetTargetGuid() const
{
	return m_elementGuid;
}


void InsertOperation::Apply(const SyncContextPtr& context)
{
	ElementPtr modifiedElement = context->GetElement(m_elementGuid);
	if (XTVERIFY(modifiedElement))
	{
		ArrayElement* arrayElement = reflection_cast<ArrayElement>(modifiedElement);
		if (XTVERIFY(arrayElement))
		{
			arrayElement->InsertXValue(m_index, m_newValue);
		}
	}
}


void InsertOperation::Serialize(const NetworkOutMessagePtr& msg) const
{
	msg->Write(m_elementGuid);
	msg->Write(m_index);
	m_newValue.Serialize(msg);

	// Write the number of ancestors
	msg->Write(static_cast<uint32>(m_hierarchy.size()));

	// Write the GUIDs of the ancestors
	for (size_t i = 0; i < m_hierarchy.size(); ++i)
	{
		msg->Write(m_hierarchy[i]);
	}
}


void InsertOperation::Deserialize(NetworkInMessage& msg)
{
	m_elementGuid = msg.ReadInt64();
	m_index = msg.ReadInt32();
	m_newValue.Deserialize(msg);

	// Read the number of ancestors
	uint32 numAncestors = msg.ReadUInt32();
	m_hierarchy.reserve(numAncestors);

	// Read the GUIDs of the ancestors
	for (uint32 i = 0; i < numAncestors; ++i)
	{
		m_hierarchy.push_back(msg.ReadInt64());
	}
}


void InsertOperation::Notify(const SyncContextPtr& context) const
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
			arrayElement->NotifyInserted(m_index, m_newValue);
		}
	}
}


std::string InsertOperation::GetOpDescription() const
{
	char buffer[512];

	sprintf_s(buffer, sizeof(buffer), "%s, ID: %lli, Index: %i, Value: %s", GetTypeName(), m_elementGuid, m_index, m_newValue.ToString().c_str());

	return buffer;
}


int32 InsertOperation::GetIndex() const
{
	return m_index;
}


const XValue& InsertOperation::GetValue() const
{
	return m_newValue;
}

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
