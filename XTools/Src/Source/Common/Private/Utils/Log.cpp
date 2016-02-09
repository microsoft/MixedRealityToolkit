//////////////////////////////////////////////////////////////////////////
// Log.cpp
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
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
