//////////////////////////////////////////////////////////////////////////
// ScopedLock.cpp
//
// 
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
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
