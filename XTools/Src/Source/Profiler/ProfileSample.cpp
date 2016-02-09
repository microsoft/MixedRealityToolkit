//////////////////////////////////////////////////////////////////////////
// ProfileSample.cpp
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ProfileSample.h"

XTOOLS_NAMESPACE_BEGIN

ProfileSample::ProfileSample(const std::string& name, uint64 startTime, uint64 duration, int32 parentIndex)
: m_name(name)
, m_startTime(startTime)
, m_duration(duration)
, m_parentIndex(parentIndex)
{

}


std::string ProfileSample::GetName() const
{
	return m_name;
}


uint64 ProfileSample::GetStartTime() const
{
	return m_startTime;
}


uint64 ProfileSample::GetDuration() const
{
	return m_duration;
}


int32 ProfileSample::GetParentIndex() const
{
	return m_parentIndex;
}

XTOOLS_NAMESPACE_END
