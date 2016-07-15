// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// SessionDescriptor.h
// This class wraps the SessionDescriptor message (JSON message at the network level).  
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
