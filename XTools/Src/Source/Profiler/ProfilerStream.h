//////////////////////////////////////////////////////////////////////////
// ProfilerStream.h
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
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
