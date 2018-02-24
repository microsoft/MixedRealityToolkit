//
// VersionInfo.h
// Header for standard version information.
//

#pragma once

#if !defined(SWIG)

#define XTOOLS_VERSION 1,1,0,1
#define XTOOLS_VERSION_STRING L"1.1.0.1"

#if defined(WIN32)
#define XTOOLS_FILE_VERSION_STRING L"1.1.0.1 Win32.Release"
#elif defined(WIN64)
#define XTOOLS_FILE_VERSION_STRING L"1.1.0.1 x64.Release"
#else
#define XTOOLS_FILE_VERSION_STRING L"1.1.0.1 Release"
#endif
#endif
