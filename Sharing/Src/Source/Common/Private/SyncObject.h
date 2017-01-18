//////////////////////////////////////////////////////////////////////////
// SyncObject.h
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "Syncable.h"

XTOOLS_NAMESPACE_BEGIN

class SyncObject : public Syncable, public ObjectElementListener
{
	XTOOLS_REFLECTION_DECLARE(SyncObject)

public:
	virtual ~SyncObject();

	const ObjectElementPtr& GetObjectElement() const;
	const XStringPtr& GetObjectType() const;

	// Syncable Functions
	virtual XGuid GetGUID() const XTOVERRIDE;
	virtual ElementType GetType() const XTOVERRIDE;
	virtual ElementPtr GetElement() const XTOVERRIDE;
	virtual bool BindLocal(const ObjectElementPtr& parent, const std::string& name, const UserPtr& owner) XTOVERRIDE;
	virtual void BindRemote(const ElementPtr& element) XTOVERRIDE;
	virtual void Unbind() XTOVERRIDE;

protected:
	typedef Callback<XGuid> MemberCallback;

	explicit SyncObject(const std::string& objectType);

	void AddMember(Syncable* memberSyncable, const std::string& name);
	void AddMember(Syncable* memberSyncable, const std::string& name, MemberCallback onChangeCallback);

	// ObjectElementListener Functions:
	virtual void OnBoolElementChanged(XGuid elementID, bool newValue) XTOVERRIDE;
	virtual void OnIntElementChanged(XGuid elementID, int32 newValue) XTOVERRIDE;
	virtual void OnLongElementChanged(XGuid elementID, int64 newValue) XTOVERRIDE;
	virtual void OnFloatElementChanged(XGuid elementID, float newValue) XTOVERRIDE;
	virtual void OnDoubleElementChanged(XGuid elementID, double newValue) XTOVERRIDE;
	virtual void OnStringElementChanged(XGuid elementID, const XStringPtr& newValue) XTOVERRIDE;
	virtual void OnElementAdded(const ElementPtr& element) XTOVERRIDE;
	virtual void OnElementDeleted(const ElementPtr& element) XTOVERRIDE;

private:
	virtual void SetValue(const XValue& newValue) XTOVERRIDE;

	template<typename T>
	void OnElementValueChanged(XGuid elementID, T newValue);

	std::map<std::string, Syncable*>		m_members;
	std::map<XGuid, Syncable*>				m_memberMap;
	std::map<Syncable*, MemberCallback>		m_callbackMap;

	ObjectElementPtr						m_element;
	XStringPtr								m_objectType;
};

XTOOLS_NAMESPACE_END