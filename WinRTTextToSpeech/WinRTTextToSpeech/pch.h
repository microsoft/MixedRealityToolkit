// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.

#ifndef PCH_H
#define PCH_H

// Exclude rarely-used stuff from Windows headers
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <unknwn.h>

#ifndef DLLEXPORT
#define DLLEXPORT __declspec(dllexport)
#endif

#endif //PCH_H
