//////////////////////////////////////////////////////////////////////////
// SideCarFunctions.h
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

typedef ::XTools::SideCar*(*SideCarEntryPointFunc)();

typedef void(*LogFunc)(::XTools::LogSeverity severity, const char* file, int line, const char* message);
typedef void(*LogFuncSetter)(LogFunc);

typedef ::XTools::ProfileManagerPtr(*ProfilerFunc)();
typedef void(*ProfileFuncSetter)(ProfilerFunc);

// Implement this function in your SideCar dll, and have it return a new instance of your
// custom implementation of SideCar.  Although this passes a naked pointer, the returned object
// will immediately be wrapped in a ref_ptr<SideCar>, so that it will be automatically deleted
// when no longer needed
extern "C" XTDLLEXPORT XTools::SideCar* CreateSideCar();

extern "C" XTDLLEXPORT void SetLogFunction(LogFunc func);

extern "C" XTDLLEXPORT void SetProfileFunc(ProfilerFunc func);
