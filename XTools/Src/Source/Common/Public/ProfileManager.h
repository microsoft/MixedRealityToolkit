//////////////////////////////////////////////////////////////////////////
// ProfileManager.h
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class ProfileManager : public AtomicRefCounted
{
	XTOOLS_REFLECTION_DECLARE(ProfileManager)

public:
	virtual ~ProfileManager() {}

	virtual void BeginRange(const std::string& name) = 0;
	virtual void EndRange() = 0;

	virtual void Log(LogSeverity severity, const std::string& message) = 0;
};

DECLARE_PTR(ProfileManager)

#if !defined(SWIG) && !defined(XTOOLS_SIDECAR)
ProfileManagerPtr GetProfileManager();
#endif

XTOOLS_NAMESPACE_END
