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

class SyncTestObject : 
	public RefCounted, 
	public ObjectElementListener, 
	public IntArrayListener,
	public FloatArrayListener,
	public StringArrayListener
{
public:
	SyncTestObject(const ObjectElementPtr& element, bool bCreatedLocally = true);

	ref_ptr<SyncTestObject> AddChild(const std::string& name);
	void RemoveChild(const std::string& name);

	const ObjectElementPtr& GetElement() const;

	ref_ptr<SyncTestObject> GetChild(int index);
	ref_ptr<SyncTestObject> GetChild(const std::string& name);

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

	int64 GetLongValue() const;
	void SetLongValue(int64 value);
	void RemoveLongValue();

	double GetDoubleValue() const;
	void SetDoubleValue(double value);
	void RemoveDoubleValue();

	int32 GetIntArrayLength() const;
	int32 GetIntArrayValue(int32 index) const;
	void SetIntArrayValue(int32 index, int32 value);
	void InsertIntArrayValue(int32 index, int32 value);
	void RemoveIntArrayValue(int32 index);

	void SetFloatArrayValue(int32 index, float value);
	void InsertFloatArrayValue(int32 index, float value);
	void RemoveFloatArrayValue(int32 index);

	void SetStringArrayValue(int32 index, const XStringPtr& value);
	void InsertStringArrayValue(int32 index, const XStringPtr& value);
	void RemoveStringArrayValue(int32 index);

	bool Equals(const ref_ptr<const SyncTestObject>& otherObj) const;

	uint32 GetIncomingIntChanges() const { return m_incomingIntChangeCount; }
	uint32 GetIncomingFloatChanges() const { return m_incomingFloatChangeCount; }
	uint32 GetIncomingStringChanges() const { return m_incomingStringChangeCount; }
	uint32 GetIncomingAdds() const { return m_incomingAddCount; }
	uint32 GetIncomingRemoves() const { return m_incomingRemoveCount; }

	virtual void OnBoolElementChanged(XGuid elementID, bool newValue) XTOVERRIDE;
	virtual void OnIntElementChanged(XGuid elementID, int32 newValue) XTOVERRIDE;
	virtual void OnFloatElementChanged(XGuid elementID, float newValue) XTOVERRIDE;
	virtual void OnStringElementChanged(XGuid elementID, const XStringPtr& newValue) XTOVERRIDE;
	virtual void OnLongElementChanged(XGuid elementID, int64 newValue) XTOVERRIDE;
	virtual void OnDoubleElementChanged(XGuid elementID, double newValue) XTOVERRIDE;

	virtual void OnElementAdded(const ElementPtr& element) XTOVERRIDE;
	virtual void OnElementDeleted(const ElementPtr& element) XTOVERRIDE;

	// IntArrayListener Functions:
	virtual void OnValueChanged(int32 index, int32 newValue) XTOVERRIDE;
	virtual void OnValueInserted(int32 index, int32 value) XTOVERRIDE;
	virtual void OnValueRemoved(int32 index, int32 value) XTOVERRIDE;

	// FloatArrayListener Functions:
	virtual void OnValueChanged(int32 index, float newValue) XTOVERRIDE;
	virtual void OnValueInserted(int32 index, float value) XTOVERRIDE;
	virtual void OnValueRemoved(int32 index, float value) XTOVERRIDE;

	// StringArrayListener Functions:
	virtual void OnValueChanged(int32 index, const XStringPtr& newValue) XTOVERRIDE;
	virtual void OnValueInserted(int32 index, const XStringPtr& value) XTOVERRIDE;
	virtual void OnValueRemoved(int32 index, const XStringPtr& value) XTOVERRIDE;

private:
	std::vector<ref_ptr<SyncTestObject> >	m_children;

	std::string							m_name;
	ObjectElementPtr					m_element;

	BoolElementPtr		m_boolElement;
	bool				m_boolMember;

	FloatElementPtr		m_floatElement;
	float				m_floatMember;

	IntElementPtr		m_intElement;
	int32				m_intMember;

	StringElementPtr	m_stringElement;
	std::string			m_stringMember;

	LongElementPtr		m_longElement;
	int64				m_longMember;

	DoubleElementPtr	m_doubleElement;
	double				m_doubleMember;

	IntArrayElementPtr	m_intArrayElement;
	std::vector<int32>	m_intArrayMember;

	FloatArrayElementPtr	m_floatArrayElement;
	std::vector<float>		m_floatArrayMember;

	StringArrayElementPtr	m_stringArrayElement;
	std::vector<XStringPtr>	m_stringArrayMember;

	uint32				m_incomingIntChangeCount;
	uint32				m_incomingFloatChangeCount;
	uint32				m_incomingStringChangeCount;
	uint32				m_incomingAddCount;
	uint32				m_incomingRemoveCount;
};

DECLARE_PTR(SyncTestObject)
