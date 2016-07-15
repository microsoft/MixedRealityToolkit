// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// PlatformUtils.h
// Collection of wrappers for platform-specific functions
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Platform)

// sleep the calling thread for the given number of milliseconds.  Not accurate; do not use as a timer!
void SleepMS(uint32 milliseconds);

std::string GetLocalMachineNetworkName();

uint64 GetThreadID();

NAMESPACE_END(Platform)
XTOOLS_NAMESPACE_END
