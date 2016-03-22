//////////////////////////////////////////////////////////////////////////
// SessionDescriptorImpl.h
//
// This class wraps the SessionDescriptorImpl message (JSON message at the network level).  
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <Public/SessionDescriptor.h>
#include <Private/UserImpl.h>
#include <Private/json/cpprest/json.h>

XTOOLS_NAMESPACE_BEGIN

class SessionDescriptorImpl;
DECLARE_PTR(SessionDescriptorImpl)

class SessionDescriptorImpl : public SessionDescriptor
{
public:
	/////////////////////////////////////////////////////////////////////////
	// SessionDescriptor methods

	virtual std::string GetName() const XTOVERRIDE;
	virtual uint32 GetID() const XTOVERRIDE;
	virtual SessionType GetSessionType() const XTOVERRIDE;
	virtual int32 GetUserCount() const XTOVERRIDE;
	virtual User* GetUser(const int32 i) const XTOVERRIDE;
	virtual std::string GetAddress() const XTOVERRIDE;
	virtual uint16 GetPortID() const XTOVERRIDE;

	/////////////////////////////////////////////////////////////////////////

	// property setters
	void SetName(const std::string& str);
	void SetID(uint32 sessionID);
	void SetSessionType(const SessionType sess);
	void SetUserCount(const int32 count);
	void SetUser(const int32 i, UserImplPtr puser);
	void SetAddress(const std::string& address);
	void SetPortID(uint16 port);

	// JSON guts to parse or create the data structure tree.
	web::json::value AsJSON() const;
	static SessionDescriptorImpl* FromJSON(const web::json::object& object);

private:
	// properties
	utility::string_t m_name;
	uint32 m_id;
	SessionType m_sessionType;
	int32 m_userCount;
	std::vector<UserImplPtr> m_users;
	std::string m_address;
	uint16 m_port;
};

XTOOLS_NAMESPACE_END