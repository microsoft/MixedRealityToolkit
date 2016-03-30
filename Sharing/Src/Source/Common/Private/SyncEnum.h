//////////////////////////////////////////////////////////////////////////
// SyncEnum.h
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "Syncable.h"

XTOOLS_NAMESPACE_BEGIN

template<typename EnumType>
class SyncEnum : public Syncable
{
public:
	SyncEnum(EnumType initialValue) : m_value(initialValue) {}

	XGuid GetGUID() const XTOVERRIDE
	{
		if (m_element) { return m_element->GetGUID(); }
		else { return kInvalidXGuid; }
	}

	ElementType GetType() const XTOVERRIDE { return ElementType::Int32Type; }

	const IntElementPtr& GetElement() { return m_element; }


	EnumType Get() const { return m_value; }

	void Set(EnumType f)
	{
		if (f != m_value)
		{
			m_value = f;
			if (m_element) m_element->SetValue((int)m_value);
		}
	}

	SyncEnum<EnumType>& operator=(EnumType rhs)
	{
		Set(rhs);
		return *this;
	}

	operator const EnumType() const { return Get(); }


protected:
	virtual bool BindLocal(const ObjectElementPtr& parent, const std::string& name, const UserPtr&) XTOVERRIDE
	{
		m_element = parent->CreateIntElement(new XString(name), m_value);
		if (!XTVERIFY(m_element != nullptr))
		{
			LogError("Failed to create sync element for object %s", name.c_str());
            return false;
		}
        return true;
	}


	virtual void BindRemote(const ElementPtr& element) XTOVERRIDE
	{
		m_element = IntElement::Cast(element);
		if (XTVERIFY(m_element != nullptr))
		{
			m_value = (EnumType)m_element->GetValue();
		}
		else
		{
			LogError("Failed to bind sync float element");
		}
	}


	virtual void SetValue(int newValue) XTOVERRIDE
	{
		m_value = (EnumType)newValue;
	}

private:
	IntElementPtr m_element;
	EnumType m_value;
};

XTOOLS_NAMESPACE_END