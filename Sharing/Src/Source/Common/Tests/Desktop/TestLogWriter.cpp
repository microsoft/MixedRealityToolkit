//////////////////////////////////////////////////////////////////////////
// TestLogWriter.cpp
//
// Implements the XTools::LogWriter interface and prints log info to the
// test output
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TestLogWriter.h"

TestLogWriter* TestLogWriter::m_sInstance = nullptr;

//static
void TestLogWriter::Init()
{
	if (m_sInstance == nullptr)
	{
		m_sInstance = new TestLogWriter();
	}
}

void TestLogWriter::Release()
{
	if (m_sInstance != nullptr)
	{
		delete m_sInstance;
		m_sInstance = nullptr;
	}
}

TestLogWriter::TestLogWriter()
	: m_logManager(new ::XTools::Logger())
{
	m_logManager->SetWriter(this);
}

void TestLogWriter::WriteLogEntry(XTools::LogSeverity, const std::string& message)
{
	Microsoft::VisualStudio::CppUnitTestFramework::Logger::WriteMessage(message.c_str());
}
