//////////////////////////////////////////////////////////////////////////
// LogMessage.h
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
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
