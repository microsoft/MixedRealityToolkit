//////////////////////////////////////////////////////////////////////////
// SyncContext.cpp
//
// Contains common data used by multiple parts of the sync process
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SyncContext.h"
#include "ObjectElementImpl.h"
#include "BoolElementImpl.h"
#include "IntElementImpl.h"
#include "LongElementImpl.h"
#include "FloatElementImpl.h"
#include "DoubleElementImpl.h"
#include "StringElementImpl.h"
#include "IntArrayElementImpl.h"
#include "FloatArrayElementImpl.h"
#include "StringArrayElementImpl.h"

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

const char* kElementTypeNames[ElementType::ElementTypeCount] =
{
	"UnknownType",
	"BoolType",
	"Int32Type",
	"Int64Type",
	"FloatType",
	"DoubleType",
	"StringType",
	"ObjectType",
	"Int32ArrayType",
	"FloatArrayType",
	"StringArrayType"
};

SyncContext::SyncContext(AuthorityLevel authorityLevel, SystemID localSystemID, const UserPtr& localUser)
: m_authorityLevel(authorityLevel)
, m_localSystemID(localSystemID)
, m_localUser(localUser)
{
	// Create the root object
	XStringPtr rootObjectName = new XString("Root");
	XGuid rootObjectGuid = CreateGUID(rootObjectName, kInvalidXGuid);
	m_rootObject = new ObjectElementImpl(this, rootObjectName, rootObjectGuid, User::kInvalidUserID, XValue(std::string("Root")));
	m_guidMap[rootObjectGuid] = m_rootObject.get();

	// Add makers for all the operations to the operation factory
	m_opFactory.RegisterMaker(Operation::Ack, new OpMakerT<AckOperation>());
	m_opFactory.RegisterMaker(Operation::Noop, new OpMakerT<NoopOperation>());
	m_opFactory.RegisterMaker(Operation::Create, new OpMakerT<CreateOperation>());
	m_opFactory.RegisterMaker(Operation::Modify, new OpMakerT<ModifyOperation>());
	m_opFactory.RegisterMaker(Operation::Delete, new OpMakerT<DeleteOperation>());
	m_opFactory.RegisterMaker(Operation::Replace, new OpMakerT<ReplaceOperation>());
	m_opFactory.RegisterMaker(Operation::Insert, new OpMakerT<InsertOperation>());
	m_opFactory.RegisterMaker(Operation::Update, new OpMakerT<UpdateOperation>());
	m_opFactory.RegisterMaker(Operation::Remove, new OpMakerT<RemoveOperation>());

	// Add makers for all the element types
	m_elementFactory.RegisterMaker(ElementType::BoolType, new ElementMakerT<BoolElementImpl>());
	m_elementFactory.RegisterMaker(ElementType::Int32Type, new ElementMakerT<IntElementImpl>());
	m_elementFactory.RegisterMaker(ElementType::Int64Type, new ElementMakerT<LongElementImpl>());
	m_elementFactory.RegisterMaker(ElementType::FloatType, new ElementMakerT<FloatElementImpl>());
	m_elementFactory.RegisterMaker(ElementType::DoubleType, new ElementMakerT<DoubleElementImpl>());
	m_elementFactory.RegisterMaker(ElementType::StringType, new ElementMakerT<StringElementImpl>());
	m_elementFactory.RegisterMaker(ElementType::ObjectType, new ElementMakerWithOwnerT<ObjectElementImpl>());
	m_elementFactory.RegisterMaker(ElementType::Int32ArrayType, new ElementMakerT<IntArrayElementImpl>());
	m_elementFactory.RegisterMaker(ElementType::FloatArrayType, new ElementMakerT<FloatArrayElementImpl>());
	m_elementFactory.RegisterMaker(ElementType::StringArrayType, new ElementMakerT<StringArrayElementImpl>());
}


ObjectElementPtr SyncContext::GetRootObject() const
{
	return m_rootObject;
}


const TransformManager& SyncContext::GetTransformManager() const
{
	return m_transformMgr;
}


const OperationFactory& SyncContext::GetOpFactory() const
{
	return m_opFactory;
}


void SyncContext::AddAppliedOperation(const OperationPtr& op)
{
	// Special case for delete ops: 

	m_appliedChanges.push_back(op);
}


const OperationList& SyncContext::GetAppliedOperations() const
{
	return m_appliedChanges;
}


void SyncContext::ClearAppliedOperations()
{
	m_appliedChanges.clear();
}


bool SyncContext::ElementExists(XGuid id) const
{
	auto elementIter = m_guidMap.find(id);
	if (elementIter != m_guidMap.end() && elementIter->second != NULL)
	{
		return true;
	}
	else
	{
		return false;
	}
}


ElementPtr SyncContext::GetElement(XGuid id) const
{
	auto elementIter = m_guidMap.find(id);
	if (elementIter != m_guidMap.end())
	{
		return elementIter->second;
	}
	else
	{
		return NULL;
	}
}


SystemID SyncContext::GetLocalSystemID() const
{
	return m_localSystemID;
}


AuthorityLevel SyncContext::GetAuthorityLevel() const
{
	return m_authorityLevel;
}


ElementPtr SyncContext::CreateElement(ElementType type, const XStringPtr& name, XGuid guid, XGuid parentGuid, UserID ownerID, XValue startingValue)
{
	XTASSERT(type != ElementType::UnknownType);
	XTASSERT(guid != kInvalidXGuid);
	XTASSERT(name);

	// Since this is a Create function, the GUID for this newly created element should be unique.
	// So verify that the GUID is not already in use.  
	XTASSERT(m_guidMap.find(guid) == m_guidMap.end());

	ElementPtr newElement = m_elementFactory.Make(type, this, name, guid, ownerID, startingValue);
	XTASSERT(newElement);
	
	m_guidMap[guid] = newElement.get();

	ElementPtr parentElement = GetElement(parentGuid);
	if (parentElement)
	{
		ObjectElementPtr parentObjElement = ObjectElement::Cast(parentElement);
		if (XTVERIFY(parentObjElement))
		{
			ObjectElementImpl* parentImpl = static_cast<ObjectElementImpl*>(parentObjElement.get());
			parentImpl->AddChild(newElement);
		}
	}

	return newElement;
}


void SyncContext::DeleteElement(const ElementPtr& element)
{
	// Remove the element from its parent
	ElementPtr parent = element->GetParent();
	if (XTVERIFY(parent))
	{
		ObjectElementPtr objElement = ObjectElement::Cast(parent);
		if (XTVERIFY(objElement))
		{
			ObjectElementImpl* objElementImp = static_cast<ObjectElementImpl*>(objElement.get());
			objElementImp->RemoveChild(element);
		}
	}

	// Remove the element and its children from the sync data set
	DeleteElementRecurs(element);
}


std::string SyncContext::GetElementPath(XGuid id) const
{
	std::string path;
	GetElementPathRecurs(id, path);
	return path;
}


XGuid SyncContext::CreateGUID(const XStringPtr& name, XGuid parent) const
{
	XTASSERT(name);

	std::string path;

	if (parent != kInvalidXGuid)
	{
		path = GetElementPath(parent);
	}
	
	path += "/";
	path += name->GetString();

	uint64 hashID = HashString(path.c_str());

	XGuid newGuid = *reinterpret_cast<XGuid*>(&hashID);

	return newGuid;
}


std::vector<XGuid> SyncContext::GetHierarchy(XGuid startingID) const
{
	std::vector<XGuid> hierarchy;

	ElementConstPtr currentElement = GetElement(startingID);

	while (currentElement)
	{
		XTASSERT(currentElement->IsValid());

		hierarchy.push_back(currentElement->GetGUID());
		currentElement = currentElement->GetParent();
	}

	return hierarchy;
}


const UserPtr& SyncContext::GetLocalUser() const
{
	return m_localUser;
}


void SyncContext::PrintSyncDataTree() const
{
	PrintElementRecurs(GetRootObject(), 0);
}


void SyncContext::PrintElementRecurs(const ElementConstPtr& element, int depth) const
{
	char preamble[512];
	int offset = 0;
	for (int i = 0; i < depth; ++i)
	{
		preamble[offset++] = '|';
		preamble[offset++] = ' ';
	}

	//preamble[offset++] = '-';
	preamble[offset++] = '\0';

	const ArrayElement* arrayElement = reflection_cast<const ArrayElement>(element.get());
	if (arrayElement)
	{
		LogInfo("%s%s (%s)", preamble, element->GetName()->GetString().c_str(), kElementTypeNames[element->GetElementType()]);
	}
	else
	{
		LogInfo("%s%s (%s, %s)", preamble, element->GetName()->GetString().c_str(), kElementTypeNames[element->GetElementType()], element->GetXValue().ToString().c_str());
	}
	

	if (element->GetElementType() == ElementType::ObjectType)
	{
		const ObjectElement* objectElement = static_cast<const ObjectElement*>(element.get());
		if (objectElement)
		{
			for (int32 i = objectElement->GetElementCount() - 1; i >= 0; --i)
			{
				PrintElementRecurs(objectElement->GetElementAt(i), depth + 1);
			}
		}
	}
	else if (arrayElement != nullptr)
	{
		int32 numArrayElements = arrayElement->GetCount();
		for (int i = 0; i < numArrayElements; ++i)
		{
			LogInfo("%s %i) %s", preamble, i, arrayElement->GetXValue(i).ToString().c_str());
		}
	}
}


void SyncContext::GetElementPathRecurs(XGuid id, std::string& pathOut) const
{
	ElementPtr element = GetElement(id);
	if (XTVERIFY(element))
	{
		ElementPtr parent = element->GetParent();
		if (parent)
		{
			GetElementPathRecurs(parent->GetGUID(), pathOut);
		}
	
		pathOut += "/";
		pathOut += element->GetName()->GetString();
	}
}


void SyncContext::DeleteElementRecurs(const ElementPtr& element)
{
	XTASSERT(element);

	// Delete the children first
	ObjectElementPtr objElement = ObjectElement::Cast(element);
	if (objElement)
	{
		for (int32 i = objElement->GetElementCount()-1; i >= 0; --i)
		{
			DeleteElementRecurs(objElement->GetElementAt(i));
		}
	}

	XTASSERT(m_guidMap[element->GetGUID()] == element);
	m_guidMap.erase(element->GetGUID());
}

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
