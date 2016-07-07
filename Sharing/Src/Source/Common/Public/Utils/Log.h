// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// Log.h
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
