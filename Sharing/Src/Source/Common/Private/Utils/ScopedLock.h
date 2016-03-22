//////////////////////////////////////////////////////////////////////////
// ScopedLock.h
//
// Utility class to lock a mutex for the current scope
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class ScopedLock
{
public:
	explicit ScopedLock(Mutex& mutex);
	~ScopedLock();

private:
	// Prevent automatic copy construction
	ScopedLock(const ScopedLock& rhs);
	ScopedLock& operator=(const ScopedLock& rhs);

	Mutex& m_mutex;
};

XTOOLS_NAMESPACE_END
