//////////////////////////////////////////////////////////////////////////
// SyncTestObject.cpp
//
// Object used to test the sync system
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SyncTestObject.h"

SyncTestObject::SyncTestObject(const ObjectElementPtr& element, bool bCreatedLocally)
	: m_name(element->GetName()->GetString())
	, m_element(element)
	, m_boolMember(false)
	, m_floatMember(0.f)
	, m_intMember(0)
	, m_stringMember("TestString")
	, m_longMember(0L)
	, m_doubleMember(0.0)
	, m_incomingIntChangeCount(0)
	, m_incomingFloatChangeCount(0)
	, m_incomingStringChangeCount(0)
	, m_incomingAddCount(0)
	, m_incomingRemoveCount(0)
{
	m_element->AddListener(this);

	if (bCreatedLocally && m_element->GetName()->GetString() != std::string("Root"))
	{
		m_boolElement = m_element->CreateBoolElement(new XString("boolMember"), m_boolMember);
		m_floatElement = m_element->CreateFloatElement(new XString("floatMember"), m_floatMember);
		m_intElement = m_element->CreateIntElement(new XString("intMember"), m_intMember);
		m_stringElement = m_element->CreateStringElement(new XString("stringMember"), new XString(m_stringMember));
		m_longElement = m_element->CreateLongElement(new XString("longMember"), m_longMember);
		m_doubleElement = m_element->CreateDoubleElement(new XString("doubleMember"), m_doubleMember);

		m_intArrayElement = m_element->CreateIntArrayElement(new XString("intArrayMember"));
		m_intArrayElement->AddListener(this);

		m_floatArrayElement = m_element->CreateFloatArrayElement(new XString("floatArrayMember"));
		m_floatArrayElement->AddListener(this);

		m_stringArrayElement = m_element->CreateStringArrayElement(new XString("stringArrayMember"));
		m_stringArrayElement->AddListener(this);
	}
}


ref_ptr<SyncTestObject> SyncTestObject::AddChild(const std::string& name)
{
	ObjectElementPtr childElement = m_element->CreateObjectElement(new XString(name), new XString("SyncTestObject"));

	SyncTestObject* newChild = new SyncTestObject(childElement, true);

	m_children.push_back(newChild);

	return newChild;
}


void SyncTestObject::RemoveChild(const std::string& name)
{
	for (size_t i = 0; i < m_children.size(); ++i)
	{
		if (m_children[i]->GetName() == name)
		{
			m_element->RemoveElement(m_children[i]->m_element.get());
			m_children.erase(m_children.begin() + i);
			break;
		}
	}
}


const ObjectElementPtr& SyncTestObject::GetElement() const
{
	return m_element;
}


ref_ptr<SyncTestObject> SyncTestObject::GetChild(int index)
{
	return m_children[index];
}


ref_ptr<SyncTestObject> SyncTestObject::GetChild(const std::string& name)
{
	for (size_t i = 0; i < m_children.size(); ++i)
	{
		if (m_children[i]->GetName() == name)
		{
			return m_children[i];
		}
	}

	return NULL;
}


std::string SyncTestObject::GetName() const
{
	return m_element->GetName()->GetString();
}


float SyncTestObject::GetFloatValue() const
{ 
	return m_floatMember; 
}


void SyncTestObject::SetFloatValue(float value)
{
	m_floatMember = value;
	m_floatElement->SetValue(value);
}


void SyncTestObject::RemoveFloatValue()
{
	m_element->RemoveElement(m_floatElement.get());
	m_floatElement = NULL;
}


bool SyncTestObject::IsFloatValid() const
{
	return (m_floatElement != NULL);
}


int32 SyncTestObject::GetIntValue() const
{
	return m_intMember;
}


void SyncTestObject::SetIntValue(int32 value)
{
	m_intMember = value;
	m_intElement->SetValue(value);
}


void SyncTestObject::RemoveIntValue()
{
	m_element->RemoveElement(m_intElement.get());
	m_intElement = NULL;
}


std::string SyncTestObject::GetStringValue() const
{
	return m_stringMember;
}


void SyncTestObject::SetStringValue(const std::string& value)
{
	m_stringMember = value;
	m_stringElement->SetValue(new XString(m_stringMember));
}


void SyncTestObject::RemoveStringValue()
{
	m_element->RemoveElement(m_stringElement.get());
	m_stringElement = NULL;
}


int64 SyncTestObject::GetLongValue() const
{
	return m_longMember;
}


void SyncTestObject::SetLongValue(int64 value)
{
	m_longMember = value;
	m_longElement->SetValue(value);
}


void SyncTestObject::RemoveLongValue()
{
	m_element->RemoveElement(m_longElement.get());
	m_longElement = NULL;
}


double SyncTestObject::GetDoubleValue() const
{
	return m_doubleMember;
}


void SyncTestObject::SetDoubleValue(double value)
{
	m_doubleMember = value;
	m_doubleElement->SetValue(value);
}


void SyncTestObject::RemoveDoubleValue()
{
	m_element->RemoveElement(m_doubleElement.get());
	m_doubleElement = NULL;
}


int32 SyncTestObject::GetIntArrayLength() const
{
	return (int32)m_intArrayMember.size();
}


int32 SyncTestObject::GetIntArrayValue(int32 index) const
{
	return m_intArrayMember[index];
}


void SyncTestObject::SetIntArrayValue(int32 index, int32 value)
{
	m_intArrayMember[index] = value;
	m_intArrayElement->SetValue(index, value);
}


void SyncTestObject::InsertIntArrayValue(int32 index, int32 value)
{
	m_intArrayMember.insert(m_intArrayMember.begin() + index, value);
	m_intArrayElement->InsertValue(index, value);
}


void SyncTestObject::RemoveIntArrayValue(int32 index)
{
	m_intArrayMember.erase(m_intArrayMember.begin() + index);
	m_intArrayElement->RemoveValue(index);
}


void SyncTestObject::SetFloatArrayValue(int32 index, float value)
{
	m_floatArrayMember[index] = value;
	m_floatArrayElement->SetValue(index, value);
}


void SyncTestObject::InsertFloatArrayValue(int32 index, float value)
{
	m_floatArrayMember.insert(m_floatArrayMember.begin() + index, value);
	m_floatArrayElement->InsertValue(index, value);
}


void SyncTestObject::RemoveFloatArrayValue(int32 index)
{
	m_floatArrayMember.erase(m_floatArrayMember.begin() + index);
	m_floatArrayElement->RemoveValue(index);
}


void SyncTestObject::SetStringArrayValue(int32 index, const XStringPtr& value)
{
	m_stringArrayMember[index] = value;
	m_stringArrayElement->SetValue(index, value);
}


void SyncTestObject::InsertStringArrayValue(int32 index, const XStringPtr& value)
{
	m_stringArrayMember.insert(m_stringArrayMember.begin() + index, value);
	m_stringArrayElement->InsertValue(index, value);
}


void SyncTestObject::RemoveStringArrayValue(int32 index)
{
	m_stringArrayMember.erase(m_stringArrayMember.begin() + index);
	m_stringArrayElement->RemoveValue(index);
}


bool SyncTestObject::Equals(const ref_ptr<const SyncTestObject>& otherObj) const
{
	if (!m_element->IsValid())
	{
		return false;
	}

	if (m_name != otherObj->m_name || // Check the name
		m_element->GetGUID() != otherObj->m_element->GetGUID() // Check the ID
		)
	{
		return false;
	}

	if ((m_boolElement == NULL) != (otherObj->m_boolElement == NULL)) return false;
	if (m_boolElement != NULL)
	{
		if (otherObj->m_boolElement == NULL ||
			m_boolMember != otherObj->m_boolMember ||
			m_boolElement->GetGUID() != otherObj->m_boolElement->GetGUID() ||
			!m_boolElement->GetName()->IsEqual(otherObj->m_boolElement->GetName()))
		{
			return false;
		}
	}

	if ((m_floatElement == NULL) != (otherObj->m_floatElement == NULL)) return false;
	if (m_floatElement != NULL)
	{
		if (otherObj->m_floatElement == NULL ||
			m_floatMember != otherObj->m_floatMember ||
			m_floatElement->GetGUID() != otherObj->m_floatElement->GetGUID() ||
			!m_floatElement->GetName()->IsEqual(otherObj->m_floatElement->GetName()))
		{
			return false;
		}
	}

	if ((m_intElement == NULL) != (otherObj->m_intElement == NULL)) return false;
	if (m_intElement != NULL)
	{
		if (otherObj->m_intElement == NULL ||
			m_intMember != otherObj->m_intMember ||
			m_intElement->GetGUID() != otherObj->m_intElement->GetGUID() ||
			!m_intElement->GetName()->IsEqual(otherObj->m_intElement->GetName()))
		{
			return false;
		}
	}

	if ((m_stringElement == NULL) != (otherObj->m_stringElement == NULL)) return false;
	if (m_stringElement != NULL)
	{
		if (otherObj->m_stringElement == NULL ||
			m_stringMember != otherObj->m_stringMember ||
			m_stringElement->GetGUID() != otherObj->m_stringElement->GetGUID() ||
			!m_stringElement->GetName()->IsEqual(otherObj->m_stringElement->GetName()))
		{
			return false;
		}
	}

	if ((m_longElement == NULL) != (otherObj->m_longElement == NULL)) return false;
	if (m_longElement != NULL)
	{
		if (otherObj->m_longElement == NULL ||
			m_longMember != otherObj->m_longMember ||
			m_longElement->GetGUID() != otherObj->m_longElement->GetGUID() ||
			!m_longElement->GetName()->IsEqual(otherObj->m_longElement->GetName()))
		{
			return false;
		}
	}

	if ((m_doubleElement == NULL) != (otherObj->m_doubleElement == NULL)) return false;
	if (m_doubleElement != NULL)
	{
		if (otherObj->m_doubleElement == NULL ||
			m_doubleMember != otherObj->m_doubleMember ||
			m_doubleElement->GetGUID() != otherObj->m_doubleElement->GetGUID() ||
			!m_doubleElement->GetName()->IsEqual(otherObj->m_doubleElement->GetName()))
		{
			return false;
		}
	}

	if ((m_intArrayElement == NULL) != (otherObj->m_intArrayElement == NULL)) return false;
	if (m_intArrayElement != NULL)
	{
		if (otherObj->m_intArrayElement == NULL ||
			m_intArrayElement->GetGUID() != otherObj->m_intArrayElement->GetGUID() ||
			!m_intArrayElement->GetName()->IsEqual(otherObj->m_intArrayElement->GetName()))
		{
			return false;
		}

		if (m_intArrayMember.size() != otherObj->m_intArrayMember.size())
		{
			return false;
		}

		for (size_t i = 0; i < m_intArrayMember.size(); ++i)
		{
			if (m_intArrayMember[i] != otherObj->m_intArrayMember[i])
			{
				return false;
			}
		}
	}


	if ((m_floatArrayElement == NULL) != (otherObj->m_floatArrayElement == NULL)) return false;
	if (m_floatArrayElement != NULL)
	{
		if (otherObj->m_floatArrayElement == NULL ||
			m_floatArrayElement->GetGUID() != otherObj->m_floatArrayElement->GetGUID() ||
			!m_floatArrayElement->GetName()->IsEqual(otherObj->m_floatArrayElement->GetName()))
		{
			return false;
		}

		if (m_floatArrayMember.size() != otherObj->m_floatArrayMember.size())
		{
			return false;
		}

		for (size_t i = 0; i < m_floatArrayMember.size(); ++i)
		{
			if (m_floatArrayMember[i] != otherObj->m_floatArrayMember[i])
			{
				return false;
			}
		}
	}

	if ((m_stringArrayElement == NULL) != (otherObj->m_stringArrayElement == NULL)) return false;
	if (m_stringArrayElement != NULL)
	{
		if (otherObj->m_stringArrayElement == NULL ||
			m_stringArrayElement->GetGUID() != otherObj->m_stringArrayElement->GetGUID() ||
			!m_stringArrayElement->GetName()->IsEqual(otherObj->m_stringArrayElement->GetName()))
		{
			return false;
		}

		if (m_stringArrayMember.size() != otherObj->m_stringArrayMember.size())
		{
			return false;
		}

		for (size_t i = 0; i < m_stringArrayMember.size(); ++i)
		{
			if (m_stringArrayMember[i]->GetString() != otherObj->m_stringArrayMember[i]->GetString())
			{
				return false;
			}
		}
	}

	// Check that the number of children is the same
	if (m_children.size() != otherObj->m_children.size()) return false;

	// Check the children
	for (size_t i = 0; i < m_children.size(); ++i)
	{
		XGuid myChildGUID = m_children[i]->m_element->GetGUID();
		bool found = false;

		// Note: the children can be in different orders in each copy, which is ok.
		// So we have to look for the matching children in the two objects
		for (size_t j = 0; j < otherObj->m_children.size(); ++j)
		{
			XGuid otherChildGuid = otherObj->m_children[j]->m_element->GetGUID();
			if (myChildGUID == otherChildGuid)
			{
				if (!m_children[i]->Equals(otherObj->m_children[j]))
				{
					return false;
				}

				found = true;

				break;
			}
		}

		if (!found)
		{
			return false;
		}
	}

	return true;
}


void SyncTestObject::OnBoolElementChanged(XGuid elementID, bool newValue)
{
	XTASSERT(m_boolElement);
	XTASSERT(m_boolElement->GetGUID() == elementID);

	m_boolMember = newValue;
}


void SyncTestObject::OnIntElementChanged(XGuid elementID, int32 newValue)
{
	XTASSERT(m_intElement);
	XTASSERT(m_intElement->GetGUID() == elementID);

	m_intMember = newValue;
	++m_incomingIntChangeCount;
}


void SyncTestObject::OnFloatElementChanged(XGuid elementID, float newValue)
{
	XTASSERT(m_floatElement);
	XTASSERT(m_floatElement->GetGUID() == elementID);

	m_floatMember = newValue;
	++m_incomingFloatChangeCount;
}


void SyncTestObject::OnStringElementChanged(XGuid elementID, const XStringPtr& newValue)
{
	XTASSERT(m_stringElement);
	XTASSERT(m_stringElement->GetGUID() == elementID);
	XTASSERT(newValue);

	m_stringMember = newValue->GetString();
	++m_incomingStringChangeCount;
}


void SyncTestObject::OnLongElementChanged(XGuid elementID, int64 newValue)
{
	XTASSERT(m_longElement);
	XTASSERT(m_longElement->GetGUID() == elementID);

	m_longMember = newValue;
}


void SyncTestObject::OnDoubleElementChanged(XGuid elementID, double newValue)
{
	XTASSERT(m_doubleElement);
	XTASSERT(m_doubleElement->GetGUID() == elementID);

	m_doubleMember = newValue;
}


void SyncTestObject::OnElementAdded(const ElementPtr& element)
{
	++m_incomingAddCount;

	if (element->GetElementType() == ElementType::ObjectType)
	{
		if (element->IsValid())
		{
			ObjectElementPtr objElement = ObjectElement::Cast(element);
			XTASSERT(objElement);

			if (objElement->GetObjectType()->GetString() == "SyncTestObject")
			{
				m_children.push_back(new SyncTestObject(objElement, false));
			}
		}
	}
	else if (element->GetName()->GetString() == "boolMember")
	{
		XTASSERT(element->GetElementType() == ElementType::BoolType);
		XTASSERT(m_boolElement == NULL);
		m_boolElement = BoolElement::Cast(element);
		m_boolMember = m_boolElement->GetValue();
	}
	else if (element->GetName()->GetString() == "floatMember")
	{
		XTASSERT(element->GetElementType() == ElementType::FloatType);
		XTASSERT(m_floatElement == NULL);
		m_floatElement = FloatElement::Cast(element);
		m_floatMember = m_floatElement->GetValue();
	}
	else if (element->GetName()->GetString() == "intMember")
	{
		XTASSERT(element->GetElementType() == ElementType::Int32Type);
		XTASSERT(m_intElement == NULL);
		m_intElement = IntElement::Cast(element);
		m_intMember = m_intElement->GetValue();
	}
	else if (element->GetName()->GetString() == "stringMember")
	{
		XTASSERT(element->GetElementType() == ElementType::StringType);
		XTASSERT(m_stringElement == NULL);
		m_stringElement = StringElement::Cast(element);
		m_stringMember = m_stringElement->GetValue()->GetString();
	}
	else if (element->GetName()->GetString() == "longMember")
	{
		XTASSERT(element->GetElementType() == ElementType::Int64Type);
		XTASSERT(m_longElement == NULL);
		m_longElement = LongElement::Cast(element);
		m_longMember = m_longElement->GetValue();
	}
	else if (element->GetName()->GetString() == "doubleMember")
	{
		XTASSERT(element->GetElementType() == ElementType::DoubleType);
		XTASSERT(m_doubleElement == NULL);
		m_doubleElement = DoubleElement::Cast(element);
		m_doubleMember = m_doubleElement->GetValue();
	}
	else if (element->GetName()->GetString() == "intArrayMember")
	{
		XTASSERT(element->GetElementType() == ElementType::Int32ArrayType);
		XTASSERT(m_intArrayElement == NULL);
		m_intArrayElement = IntArrayElement::Cast(element);
		m_intArrayElement->AddListener(this);
		m_intArrayMember.clear();
	}
	else if (element->GetName()->GetString() == "floatArrayMember")
	{
		XTASSERT(element->GetElementType() == ElementType::FloatArrayType);
		XTASSERT(m_floatArrayElement == NULL);
		m_floatArrayElement = FloatArrayElement::Cast(element);
		m_floatArrayElement->AddListener(this);
		m_floatArrayMember.clear();
	}
	else if (element->GetName()->GetString() == "stringArrayMember")
	{
		XTASSERT(element->GetElementType() == ElementType::StringArrayType);
		XTASSERT(m_stringArrayElement == NULL);
		m_stringArrayElement = StringArrayElement::Cast(element);
		m_stringArrayElement->AddListener(this);
		m_stringArrayMember.clear();
	}
}


void SyncTestObject::OnElementDeleted(const ElementPtr& element)
{
	XTASSERT(m_element->GetGUID() != element->GetGUID());

	++m_incomingRemoveCount;

	if (m_floatElement && m_floatElement->GetGUID() == element->GetGUID())
	{
		m_floatElement = NULL;
	}
	else if (m_intElement && m_intElement->GetGUID() == element->GetGUID())
	{
		m_intElement = NULL;
	}
	else if (m_stringElement && m_stringElement->GetGUID() == element->GetGUID())
	{
		m_stringElement = NULL;
	}
	else if (m_longElement && m_longElement->GetGUID() == element->GetGUID())
	{
		m_longElement = NULL;
	}
	else if (m_doubleElement && m_doubleElement->GetGUID() == element->GetGUID())
	{
		m_doubleElement = NULL;
	}
	else
	{
		bool bFound = false;
		for (size_t i = 0; i < m_children.size(); ++i)
		{
			if (m_children[i]->m_element->GetGUID() == element->GetGUID())
			{
				bFound = true;
				m_children.erase(m_children.begin() + i);
				break;
			}
		}

		// NOTE: deleted elements won't have been added in OnElementAdded, but we'll
		// still get deleted notifications
	}
}


void SyncTestObject::OnValueChanged(int32 index, int32 newValue)
{
	m_intArrayMember[index] = newValue;
}


void SyncTestObject::OnValueInserted(int32 index, int32 value)
{
	m_intArrayMember.insert(m_intArrayMember.begin() + index, value);
}


void SyncTestObject::OnValueRemoved(int32 index, int32 )
{
	m_intArrayMember.erase(m_intArrayMember.begin() + index);
}



void SyncTestObject::OnValueChanged(int32 index, float newValue)
{
	m_floatArrayMember[index] = newValue;
}


void SyncTestObject::OnValueInserted(int32 index, float value)
{
	m_floatArrayMember.insert(m_floatArrayMember.begin() + index, value);
}


void SyncTestObject::OnValueRemoved(int32 index, float )
{
	m_floatArrayMember.erase(m_floatArrayMember.begin() + index);
}


void SyncTestObject::OnValueChanged(int32 index, const XStringPtr& newValue)
{
	m_stringArrayMember[index] = newValue;
}


void SyncTestObject::OnValueInserted(int32 index, const XStringPtr& value)
{
	m_stringArrayMember.insert(m_stringArrayMember.begin() + index, value);
}


void SyncTestObject::OnValueRemoved(int32 index, const XStringPtr& )
{
	m_stringArrayMember.erase(m_stringArrayMember.begin() + index);
}
