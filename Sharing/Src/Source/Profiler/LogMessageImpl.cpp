// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// LogMessageImpl.cpp
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "LogMessageImpl.h"

XTOOLS_NAMESPACE_BEGIN

LogMessageImpl::LogMessageImpl(LogSeverity severity, const std::string& message)
: m_severity(severity)
, m_message(message)
{}


LogSeverity LogMessageImpl::GetSeverity() const
{
	return m_severity;
}


std::string LogMessageImpl::GetLogMessage() const
{
	return m_message;
}

XTOOLS_NAMESPACE_END
