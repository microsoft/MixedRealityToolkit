//////////////////////////////////////////////////////////////////////////
// LogManager.h
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

enum LogSeverity : byte
{
	Info,
	Warning,
	Error
};

class LogManager : public AtomicRefCounted
{
public:
	virtual ~LogManager() {}
	virtual void Log(LogSeverity severity, const char* message) = 0;
};

DECLARE_PTR(LogManager)

#if !defined(SWIG) && !defined(XTOOLS_SIDECAR)
extern void LogNative(::XTools::LogSeverity severity, const char* file, int line, const char* message);

extern void LogFormat(::XTools::LogSeverity severity, const char* file, int line, const char* message, ...);

#define LogInfo(X, ...) \
	::XTools::LogFormat(::XTools::LogSeverity::Info, __FILE__, __LINE__, X, ##__VA_ARGS__)

#define LogWarning(X, ...) \
	::XTools::LogFormat(::XTools::LogSeverity::Warning, __FILE__, __LINE__, X, ##__VA_ARGS__)

#define LogError(X, ...) \
	::XTools::LogFormat(::XTools::LogSeverity::Error, __FILE__, __LINE__, X, ##__VA_ARGS__)
#endif

XTOOLS_NAMESPACE_END
