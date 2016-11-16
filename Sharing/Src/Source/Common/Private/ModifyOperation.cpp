//////////////////////////////////////////////////////////////////////////
// ModifyOperation.cpp
//
// An operation that changes the value of an element that exists in the shared state
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ModifyOperation.h"
#include "ObjectElementImpl.h"
#include "BoolElementImpl.h"
#include "IntElementImpl.h"
#include "LongElementImpl.h"
#include "FloatElementImpl.h"
#include "DoubleElementImpl.h"
#include "StringElementImpl.h"

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

ModifyOperation::ModifyOperation(AuthorityLevel authLevel)
: Operation(Operation::Modify, authLevel)
, m_elementGuid(kInvalidXGuid)
{

}


ModifyOperation::ModifyOperation(XGuid guid, XValue newValue, AuthorityLevel authLevel, const SyncContextPtr& context)
	: Operation(Operation::Modify, authLevel, context->GetHierarchy(context->GetElement(guid)->GetParent()->GetGUID()))
	, m_elementGuid(guid)
	, m_newValue(newValue)
{

}

ModifyOperation::ModifyOperation(XGuid guid, XValue newValue, AuthorityLevel authLevel, const std::vector<XGuid>& hierarchy)
	: Operation(Operation::Modify, authLevel, hierarchy)
	, m_elementGuid(guid)
	, m_newValue(newValue)
{

}


ModifyOperation::ModifyOperation(const ModifyOperation& rhs)
	: Operation(Operation::Modify, rhs.GetAuthorityLevel(), rhs.m_hierarchy)
	, m_elementGuid(rhs.m_elementGuid)
	, m_newValue(rhs.m_newValue)
{

}


ModifyOperation::ModifyOperation(const CreateOperationConstPtr& createOp)
	: Operation(Operation::Modify, createOp->GetAuthorityLevel())
	, m_elementGuid(createOp->GetTargetGuid())
	, m_newValue(createOp->GetValue())
{
	m_hierarchy = createOp->GetHierarchy();
}


XGuid ModifyOperation::GetTargetGuid() const
{
	return m_elementGuid;
}


void ModifyOperation::Apply(const SyncContextPtr& context)
{
	// Get the element and its parent (if any)
	ElementPtr modifiedElement = context->GetElement(m_elementGuid);
	if (XTVERIFY(modifiedElement))
	{
		modifiedElement->SetXValue(m_newValue);
	}
}


void ModifyOperation::Serialize(const NetworkOutMessagePtr& msg) const
{
	msg->Write(m_elementGuid);
	m_newValue.Serialize(msg);

	// Write the number of ancestors
	msg->Write(static_cast<uint32>(m_hierarchy.size()));

	// Write the GUIDs of the ancestors
	for (size_t i = 0; i < m_hierarchy.size(); ++i)
	{
		msg->Write(m_hierarchy[i]);
	}
}


void ModifyOperation::Deserialize(NetworkInMessage& msg)
{
	m_elementGuid = msg.ReadInt64();
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


void ModifyOperation::Notify(const SyncContextPtr& context) const
{
	// Check that the element has not been deleted since this op was applied
	ElementPtr modifiedElement = context->GetElement(m_elementGuid);
	if (modifiedElement)
	{
		// Check that the parent element has not been deleted since this op was applied
		ElementPtr parentElement = modifiedElement->GetParent();
		if (parentElement)
		{
			ObjectElementImpl* parentObjectElement = reflection_cast<ObjectElementImpl>(parentElement);
			if (XTVERIFY(parentObjectElement))
			{
				// Notify the listener that the value of the element has changed
				switch (m_newValue.GetType())
				{
				case XValue::Bool:
				{
					bool boolValue = *(m_newValue.Get<bool>());
					parentObjectElement->GetListeners()->NotifyListeners(&ObjectElementListener::OnBoolElementChanged, m_elementGuid, boolValue);
					break;
				}

				case XValue::Int:
				{
					int32 intValue = *(m_newValue.Get<int32>());
					parentObjectElement->GetListeners()->NotifyListeners(&ObjectElementListener::OnIntElementChanged, m_elementGuid, intValue);
					break;
				}

				case XValue::Int64:
				{
					int64 int64Value = *(m_newValue.Get<int64>());
					parentObjectElement->GetListeners()->NotifyListeners(&ObjectElementListener::OnLongElementChanged, m_elementGuid, int64Value);
					break;
				}

				case XValue::Float:
				{
					float floatValue = *(m_newValue.Get<float>());
					parentObjectElement->GetListeners()->NotifyListeners(&ObjectElementListener::OnFloatElementChanged, m_elementGuid, floatValue);
					break;
				}

				case XValue::Double:
				{
					double doubleValue = *(m_newValue.Get<double>());
					parentObjectElement->GetListeners()->NotifyListeners(&ObjectElementListener::OnDoubleElementChanged, m_elementGuid, doubleValue);
					break;
				}

				case XValue::String:
				{
					XStringPtr stringValue = new XString(*(m_newValue.Get<std::string>()));
					parentObjectElement->GetListeners()->NotifyListeners(&ObjectElementListener::OnStringElementChanged, m_elementGuid, stringValue);
					break;
				}

				case XValue::UInt:
					// Currently this is only used for setting the ownerID on ObjectElements, and the user is 
					// not provided with a callback for when that happens
					break;

				default:
					XTASSERT(false);
					break;
				}
			}
		}
	}
}


std::string ModifyOperation::GetOpDescription() const
{
	char buffer[512];

	sprintf_s(buffer, sizeof(buffer), "%s, ID: %lli, Value: %s", GetTypeName(), m_elementGuid, m_newValue.ToString().c_str());

	return buffer;
}


const XValue& ModifyOperation::GetValue() const
{
	return m_newValue;
}

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
