//////////////////////////////////////////////////////////////////////////
// ProfileEntry.h
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class ProfileEntry
{
public:
	ProfileEntry();

	ProfileEntry(const std::string& name, int32 parent);

	void Start();
	void End();

	const std::string& GetName() const;

	int32 GetParent() const;

	uint64 GetStartTime() const;
	uint64 GetDuration() const;	// in nanoseconds

private:
	
	
#if defined(XTOOLS_PLATFORM_WINDOWS_ANY)
	typedef long long										rep;
	typedef std::nano										period;
	typedef std::chrono::duration<rep, period>				duration;
	typedef std::chrono::time_point<ProfileEntry>			time_point;

	static const uint64 m_sFrequency;
	
#else
    typedef std::chrono::high_resolution_clock::duration	duration;
	typedef std::chrono::high_resolution_clock::time_point	time_point;

#endif

    time_point GetTimeNow() const;
    
	std::string		m_name;
	time_point		m_startTime;
	time_point		m_endTime;
	int32			m_parent;
};

XTOOLS_NAMESPACE_END
