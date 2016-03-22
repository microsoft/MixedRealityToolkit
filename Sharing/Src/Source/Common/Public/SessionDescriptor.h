//////////////////////////////////////////////////////////////////////////
// SessionDescriptor.h
//
// This class wraps the SessionDescriptor message (JSON message at the network level).  
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class SessionDescriptor : public RefCounted
{
public:
	virtual std::string GetName() const = 0;

	virtual uint32 GetID() const = 0;

	virtual SessionType GetSessionType() const = 0;

	virtual int32 GetUserCount() const = 0;

	virtual User* GetUser(const int32 i) const = 0;

	virtual std::string GetAddress() const = 0;

	virtual uint16 GetPortID() const = 0;
};

DECLARE_PTR(SessionDescriptor)

XTOOLS_NAMESPACE_END