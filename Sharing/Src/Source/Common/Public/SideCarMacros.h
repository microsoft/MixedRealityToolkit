//////////////////////////////////////////////////////////////////////////
// SideCarMacros.h
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <stdarg.h>

#define XT_DECLARE_SIDECAR(SideCarName) \
	extern ProfilerFunc gProfilerFunction; \
	extern void SideCarLog(::XTools::LogSeverity severity, const char* file, int line, const char* message, ...);

#define XT_DEFINE_SIDECAR(SideCarName) \
	LogFunc gLoggingFunction = nullptr; \
	ProfilerFunc gProfilerFunction = nullptr; \
	::XTools::SideCar* CreateSideCar() { return new SideCarName(); } \
	void SetLogFunction(LogFunc func) { gLoggingFunction = func; } \
	void SetProfileFunc(ProfilerFunc func) { gProfilerFunction = func; } \
	void SideCarLog(::XTools::LogSeverity severity, const char* file, int line, const char* message, ...) { \
		char buffer[512]; \
		va_list ap; \
		va_start(ap, message); \
		vsnprintf_s(buffer, sizeof(buffer), message, ap); \
		va_end(ap); \
		if (gLoggingFunction) gLoggingFunction(severity, file, line, buffer); \
				} 

#if defined(XTOOLS_SIDECAR)
#define LogInfo(X, ...) \
	::SideCarLog(::XTools::LogSeverity::Info, __FILE__, __LINE__, X, ##__VA_ARGS__)

#define LogWarning(X, ...) \
	::SideCarLog(::XTools::LogSeverity::Warning, __FILE__, __LINE__, X, ##__VA_ARGS__)

#define LogError(X, ...) \
	::SideCarLog(::XTools::LogSeverity::Error, __FILE__, __LINE__, X, ##__VA_ARGS__)
#endif