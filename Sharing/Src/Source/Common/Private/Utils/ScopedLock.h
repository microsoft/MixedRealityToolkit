// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// ScopedLock.h
// Utility class to lock a mutex for the current scope
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
