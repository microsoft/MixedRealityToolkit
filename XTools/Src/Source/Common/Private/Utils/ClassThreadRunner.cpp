//////////////////////////////////////////////////////////////////////////
// ClassThreadRunner.cpp
//
// Creates a class on its own thread and updates it until the thread exits
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Public/Utils/ClassThreadRunner.h>

XTOOLS_NAMESPACE_BEGIN

ClassThreadRunner::~ClassThreadRunner()
{
	WaitForThreadExit();
}


void ClassThreadRunner::WaitForThreadExit()
{
	m_thread.Join();
}


XTOOLS_NAMESPACE_END
