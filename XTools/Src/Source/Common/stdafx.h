//////////////////////////////////////////////////////////////////////////
// stdafx.h
//
// Precompiled header for the XTools Common library
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#if defined(_WIN32)
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN	// Exclude rarely-used stuff from Windows headers
# endif
# ifndef NOMINMAX
#  define NOMINMAX				// Using std::min and std::max
# endif
#include <windows.h>
#endif

#include <string>
#include <stddef.h>
#include "Common.h"
#include "CommonPrivate.h"
