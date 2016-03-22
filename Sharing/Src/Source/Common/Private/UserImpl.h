//////////////////////////////////////////////////////////////////////////
// UserImpl.h
//
// This class wraps the UserImpl message (JSON message at the network level).  
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class UserImpl : public User
{
public:
	UserImpl();
	UserImpl(const std::string& name, UserID id, bool muteState);

	// Returns the name of the user
	virtual const XStringPtr& GetName() const XTOVERRIDE;

	// Returns the unique ID of the user
	virtual UserID GetID() const XTOVERRIDE;

	// Returns false if the user's ID is kInvalidUserID or the length of the name is zero
	virtual bool IsValid() const XTOVERRIDE;

	virtual bool GetMuteState() const XTOVERRIDE;

	virtual void SetID(UserID id) XTOVERRIDE;

	virtual void SetName(const XStringPtr& name) XTOVERRIDE;

	virtual void SetMuteState(bool muteState) XTOVERRIDE;

	// JSON guts to parse or create the data structure tree.
	web::json::value AsJSON() const;
	static UserImpl* FromJSON(const web::json::object& object);
private:
	XStringPtr	m_name;
	UserID		m_id;
	bool        m_muteState;
};

DECLARE_PTR(UserImpl)

XTOOLS_NAMESPACE_END