//////////////////////////////////////////////////////////////////////////
// SyncArray.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "Syncable.h"

XTOOLS_NAMESPACE_BEGIN

class SyncArray : public Syncable, public IntArrayListener
{
public:
	SyncArray() : m_listeners(ListenerList<IntArrayListener>::Create()) {}
	virtual ~SyncArray()
	{
		if (m_element && m_element->IsValid())
		{
			ObjectElementPtr parent = ObjectElement::Cast(m_element->GetParent());
			if (parent)
			{
				parent->RemoveElement(m_element);
			}
		}
	}

	virtual XGuid GetGUID() const XTOVERRIDE
	{
		if (m_element) { return m_element->GetGUID(); }
		else { return kInvalidXGuid; }
	}

	virtual ElementType GetType() const XTOVERRIDE
	{
		return ElementType::Int32ArrayType;
	}

	ElementPtr GetElement() const XTOVERRIDE { return m_element; }

	void AddListener(IntArrayListener* listener) { m_listeners->AddListener(listener); }
	void RemoveListener(IntArrayListener* listener) { m_listeners->RemoveListener(listener); }

	const IntArrayElementPtr& GetIntArrayElement() const { return m_element; }

	int32 GetCount() const { return (int32)m_array.size(); }

	int32 Get(int32 index) const 
	{
		XTASSERT(index >= 0);
		XTASSERT(index < GetCount());
		return m_array[index];
	}

	void Set(int32 index, int32 value)
	{
		XTASSERT(index >= 0);
		XTASSERT(index < GetCount());
		m_array[index] = value;
		if (m_element) m_element->SetValue(index, value);
	}

	void Insert(int32 index, int32 value)
	{
		XTASSERT(index >= 0);
		XTASSERT(index <= GetCount());
		m_array.insert(m_array.begin() + index, value);
		if (m_element) m_element->InsertValue(index, value);
	}

	void Remove(int32 index)
	{
		XTASSERT(index >= 0);
		XTASSERT(index < GetCount());
		m_array.erase(m_array.begin() + index);
		if (m_element) m_element->RemoveValue(index);
	}

	struct EntryProxy
	{
		EntryProxy(SyncArray& arrayRef, int32 index) : m_arrayRef(arrayRef) , m_index(index) {}
		EntryProxy& operator=(int32 rhs) { m_arrayRef.Set(m_index, rhs); return *this; }
		operator const int32() const { return m_arrayRef.Get(m_index); }

	private:
		SyncArray& m_arrayRef;
		int32 m_index;
	};

	EntryProxy operator[](int32 index) { return EntryProxy(*this, index); }


	// Create a new element for this instance in the sync system
	virtual bool BindLocal(const ObjectElementPtr& parent, const std::string& name, const UserPtr& ) XTOVERRIDE
	{
		m_element = parent->CreateIntArrayElement(new XString(name));
		if (XTVERIFY(m_element != nullptr))
		{
			// Register for change events
			m_element->AddListener(this);

			// Add all the existing values
			const int32 numEntries = (int32)m_array.size();
			for (int32 i = 0; i < numEntries; ++i)
			{
				m_element->InsertValue(i, m_array[i]);
			}

			return true;
		}
		else
		{
			LogError("Failed to create sync element for array %s", name.c_str());
			return false;
		}
	}

	// Bind this instance to an element that already exists in the sync system
	virtual void BindRemote(const ElementPtr& element) XTOVERRIDE
	{
		m_element = IntArrayElement::Cast(element);
		if (XTVERIFY(m_element != nullptr))
		{
			// Register this object as a listener for change events of its children
			m_element->AddListener(this);

			// Clear out any values that might be in the array already
			// The sync system will fire callbacks to fill out the array with new elements once
			// this function returns
			m_array.clear();
		}
		else
		{
			LogError("Failed to use remotely created sync element for array %s", element->GetName()->GetString().c_str());
		}
	}

	virtual void Unbind() XTOVERRIDE
	{
		if (m_element)
		{
			m_element->RemoveListener(this);
			m_element = nullptr;
		}
	}

private:
	virtual void SetValue(const XValue& ) XTOVERRIDE
	{
		// Should not get called for arrays
		XTASSERT(false);
	}

	virtual void OnValueChanged(int32 index, int32 newValue) XTOVERRIDE
	{
		m_array[index] = newValue;
		m_listeners->NotifyListeners(&IntArrayListener::OnValueChanged, index, newValue);
	}

	virtual void OnValueInserted(int32 index, int32 value) XTOVERRIDE
	{
		m_array.insert(m_array.begin() + index, value);
		m_listeners->NotifyListeners(&IntArrayListener::OnValueInserted, index, value);
	}

	virtual void OnValueRemoved(int32 index, int32 value) XTOVERRIDE
	{
		m_array.erase(m_array.begin() + index);
		m_listeners->NotifyListeners(&IntArrayListener::OnValueRemoved, index, value);
	}

	IntArrayElementPtr m_element;
	std::vector<int32> m_array;
	ref_ptr<ListenerList<IntArrayListener> > m_listeners;
};

XTOOLS_NAMESPACE_END
