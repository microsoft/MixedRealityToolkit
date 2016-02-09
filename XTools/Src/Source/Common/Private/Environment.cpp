//////////////////////////////////////////////////////////////////////////
// Environment.cpp
//
// 
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Public/Environment.h>
#include <iostream>

XTOOLS_NAMESPACE_BEGIN

void DoAssert(const char* message, const char* file, unsigned line)
{
	try
	{
		LogError("Assert (%s:%i) %s", file, line, message);
	}
	catch (...)
	{
		std::cerr << "Assert (" << file << ":" << line << ") " << message;
	}

#if defined(XTOOLS_PLATFORM_WINDOWS_DESKTOP)
	if (IsDebuggerPresent())
	{
		DebugBreak();
	}
	else
	{
		throw "Assert Failed";
	}
#else
	assert(false && message && file && line);
#endif
}

XTOOLS_NAMESPACE_END