//////////////////////////////////////////////////////////////////////////
// SyncTestObject.h
//
// Object used to test the sync system
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

using namespace XTools;
using namespace XTools::Sync;

class SyncObject : public RefCounted, public ObjectElementListener
{
public:
	SyncObject(const ObjectElementPtr& element, bool bCreatedLocally = true);

	ref_ptr<SyncObject> AddChild(const std::string& name);
	void RemoveChild(const std::string& name);

	ref_ptr<SyncObject> GetChild(int index);
	ref_ptr<SyncObject> GetChild(const std::string& name);

	std::string GetName() const;

	float GetFloatValue() const;
	void SetFloatValue(float value);
	void RemoveFloatValue();
	bool IsFloatValid() const;

	int32 GetIntValue() const;
	void SetIntValue(int32 value);
	void RemoveIntValue();

	std::string GetStringValue() const;
	void SetStringValue(const std::string& value);
	void RemoveStringValue();

	bool Equals(const ref_ptr<const SyncObject>& otherObj) const;

	uint32 GetIncomingIntChanges() const { return m_incomingIntChangeCount; }
	uint32 GetIncomingFloatChanges() const { return m_incomingFloatChangeCount; }
	uint32 GetIncomingStringChanges() const { return m_incomingStringChangeCount; }
	uint32 GetIncomingAdds() const { return m_incomingAddCount; }
	uint32 GetIncomingRemoves() const { return m_incomingRemoveCount; }

	virtual void OnIntElementChanged(XGuid elementID, int32 newValue) XTOVERRIDE;
	virtual void OnFloatElementChanged(XGuid elementID, float newValue) XTOVERRIDE;
	virtual void OnStringElementChanged(XGuid elementID, const XStringPtr& newValue) XTOVERRIDE;

	virtual void OnElementAdded(const ElementPtr& element) XTOVERRIDE;
	virtual void OnElementDeleted(const ElementPtr& element) XTOVERRIDE;

private:
	std::vector<ref_ptr<SyncObject> >	m_children;

	std::string							m_name;
	ObjectElementPtr					m_element;

	FloatElementPtr m_floatElement;
	float			m_floatMember;

	IntElementPtr	m_intElement;
	int32			m_intMember;

	StringElementPtr	m_stringElement;
	std::string			m_stringMember;

	uint32				m_incomingIntChangeCount;
	uint32				m_incomingFloatChangeCount;
	uint32				m_incomingStringChangeCount;
	uint32				m_incomingAddCount;
	uint32				m_incomingRemoveCount;
};

DECLARE_PTR(SyncObject)
