// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// XSession.h
// The XTools server's representation of an active session
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class SessionChangeCallback;

class XSession : public RefCounted
{
public:

	// Register to receive callbacks when the session changes.  
	virtual ReceiptPtr RegisterCallback(SessionChangeCallback* cb) = 0;

	virtual uint32		GetId() const = 0;

	virtual std::string GetName() const = 0;

	virtual SessionType GetType() const = 0;

	virtual int32		GetUserCount() const = 0;
	virtual std::string GetSessionUserName(int32 i) const = 0;
	virtual uint32		GetSessionUserID(int32 i) const = 0;
    virtual bool        GetUserMuteState(int32 i) const = 0;
};

DECLARE_PTR(XSession)

XTOOLS_NAMESPACE_END
