//////////////////////////////////////////////////////////////////////////
// Logger.cpp
//
// Simple logging class to capture info, warnings, and errors to help with 
// debugging and telemetry
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Public/Utils/Logger.h>
#include <iostream>
#include "../ProfileManagerImpl.h"

XTOOLS_NAMESPACE_BEGIN

void LogNative(::XTools::LogSeverity severity, const char* file, int line, const char* message)
{
	char logEntryBuffer[1024];

	switch (severity)
	{
	case Warning:
	case Error:
		sprintf_s(logEntryBuffer, sizeof(logEntryBuffer), "SharingService [%s (%i)]: %s", file, line, message);
		break;

	case Info:
	default:
		sprintf_s(logEntryBuffer, sizeof(logEntryBuffer), "SharingService: %s", message);
		break;
	}

	LoggerPtr mgr = Logger::GetInstance();
	if (mgr)
	{
		mgr->Log(severity, logEntryBuffer);
	}
}


void LogFormat(::XTools::LogSeverity severity, const char* file, int line, const char* message, ...)
{
	char buffer[512];

    {
        va_list ap;
        va_start(ap, message);
        vsprintf_s(buffer, sizeof(buffer), message, ap);
        va_end(ap);
    }
	
	LogNative(severity, file, line, buffer);
}


Logger* Logger::m_sInstance = nullptr;
Mutex Logger::m_sInstanceMutex;

//static 
LoggerPtr Logger::GetInstance()
{
	return m_sInstance;
}


Logger::Logger()
	: m_writer(nullptr)
{
	ScopedLock lock(m_sInstanceMutex);
	if (m_sInstance == nullptr)
	{
		m_sInstance = this;
	}
}


Logger::~Logger()
{
	ScopedLock lock(m_sInstanceMutex);
	if (m_sInstance == this)
	{
		m_sInstance = nullptr;
	}
}

void Logger::Log(LogSeverity severity, const char* message)
{
	ProfileManagerImplPtr profMgr = ProfileManagerImpl::GetInstance();
	if (profMgr)
	{
		profMgr->Log(severity, message);
	}

	ScopedLock lock(m_mutex);
	if (m_writer)
	{
		m_writer->WriteLogEntry(severity, message);
	}
	else
	{
		std::cout << message << std::endl << std::flush;
	}
}


bool Logger::IsWriterSet()
{
	ScopedLock lock(m_mutex);
	return (m_writer != nullptr);
}


void Logger::SetWriter(LogWriter* writer)
{
	ScopedLock lock(m_mutex);
	m_writer = writer;
}


void Logger::ClearWriter()
{
	ScopedLock lock(m_mutex);
	m_writer = nullptr;
}

XTOOLS_NAMESPACE_END
