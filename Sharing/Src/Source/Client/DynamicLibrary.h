//////////////////////////////////////////////////////////////////////////
// DynamicLibrary.h
//
// Creates an abstraction over a dynamic library (DLL on Windows,
// dylib on OSX)
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#if !defined(SWIG)
# if defined(XTOOLS_PLATFORM_OSX)
#  include <dirent.h>
#  include <sys/stat.h>
#  include <dlfcn.h>
# endif
# include <Public/SideCarFunctions.h>
#endif

XTOOLS_NAMESPACE_BEGIN

class DynamicLibrary : public RefCounted
{
public:
	DynamicLibrary(const utility::char_t* const path);
	
	bool IsValid() const;
	SideCarEntryPointFunc GetEntryPoint() const;
	LogFuncSetter GetLogFunc() const;
	ProfileFuncSetter GetProfileFunc() const;
	const char* GetErrorMessage() const;
	
private:
#if defined(XTOOLS_PLATFORM_WINDOWS_DESKTOP)
	HINSTANCE m_libraryHandle;
#elif defined(XTOOLS_PLATFORM_OSX)
	void* m_libraryHandle;
	const char* m_error;
#endif
};

DECLARE_PTR(DynamicLibrary)

XTOOLS_NAMESPACE_END