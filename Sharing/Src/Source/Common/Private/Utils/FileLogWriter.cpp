//////////////////////////////////////////////////////////////////////////
// FileLogWriter.cpp
//
// A logger for writing to a file and to write to stdout. 
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Public/Utils/Logger.h>
#include <Private/Utils/FileLogWriter.h>
#include <fstream>

XTOOLS_NAMESPACE_BEGIN

FileLogWriter::FileLogWriter()
: m_infoPrefix("")
, m_warningPrefix("Warning: ")
, m_errorPrefix("Error: ")
{
}

FileLogWriter::~FileLogWriter()
{
    size_t streamCount = m_outputStreams.size();
    for (size_t i = 0; i < streamCount; ++i)
    {
        std::ofstream* curStream = m_outputStreams[i];
        if (curStream)
        {
            delete curStream;
            curStream = NULL;
        }
    }

    m_outputStreams.clear();
}

void FileLogWriter::WriteLogEntry(LogSeverity severity, const std::string& message)
{
	std::string prefix;
	if (severity == LogSeverity::Info)
	{
		prefix = m_infoPrefix;
	}
	else if (severity == LogSeverity::Warning)
	{
		prefix = m_warningPrefix;
	}
	else
	{
		prefix = m_errorPrefix;
	}

    // Always log to any attached console.
	std::cout << prefix << message << std::endl << std::flush;

    // Log to all the files.
    size_t streamCount = m_outputStreams.size();
    for (size_t i = 0; i < streamCount; ++i)
    {
        std::ofstream* curStream = m_outputStreams[i];
        
        // If curStream is null, that's unexpected. ASSERT.
        if (XTVERIFY(curStream) && curStream->good())
        {
			*curStream << prefix << message << std::endl << std::flush;
        }
    }
}

void FileLogWriter::AddTargetFile(const std::string& filename)
{
    std::ios::openmode mode = std::ios_base::out | std::ios_base::app | std::ios_base::ate;

    std::ofstream* newStream = new std::ofstream();

    if (newStream)
    {
        newStream->open(filename, mode);

        if (newStream->is_open())
        {
            m_outputStreams.push_back(newStream);
        }
        else
        {
            // Bail and clean up the newStream;
            std::cout << "FileLogWriter: Failed to open a stream for " << filename << "." << std::endl;
            delete newStream;
            newStream = NULL;
        }
    }
    else
    {
        std::cout << "FileLogWriter: Failed to create a new output stream for " << filename << "." << std::endl;
    }
}


XTOOLS_NAMESPACE_END
