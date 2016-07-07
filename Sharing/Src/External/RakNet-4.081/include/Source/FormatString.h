// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant 
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

/// \file FormatString.h
///


#ifndef __FORMAT_STRING_H
#define __FORMAT_STRING_H

#include "Export.h"

extern "C" {
char * FormatString(const char *format, ...);
}
// Threadsafe
extern "C" {
char * FormatStringTS(char *output, const char *format, ...);
}


#endif

