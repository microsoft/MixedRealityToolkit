// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// LogMessageImpl.h
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class LogMessageImpl : public LogMessage
{
public:
	LogMessageImpl(LogSeverity severity, const std::string& message);

	virtual LogSeverity GetSeverity() const XTOVERRIDE;
	virtual std::string GetLogMessage() const XTOVERRIDE;

private:
	LogSeverity m_severity;
	std::string m_message;
};

XTOOLS_NAMESPACE_END
