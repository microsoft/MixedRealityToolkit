// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// MemberFuncThread.cpp
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
