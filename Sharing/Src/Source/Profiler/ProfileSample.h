// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// ProfileSample.h
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class ProfileSample : public AtomicRefCounted
{
public:
	ProfileSample(const std::string& name, uint64 startTime, uint64 duration, int32 parentIndex);

	std::string GetName() const;
	uint64		GetStartTime() const;
	uint64		GetDuration() const;
	int32		GetParentIndex() const;

private:
	std::string m_name;
	uint64		m_startTime;
	uint64		m_duration;
	int32		m_parentIndex;
};

DECLARE_PTR(ProfileSample)

XTOOLS_NAMESPACE_END
