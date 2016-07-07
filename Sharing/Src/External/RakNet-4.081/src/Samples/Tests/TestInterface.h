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


#include "RakString.h"
#include "DS_List.h"

using namespace RakNet;
class TestInterface
{
public:
	TestInterface();
	virtual ~TestInterface();
	virtual int RunTest(DataStructures::List<RakString> params,bool isVerbose,bool noPauses)=0;//should return 0 if no error, or the error number
	virtual RakString GetTestName()=0;
	virtual RakString ErrorCodeToString(int errorCode)=0;
	virtual void DestroyPeers()=0;
};
