// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// ProfilerStream.h
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class ProfilerStream : public AtomicRefCounted
{
public:
	virtual void AddListener(ProfilerStreamListener* newListener) = 0;

	virtual void RemoveListener(ProfilerStreamListener* oldListener) = 0;

	virtual bool IsConnected() const = 0;

	virtual void Connect() = 0;

	virtual void Disconnect() = 0;

	virtual std::string GetRemoteSystemName() const = 0;
};

DECLARE_PTR(ProfilerStream)

XTOOLS_NAMESPACE_END
