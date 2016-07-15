// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// SwigHelperMacros.h
//////////////////////////////////////////////////////////////////////////

#pragma once

// Macro to enable classes to act as listeners for callbacks.  
// The director feature allows C++ calls to these functions to cause the Java or C# code overloading the function to execute.
// The listener_declare
#if defined(SWIG)
#define XT_LISTENER_DECLARE(LISTENERTYPE) \
%feature("director") LISTENERTYPE;

#else
#define XT_LISTENER_DECLARE(X)

#endif
