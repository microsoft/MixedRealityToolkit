//////////////////////////////////////////////////////////////////////////
// MemberFuncThread.cpp
//
// 
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Public/Utils/MemberFuncThread.h>

XTOOLS_NAMESPACE_BEGIN

MemberFuncThread::~MemberFuncThread()
{
	WaitForThreadExit();
}

void MemberFuncThread::WaitForThreadExit()
{
	m_thread.Join();
}


XTOOLS_NAMESPACE_END
