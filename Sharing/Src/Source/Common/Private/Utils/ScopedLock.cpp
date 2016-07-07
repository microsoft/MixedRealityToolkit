// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// ScopedLock.cpp
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ScopedLock.h"

XTOOLS_NAMESPACE_BEGIN

ScopedLock::ScopedLock(Mutex& mutex)
: m_mutex(mutex)
{
	m_mutex.Lock();
}


ScopedLock::~ScopedLock()
{
	m_mutex.Unlock();
}

XTOOLS_NAMESPACE_END
