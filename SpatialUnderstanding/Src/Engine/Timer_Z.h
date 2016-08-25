// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#ifndef _TIMER_Z_H
#define _TIMER_Z_H

#include <BeginDef_Z.h>

EXTERN_Z Float	GetAbsoluteTime(void);		// Time since start, single precision float seconds
EXTERN_Z Double	GetDAbsoluteTime(void);		// Time since start, double precision float seconds
EXTERN_Z U64 GetAbsoluteTickCount();
EXTERN_Z void CalibrateTimer(void);

#endif
