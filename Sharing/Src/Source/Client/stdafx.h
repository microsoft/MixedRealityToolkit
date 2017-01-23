//////////////////////////////////////////////////////////////////////////
// stdafx.h
//
// Precompiled header for the XTools Client library
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

#include <Common.h>
#include <Public/SideCarContext.h>
#include <Public/SideCar.h>
#include "ClientWrapperAPI.h"
#include "ZXingIncludes.h"
