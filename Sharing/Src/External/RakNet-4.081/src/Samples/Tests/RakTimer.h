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

#pragma once


#include "RakNetTime.h"
#include "GetTime.h"

using namespace RakNet;
class RakTimer
{
public:
	RakTimer(void);
	RakTimer(int lengthInMilliseconds);
	~RakTimer(void);
    void SetTimerLength(int lengthInMilliseconds);
    void Start();
    void Pause();
    void Resume();
	bool IsExpired();
private:
	int timerLength;//Modified by SetTimerLength
	int pauseOffset;
	TimeMS startTime;
};
