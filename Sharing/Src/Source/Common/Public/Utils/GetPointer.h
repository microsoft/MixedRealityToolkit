// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// GetPointer.h
// Provide a common way for templated code to ensure they get a naked pointer for
// and smart pointer type they are given.  
// Each smart point should provide its own templated version of get_pointer,
// so other templated code can call get_pointer(X) to always get a naked pointer.  
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

template<class T> 
T* get_pointer(T* p)
{
	return p;
}

template<class T>
const T* get_pointer(const T* p)
{
	return p;
}

XTOOLS_NAMESPACE_END
