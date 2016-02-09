//////////////////////////////////////////////////////////////////////////
// PlatformUtils.h
//
// Collection of wrappers for platform-specific functions
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
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
