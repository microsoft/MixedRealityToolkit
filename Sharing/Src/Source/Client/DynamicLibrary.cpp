//////////////////////////////////////////////////////////////////////////
// DynamicLibrary.cpp
//
// Creates an abstraction over a dynamic library (DLL on Windows,
// dylib on OSX)
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DynamicLibrary.h"

XTOOLS_NAMESPACE_BEGIN

DynamicLibrary::DynamicLibrary(const utility::char_t* const path)
{
#if defined(XTOOLS_PLATFORM_WINDOWS_DESKTOP)
	m_libraryHandle = LoadLibrary(path);
#elif defined(XTOOLS_PLATFORM_OSX)
	m_libraryHandle = dlopen(path, RTLD_LAZY);
	if (!m_libraryHandle)
	{
		m_error = dlerror();
	}
	else
	{
		m_error = nullptr;
	}
#else
	XT_UNREFERENCED_PARAM(path);
	// TODO: Support for platforms other than Windows Desktop and OSX
#endif
}

bool DynamicLibrary::IsValid() const
{
#if defined(XTOOLS_PLATFORM_WINDOWS_DESKTOP)
	return m_libraryHandle != NULL;
#elif defined(XTOOLS_PLATFORM_OSX)
	return m_libraryHandle != nullptr;
#else
	// TODO: Support for platforms other than Windows Desktop and OSX
	return false;
#endif
}

SideCarEntryPointFunc DynamicLibrary::GetEntryPoint() const
{
#if defined(XTOOLS_PLATFORM_WINDOWS_DESKTOP)
	return (SideCarEntryPointFunc)GetProcAddress(m_libraryHandle, "CreateSideCar");
#elif defined(XTOOLS_PLATFORM_OSX)
	return (SideCarEntryPointFunc)dlsym(m_libraryHandle, "CreateSideCar");
#else
	// TODO: Support for platforms other than Windows Desktop and OSX
	return nullptr;
#endif
}

LogFuncSetter DynamicLibrary::GetLogFunc() const
{
#if defined(XTOOLS_PLATFORM_WINDOWS_DESKTOP)
	return (LogFuncSetter)GetProcAddress(m_libraryHandle, "SetLogFunction");
#elif defined(XTOOLS_PLATFORM_OSX)
	return (LogFuncSetter)dlsym(m_libraryHandle, "SetLogFunction");
#else
	// TODO: Support for platforms other than Windows Desktop and OSX
	return nullptr;
#endif
}

ProfileFuncSetter DynamicLibrary::GetProfileFunc() const
{
#if defined(XTOOLS_PLATFORM_WINDOWS_DESKTOP)
	return (ProfileFuncSetter)GetProcAddress(m_libraryHandle, "SetProfileFunc");
#elif defined(XTOOLS_PLATFORM_OSX)
	return (ProfileFuncSetter)dlsym(m_libraryHandle, "SetProfileFunc");
#else
	// TODO: Support for platforms other than Windows Desktop and OSX
	return nullptr;
#endif
}

const char* DynamicLibrary::GetErrorMessage() const
{
#if defined(XTOOLS_PLATFORM_WINDOWS_DESKTOP)
	return nullptr;
#elif defined(XTOOLS_PLATFORM_OSX)
	return m_error;
#else
	// TODO: Support for platforms other than Windows Desktop and OSX
	return nullptr;
#endif
}

XTOOLS_NAMESPACE_END