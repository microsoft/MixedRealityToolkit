// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// ProfilerStreamManagerImpl.h
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class ProfilerStreamManagerImpl : public ProfilerStreamManager
{
public:
	ProfilerStreamManagerImpl();

	virtual ProfilerStreamPtr CreateStream(std::string remoteSystemName, SystemRole role) XTOVERRIDE;

	virtual void CloseStream(const ProfilerStreamPtr& stream) XTOVERRIDE;

	virtual void Update() XTOVERRIDE;

private:
	XSocketManagerPtr				m_socketMgr;
	std::vector<ProfilerStreamPtr>	m_streams;
};

XTOOLS_NAMESPACE_END
