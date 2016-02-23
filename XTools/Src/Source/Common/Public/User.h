//////////////////////////////////////////////////////////////////////////
// User.h
//
// Contains information about a session user
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

typedef ::XTools::int32 UserID;

class User : public AtomicRefCounted
{
public:
	static const UserID kInvalidUserID = -1;
	
	// Returns the name of the user
	virtual const XStringPtr& GetName() const = 0;

	// Returns the unique ID of the user
	virtual UserID GetID() const = 0;

	// Returns false if the user's ID is kInvalidUserID or the length of the name is zero
	virtual bool IsValid() const = 0;

	virtual bool GetMuteState() const = 0;

#if !defined(SWIG)
	virtual void SetID(UserID id) = 0;

	virtual void SetName(const XStringPtr& name) = 0;

	virtual void SetMuteState(bool muteState) = 0;

#endif
};

DECLARE_PTR(User)

XTOOLS_NAMESPACE_END
