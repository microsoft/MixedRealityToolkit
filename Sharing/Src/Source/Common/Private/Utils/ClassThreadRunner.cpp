// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// ClassThreadRunner.cpp
// Creates a class on its own thread and updates it until the thread exits
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
