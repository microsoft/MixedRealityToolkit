//////////////////////////////////////////////////////////////////////////
// Environment.h
//
// Define types, settings and macros used across the apps
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#if !defined(SWIG) && !defined(__APPLE__)
# ifndef NOMINMAX
#  define NOMINMAX // Using std::min and std::max
# endif
# include <winapifamily.h>	// for WINAPI_FAMILY_PARTITION
#endif

// Make the beginning and ending of name spaces clearer, and avoids auto-indenting
#define XTOOLS_NAMESPACE_BEGIN namespace XTools {
#define XTOOLS_NAMESPACE_END }

#define NAMESPACE_BEGIN(X) namespace X {
#define NAMESPACE_END(X) }

// Define our own debug and release config macros, so we can control the logic for when 
// each is used
#if defined( DEBUG ) || defined( _DEBUG ) || (defined(CONFIGURATION) && CONFIGURATION == Debug)
# define XTOOLS_DEBUG
#else
# define XTOOLS_RELEASE
#endif

// Define macros for 32 or 64 bit
#if defined(_M_X64) || defined(__LP64__)
# define XTOOLS_64BIT
#elif defined(_M_IX86) || defined(_M_ARM)
# define XTOOLS_32BIT
#else
# error Unsupported bit depth
#endif

// Define per-platform macros
#if defined(SWIG)
# define XTOOLS_PLATFORM_WINDOWS_DESKTOP

#elif __APPLE__
# include "TargetConditionals.h"
# if TARGET_OS_MAC
#  define XTOOLS_PLATFORM_OSX
# elif TARGET_IPHONE_SIMULATOR
#  define XTOOLS_PLATFORM_IOS_SIMULATOR
# elif TARGET_OS_IPHONE
#  define XTOOLS_PLATFORM_IOS
# endif

#elif WINAPI_FAMILY_PARTITION( WINAPI_PARTITION_DESKTOP )
# define XTOOLS_PLATFORM_WINDOWS_DESKTOP

#elif WINAPI_FAMILY_PARTITION( WINAPI_PARTITION_APP )
# define XTOOLS_PLATFORM_WINRT

#else
# error Unsupported Platform
#endif

// Convenience macro for multiple platforms
#if defined(XTOOLS_PLATFORM_WINDOWS_DESKTOP) || defined(XTOOLS_PLATFORM_WINRT)
# define XTOOLS_PLATFORM_WINDOWS_ANY
#endif

// Cross-platform macro to avoid unreferenced parameter or variable warnings
#define XT_UNREFERENCED_PARAM(X) (void)(X)

// Define custom assert macro, so we can more easily control when it is defined and what it does
#if defined(SWIG)
# define XTASSERT(_Expression) (void)(0)
# define XTVERIFY(_Expression) (_Expression)	// Same as XTASSERT but should never get compiled out
#else
XTOOLS_NAMESPACE_BEGIN
void DoAssert(const char* message, const char* file, unsigned line);
XTOOLS_NAMESPACE_END
# define XTASSERT(_Expression) (void)( (!!(_Expression)) || (::XTools::DoAssert(#_Expression, __FILE__, __LINE__), 0) )
# define XTVERIFY(_Expression) ( (!!(_Expression)) || (::XTools::DoAssert(#_Expression, __FILE__, __LINE__), 0) )
#endif

// Define macros for Microsoft override specifiers so that they don't cause compile errors on non-Microsoft platforms
#if defined(_MSC_VER) && !defined(SWIG)
# pragma warning( disable : 4481 )	// Disable warning about using nonstandard extension 'abstract'
# define XTOVERRIDE override		// indicates that a member of a type overrides a base class or a base interface member
# define XTABSTRACT abstract		// either: A type can be used as a base type, but the type itself cannot be instantiated, or a type member function can be defined only in a derived type.
# define XTFINAL final				// indicates that a virtual member cannot be overridden, or that a type cannot be used as a base type
# define XTDLLEXPORT __declspec(dllexport)
#else
# define XTOVERRIDE
# define XTABSTRACT
# define XTFINAL
# define XTDLLEXPORT __attribute__((visibility("default")))
#define vsnprintf_s vsnprintf
#endif