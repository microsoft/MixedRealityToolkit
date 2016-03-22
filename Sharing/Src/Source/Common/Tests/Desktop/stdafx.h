// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#if defined(_WIN32)
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
# endif
#endif

#include <windows.h>
#include <Common.h>
#include <CommonPrivate.h>

// Headers for CppUnitTest
#include "CppUnitTest.h"