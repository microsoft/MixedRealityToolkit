//////////////////////////////////////////////////////////////////////////
// TestLogWriter.h
//
// Implements the XTools::LogWriter interface and prints log info to the
// test output
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

class TestLogWriter : public XTools::LogWriter
{
public:
	static void Init();
	static void Release();
	virtual void WriteLogEntry(XTools::LogSeverity, const std::string& message) XTOVERRIDE;

private:
	TestLogWriter();
	XTools::LoggerPtr	m_logManager;
	static TestLogWriter* m_sInstance;
};
