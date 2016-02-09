//////////////////////////////////////////////////////////////////////////
// Logger.h
//
// Simple logging class to capture info, warnings, and errors to help with 
// debugging and telemetry
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <Public/Utils/Mutex.h>

XTOOLS_NAMESPACE_BEGIN

class Logger : public LogManager
{
public:
	static ref_ptr<Logger> GetInstance();

	Logger();
	virtual ~Logger();

	// All logging goes through this function
	virtual void Log(LogSeverity severity, const char* message) XTOVERRIDE;

	bool IsWriterSet();
	void SetWriter(LogWriter* writer);
	void ClearWriter();

private:
	static Logger*	m_sInstance;
	static Mutex	m_sInstanceMutex;
	LogWriter*		m_writer;
	Mutex			m_mutex;
};

DECLARE_PTR(Logger)

XTOOLS_NAMESPACE_END


