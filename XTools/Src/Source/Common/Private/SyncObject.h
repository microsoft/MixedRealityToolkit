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
	// Syncable Functions
	virtual bool BindLocal(const ObjectElementPtr& parent, const std::string& name, const UserPtr& owner) XTOVERRIDE;
	virtual void BindRemote(const ElementPtr& element) XTOVERRIDE;

protected:
	typedef Callback<XGuid> MemberCallback;

	SyncObject();

	const ObjectElementPtr& GetElement();

	void AddMember(Syncable* memberSyncable, const std::string& name);
	void AddMember(Syncable* memberSyncable, const std::string& name, MemberCallback onChangeCallback);


	// ObjectElementListener Functions:
	virtual void OnIntElementChanged(XGuid elementID, int32 newValue) XTOVERRIDE;
	virtual void OnFloatElementChanged(XGuid elementID, float newValue) XTOVERRIDE;
	virtual void OnStringElementChanged(XGuid elementID, const XStringPtr& newValue) XTOVERRIDE;
	virtual void OnElementAdded(const ElementPtr& element) XTOVERRIDE;
	virtual void OnElementDeleted(const ElementPtr& element) XTOVERRIDE;

private:
	template<typename T>
	void OnElementValueChanged(XGuid elementID, T newValue);

	std::map<std::string, Syncable*>		m_members;
	std::map<XGuid, Syncable*>		m_memberMap;
	std::map<Syncable*, MemberCallback>		m_callbackMap;

	ObjectElementPtr				m_element;

	friend class SyncRoot;
};

XTOOLS_NAMESPACE_END