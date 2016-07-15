// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#ifndef __DECLSPEC_H__
#define __DECLSPEC_H__

#if defined(_WIN32) && !defined(STATICLIB)
	#ifdef MINIUPNP_EXPORTS
		#define LIBSPEC __declspec(dllexport)
	#else
		#define LIBSPEC __declspec(dllimport)
	#endif
#else
	#define LIBSPEC
#endif

#endif

