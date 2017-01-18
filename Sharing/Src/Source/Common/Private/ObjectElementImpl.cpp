//////////////////////////////////////////////////////////////////////////
// ObjectElementImpl.cpp
//
// Implements the ObjectElement interface
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ObjectElementImpl.h"
#include "CreateOperation.h"
#include "DeleteOperation.h"

XTOOLS_NAMESPACE_BEGIN

XTOOLS_REFLECTION_DEFINE(Element)
.BaseClass<Reflection::XTObject>();

XTOOLS_REFLECTION_DEFINE(ObjectElement)
.BaseClass<Element>();

// If the given element is an Object, cast it to the derived type.  Otherwise return null
ObjectElementPtr ObjectElement::Cast(const ElementPtr& element)
{
	if (element != NULL && element->GetElementType() == ElementType::ObjectType)
	{
		return static_cast<ObjectElement*>(element.get());
	}
	else
	{
		return NULL;
	}
}

NAMESPACE_BEGIN(Sync)

XTOOLS_REFLECTION_DEFINE(ObjectElementImpl)
.BaseClass<ObjectElement>();

ObjectElementImpl::ObjectElementImpl(SyncContext* syncContext, const XStringPtr& name, XGuid id, UserID ownerID, const XValue& typeValue)
	: m_syncContext(syncContext)
	, m_parent(NULL)
	, m_name(name)
	, m_guid(id)
	, m_listenerList(ListenerList::Create())
	, m_ownerID(ownerID)
{
	XTASSERT(name);

	const std::string* typeString = typeValue.Get<std::string>();
	if (XTVERIFY(typeString))
	{
		m_objectType = new XString(*typeString);
	}

#if defined(SYNC_DEBUG)
	LogInfo("%s Created object %s with owner %i", syncContext->GetLocalUser()->GetName().GetString().c_str(), m_name.GetString().c_str(), m_ownerID);
#endif
}


BoolElementPtr ObjectElementImpl::CreateBoolElement(const XStringPtr& name, bool value)
{
	if (!name)
	{
		LogError("Null name passed to CreateBoolElement");
		return NULL;
	}

	ElementPtr newElement = CreateElement(ElementType::BoolType, name, User::kInvalidUserID, value);
	return BoolElement::Cast(newElement);
}


IntElementPtr ObjectElementImpl::CreateIntElement(const XStringPtr& name, int value)
{
	if (!name)
	{
		LogError("Null name passed to CreateIntElement");
		return NULL;
	}

	ElementPtr newElement = CreateElement(ElementType::Int32Type, name, User::kInvalidUserID, value);
	return IntElement::Cast(newElement);
}


LongElementPtr ObjectElementImpl::CreateLongElement(const XStringPtr& name, int64 value)
{
	if (!name)
	{
		LogError("Null name passed to CreateLongElement");
		return NULL;
	}

	ElementPtr newElement = CreateElement(ElementType::Int64Type, name, User::kInvalidUserID, value);
	return LongElement::Cast(newElement);
}


FloatElementPtr ObjectElementImpl::CreateFloatElement(const XStringPtr& name, float value)
{
	if (!name)
	{
		LogError("Null name passed to CreateFloatElement");
		return NULL;
	}

	ElementPtr newElement = CreateElement(ElementType::FloatType, name, User::kInvalidUserID, value);
	return FloatElement::Cast(newElement);
}


DoubleElementPtr ObjectElementImpl::CreateDoubleElement(const XStringPtr& name, double value)
{
	if (!name)
	{
		LogError("Null name passed to CreateDoubleElement");
		return NULL;
	}

	ElementPtr newElement = CreateElement(ElementType::DoubleType, name, User::kInvalidUserID, value);
	return DoubleElement::Cast(newElement);
}


StringElementPtr ObjectElementImpl::CreateStringElement(const XStringPtr& name, const XStringPtr& value)
{
	if (!name)
	{
		LogError("Null name passed to CreateStringElement");
		return NULL;
	}

	if (!value)
	{
		LogError("Null value passed to CreateStringElement");
		return NULL;
	}

	ElementPtr newElement = CreateElement(ElementType::StringType, name, User::kInvalidUserID, value->GetString());
	return StringElement::Cast(newElement);
}


ObjectElementPtr ObjectElementImpl::CreateObjectElement(const XStringPtr& name, const XStringPtr& objectType, const User* user)
{
	if (!name)
	{
		LogError("Null name passed to CreateObjectElement");
		return NULL;
	}

	UserID userID = User::kInvalidUserID;
	if (user)
	{
		userID = user->GetID();
	}

	ElementPtr newElement = CreateElement(ElementType::ObjectType, name, userID, objectType->GetString());
	return ObjectElement::Cast(newElement);
}


IntArrayElementPtr ObjectElementImpl::CreateIntArrayElement(const XStringPtr& name)
{
	if (!name)
	{
		LogError("Null name passed to CreateIntArrayElement");
		return NULL;
	}

	ElementPtr newElement = CreateElement(ElementType::Int32ArrayType, name, User::kInvalidUserID, XValue());
	return IntArrayElement::Cast(newElement);
}


FloatArrayElementPtr ObjectElementImpl::CreateFloatArrayElement(const XStringPtr& name)
{
	if (!name)
	{
		LogError("Null name passed to CreateFloatArrayElement");
		return NULL;
	}

	ElementPtr newElement = CreateElement(ElementType::FloatArrayType, name, User::kInvalidUserID, XValue());
	return FloatArrayElement::Cast(newElement);
}


StringArrayElementPtr ObjectElementImpl::CreateStringArrayElement(const XStringPtr& name)
{
	if (!name)
	{
		LogError("Null name passed to CreateStringArrayElement");
		return NULL;
	}

	ElementPtr newElement = CreateElement(ElementType::StringArrayType, name, User::kInvalidUserID, XValue());
	return StringArrayElement::Cast(newElement);
}


int32 ObjectElementImpl::GetElementCount() const
{
	return static_cast<int32>(m_children.size());
}


ElementPtr ObjectElementImpl::GetElement(XGuid id) const
{
	for (size_t i = 0; i < m_children.size(); ++i)
	{
		if (m_children[i]->GetGUID() == id)
		{
			return m_children[i];
		}
	}

	return NULL;
}


ElementPtr ObjectElementImpl::GetElement(const XStringPtr& name) const
{
	if (!name)
	{
		LogWarning("Null name passed to GetElement");
		return NULL;
	}

	for (size_t i = 0; i < m_children.size(); ++i)
	{
		if (m_children[i]->GetName()->IsEqual(name))
		{
			return m_children[i];
		}
	}

	return NULL;
}


ElementPtr ObjectElementImpl::GetElementAt(int32 index) const
{
	if (index < (int32)m_children.size())
	{
		return m_children[index];
	}

	return NULL;
}


void ObjectElementImpl::RemoveElement(const ElementPtr& element)
{
	if (!element)
	{
		LogError("Attempting to remove NULL element from object element %s", m_name->GetString().c_str());
	}
	else if (!IsValid())
	{
		LogWarning("Attempting to remove element %s from invalid object element %s", element->GetName()->GetString().c_str(), m_name->GetString().c_str());
	}
	else if (element->GetParent() != this)
	{
		LogError("Attempting to remove element %s from object element %s which is not its parent", element->GetName()->GetString().c_str(), m_name->GetString().c_str());
	}
	else
	{
		// Create a delete operation to represent this change
		DeleteOperationPtr deleteOp = new DeleteOperation(element->GetGUID(), m_guid, m_syncContext);

		// Execute the operation
		deleteOp->Apply(m_syncContext);

		// Add the op to the list of ops applied sync the last sync
		m_syncContext->AddAppliedOperation(deleteOp);
	}
}


void ObjectElementImpl::RemoveElement(XGuid id)
{
	if (XTVERIFY(id != kInvalidXGuid))
	{
		ElementPtr element = m_syncContext->GetElement(id);
		RemoveElement(element.get());
	}
}


void ObjectElementImpl::RemoveElementAt(int32 index)
{
	RemoveElement(GetElementAt(index));
}


void ObjectElementImpl::AddListener(ObjectElementListener* newListener)
{
	if (!IsValid())
	{
		LogWarning("Attempting to add a listener to invalid object element %s", m_name->GetString().c_str());
	}

	m_listenerList->AddListener(newListener);
}


void ObjectElementImpl::RemoveListener(ObjectElementListener* oldListener)
{
	m_listenerList->RemoveListener(oldListener);
}


UserID ObjectElementImpl::GetOwnerID() const
{
	return m_ownerID;
}


const XStringPtr& ObjectElementImpl::GetObjectType() const
{
	return m_objectType;
}


ElementType ObjectElementImpl::GetElementType() const
{
	return ElementType::ObjectType;
}


XGuid ObjectElementImpl::GetGUID() const
{
	return m_guid;
}


const XStringPtr& ObjectElementImpl::GetName() const
{
	return m_name;
}


// Returns a pointer to this Element's parent Element.
// If this is the root element, then it returns NULL
ElementPtr ObjectElementImpl::GetParent() const
{
	return m_parent;
}


bool ObjectElementImpl::IsValid() const
{
	return (m_syncContext && m_syncContext->ElementExists(m_guid));
}


XValue ObjectElementImpl::GetXValue() const
{
	return m_objectType;
}


void ObjectElementImpl::SetXValue(const XValue& value)
{
	XTASSERT(IsValid());

	const std::string* typeString = value.Get<std::string>();
	if (XTVERIFY(typeString))
	{
		m_objectType = new XString(*typeString);
	}
}


void ObjectElementImpl::SetParent(Element* parent)
{
	m_parent = parent;
}


const ObjectElementImpl::ListenerListPtr& ObjectElementImpl::GetListeners() const
{
	return m_listenerList;
}


void ObjectElementImpl::AddChild(const ElementPtr& newElement)
{
	XTASSERT(IsValid());
	XTASSERT(newElement);
	XTASSERT(newElement->IsValid());

	m_children.push_back(newElement);
	newElement->SetParent(this);
}


void ObjectElementImpl::RemoveChild(const ElementPtr& element)
{
	XTASSERT(IsValid());
	XTASSERT(element);
	XTASSERT(element->IsValid());
	XTASSERT(element->GetParent() == this);

	element->SetParent(NULL);

	for (size_t i = 0; i < m_children.size(); ++i)
	{
		if (m_children[i] == element)
		{
			m_children.erase(m_children.begin() + i);
			break;
		}
	}
}


ElementPtr ObjectElementImpl::CreateElement(ElementType type, const XStringPtr& name, UserID ownerID, const XValue& value)
{
	XTASSERT(name);

	if (!IsValid())
	{
		LogError("Cannot create new element %s from an invalid element %s", name->GetString().c_str(), m_name->GetString().c_str());
		return NULL;
	}

	// Create a GUID from the full path of the new element.  
	XGuid elementGuid = m_syncContext->CreateGUID(name, m_guid);
	XTASSERT(elementGuid != kInvalidXGuid);

	// Check to make sure we aren't trying to create an element that already exists
	if (m_syncContext->ElementExists(elementGuid))
	{
		LogError("Cannot create new element %s because it already exists in the sync dataset", name->GetString().c_str());
		m_syncContext->PrintSyncDataTree();
		return NULL;
	}

	// Create an Operation to represent this change
	CreateOperationPtr createOp = new CreateOperation(type, name, elementGuid, m_guid, ownerID, value, m_syncContext->GetAuthorityLevel(), m_syncContext);

	// Execute the creation operations
	createOp->Apply(m_syncContext);

	// Add the op to the list of ops applied sync the last sync
	m_syncContext->AddAppliedOperation(createOp);

	// Return the newly created element
	return m_syncContext->GetElement(createOp->GetTargetGuid());
}

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END