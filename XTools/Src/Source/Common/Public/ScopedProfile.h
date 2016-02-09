//////////////////////////////////////////////////////////////////////////
// ScopedProfile.h
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#if defined(XTOOLS_SIDECAR)
extern ProfilerFunc gProfilerFunction;
#endif

XTOOLS_NAMESPACE_BEGIN

class ScopedProfile
{
public:
	explicit ScopedProfile(const std::string& name)
	{
#if defined(XTOOLS_SIDECAR)
		ProfileManagerPtr mgr = GetProfileMgr();
		if (mgr)
		{
			mgr->BeginRange(name);
		}
#else
		Profile::BeginRange(name);
#endif
	}


	~ScopedProfile()
	{
#if defined(XTOOLS_SIDECAR)
		ProfileManagerPtr mgr = GetProfileMgr();
		if (mgr)
		{
			mgr->EndRange();
		}
#else
		Profile::EndRange();
#endif
	}

private:
#if defined(XTOOLS_SIDECAR)
	ProfileManagerPtr GetProfileMgr()
	{
		return ((gProfilerFunction) ? gProfilerFunction() : nullptr);
	}
#endif
};

XTOOLS_NAMESPACE_END

#if defined ( WIN32 )
#define __func__ __FUNCTION__
#endif

#define PROFILE_SCOPE(X) \
	::XTools::ScopedProfile profileScope(X)

#define PROFILE_FUNCTION() \
	::XTools::ScopedProfile profileFunc(__func__)
