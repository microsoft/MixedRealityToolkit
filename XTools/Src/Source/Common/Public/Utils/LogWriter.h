//////////////////////////////////////////////////////////////////////////
// LogWriter.h
//
// Base class for objects to receive logging from xtools
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#ifdef SWIG
%feature("director") XTools::LogWriter;
#endif

// Disable unused parameter warnings.  Pure virtual functions don't play nice with SWIG, but we still want 
// to see the names of the parameters to make reading the code clearer
#pragma warning( push )
#pragma warning( disable : 4100 ) 

XTOOLS_NAMESPACE_BEGIN

class LogWriter
{
public:
	virtual ~LogWriter() {}
	virtual void WriteLogEntry(LogSeverity severity, const std::string& message) {}
};

XTOOLS_NAMESPACE_END

#pragma warning( pop )
