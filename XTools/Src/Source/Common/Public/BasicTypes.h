//////////////////////////////////////////////////////////////////////////
// BasicTypes.h
//
// Define types so that they are consistent across platforms and bit depths
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#ifndef SWIG
// This is actually needed, depending on include order
#ifndef NULL
#define NULL 0
#endif
#endif

XTOOLS_NAMESPACE_BEGIN

#if defined(XTOOLS_PLATFORM_WINDOWS_DESKTOP) || defined(XTOOLS_PLATFORM_WINRT)

typedef unsigned char		byte;
typedef signed short		int16;
typedef unsigned short		uint16;
typedef signed int			int32;
typedef unsigned int		uint32;
typedef long long			int64;
typedef unsigned long long	uint64;

#elif defined(XTOOLS_PLATFORM_OSX)

typedef unsigned char		byte;
typedef signed short		int16;
typedef unsigned short		uint16;
typedef signed int          int32;
typedef unsigned int        uint32;
typedef long long           int64;
typedef unsigned long long	uint64;

#else
# error Undefined Platform

#endif

#if defined(XTOOLS_32BIT)
typedef uint32				uintptr;
#elif defined(XTOOLS_64BIT)
typedef uint64				uintptr;
#else
# error Please ensure that XTOOLS_32BIT or XTOOLS_64BIT get defined for all builds
#endif

#if !defined(SWIG)
static_assert(sizeof(byte  ) == 1, "size of byte is not 1");
static_assert(sizeof(int16 ) == 2, "size of int16 is not 2");
static_assert(sizeof(uint16) == 2, "size of uint16 is not 2");
static_assert(sizeof(int32 ) == 4, "size of int32 is not 4");
static_assert(sizeof(uint32) == 4, "size of uint32 is not 4");
static_assert(sizeof(int64 ) == 8, "size of int64 is not 8");
static_assert(sizeof(uint64) == 8, "size of uint64 is not 8");
static_assert(sizeof(uintptr) == sizeof(void*), "uintptr is not the same size as void*");
#endif

XTOOLS_NAMESPACE_END
