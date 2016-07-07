// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// TypeUtils.h
// Templates to help with type manipulation
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

template<typename T> struct RemoveConst { typedef T type; };
template<typename T> struct RemoveConst<const T> { typedef T type; };

XTOOLS_NAMESPACE_END
