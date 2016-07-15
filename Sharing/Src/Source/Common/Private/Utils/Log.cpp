// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// Log.cpp
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Public/Utils/Log.h>
#include <Public/Utils/Logger.h>

XTOOLS_NAMESPACE_BEGIN

// static 
void Log::Info(const char* message)
{
	LoggerPtr mgr = Logger::GetInstance();
	if (mgr != nullptr)
	{
		mgr->Log(LogSeverity::Info, message);
	}
}


// static 
void Log::Warning(const char* message)
{
	LoggerPtr mgr = Logger::GetInstance();
	if (mgr != nullptr)
	{
		mgr->Log(LogSeverity::Warning, message);
	}
}


// static 
void Log::Error(const char* message)
{
	LoggerPtr mgr = Logger::GetInstance();
	if (mgr != nullptr)
	{
		mgr->Log(LogSeverity::Error, message);
	}
}

XTOOLS_NAMESPACE_END
