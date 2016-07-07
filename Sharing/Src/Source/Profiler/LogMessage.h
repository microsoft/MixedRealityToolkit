// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// LogMessage.h
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class LogMessage : public AtomicRefCounted
{
public:
	virtual ~LogMessage() {}

	virtual LogSeverity GetSeverity() const = 0;
	virtual std::string GetLogMessage() const = 0;
};

DECLARE_PTR(LogMessage)

XTOOLS_NAMESPACE_END
