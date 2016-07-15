// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// stdafx.h
// Precompiled header for the XTools Client library
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
#include <Public/DownloadManager.h>
#include <Public/SideCarContext.h>
#include <Public/SideCar.h>
#include "ClientWrapperAPI.h"
#include "ZXingIncludes.h"
#include "AprilTagIncludes.h"
