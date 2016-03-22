//////////////////////////////////////////////////////////////////////////
// LogMessageImpl.cpp
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
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
