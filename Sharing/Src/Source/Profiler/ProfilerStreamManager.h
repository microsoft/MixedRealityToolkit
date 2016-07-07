// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// ProfilerStreamManager.h
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

DECLARE_PTR_PRE(ProfilerStreamManager)

class ProfilerStreamManager : public AtomicRefCounted
{
public:
	static ref_ptr<ProfilerStreamManager> Create();

	virtual ProfilerStreamPtr CreateStream(std::string remoteSystemName, SystemRole role) = 0;

	virtual void CloseStream(const ProfilerStreamPtr& stream) = 0;

	virtual void Update() = 0;
};

DECLARE_PTR_POST(ProfilerStreamManager)

XTOOLS_NAMESPACE_END
