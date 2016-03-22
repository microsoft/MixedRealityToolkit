//////////////////////////////////////////////////////////////////////////
// ProfileEntry.cpp
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ProfileEntry.h"

XTOOLS_NAMESPACE_BEGIN

#if defined(XTOOLS_PLATFORM_WINDOWS_ANY)
const uint64 ProfileEntry::m_sFrequency = []() -> uint64
{
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);
	return frequency.QuadPart;
}();

#endif

ProfileEntry::ProfileEntry()
: m_parent(-1)
{

}


ProfileEntry::ProfileEntry(const std::string& name, int32 parent)
	: m_name(name)
	, m_parent(parent)
{

}


void ProfileEntry::Start()
{
	m_startTime = GetTimeNow();
}


void ProfileEntry::End()
{
	m_endTime = GetTimeNow();
}


const std::string& ProfileEntry::GetName() const
{
	return m_name;
}


int32 ProfileEntry::GetParent() const
{
	return m_parent;
}


uint64 ProfileEntry::GetStartTime() const
{
    duration dtn = m_startTime.time_since_epoch();
	return dtn.count();
}


uint64 ProfileEntry::GetDuration() const
{
	return std::chrono::duration_cast<std::chrono::nanoseconds>(m_endTime - m_startTime).count();
}


ProfileEntry::time_point ProfileEntry::GetTimeNow() const
{
#if defined(XTOOLS_PLATFORM_WINDOWS_ANY)
	LARGE_INTEGER count;
	QueryPerformanceCounter(&count);
	return time_point(duration(count.QuadPart * static_cast<rep>(period::den) / m_sFrequency));
#else
	return std::chrono::high_resolution_clock::now();
#endif
}


XTOOLS_NAMESPACE_END
