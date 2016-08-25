// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef	TYPEINFO_Z_H
#define TYPEINFO_Z_H

#define _USE_RTTI	// Defined by default

// Exclusion - turnoff RTTI for some configuration
#if defined(_MASTER)
	#undef _USE_RTTI
#elif defined(__SPU__)
	#undef _USE_RTTI	// No RTTI on SPU code.
#elif defined(__clang__)
	#if !__has_feature(cxx_rtti)
		#undef _USE_RTTI
	#endif
#elif defined(_SUBMISSION)
	#undef _USE_RTTI	// Remove RTTI on SUBMISSION for every platform
#endif

#if defined(_USE_RTTI)
	#include <typeinfo>
	template<class T > const Char * TYPEINFO_Z ( T *Ptr)	{ return typeid(Ptr).name(); };
#else
	template<class T > const Char * TYPEINFO_Z ( T *Ptr)	{ return "NoTypeInfo"; };
#endif

#endif
