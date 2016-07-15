// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// FileLogWriter.h
// A logger for writing to a file and to write to stdout. 
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
