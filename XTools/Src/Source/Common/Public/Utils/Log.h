//////////////////////////////////////////////////////////////////////////
// Log.h
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class Log
{
public:
	static void Info(const char* message);
	static void Warning(const char* message);
	static void Error(const char* message);
};

XTOOLS_NAMESPACE_END
