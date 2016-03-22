//////////////////////////////////////////////////////////////////////////
// FileLogWriter.h
//
// A logger for writing to a file and to write to stdout. 
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class FileLogWriter : public LogWriter
{
public:

    FileLogWriter();
    virtual ~FileLogWriter();
    virtual void WriteLogEntry(LogSeverity severity, const std::string& message) XTOVERRIDE;

    virtual void AddTargetFile(const std::string& filename);

private:

    std::vector<std::ofstream*>			m_outputStreams;
	std::string							m_infoPrefix;
	std::string							m_warningPrefix;
	std::string							m_errorPrefix;
};

XTOOLS_NAMESPACE_END