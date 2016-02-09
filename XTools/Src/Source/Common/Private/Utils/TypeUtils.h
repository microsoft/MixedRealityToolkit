//////////////////////////////////////////////////////////////////////////
// TypeUtils.h
//
// Templates to help with type manipulation
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

template<typename T> struct RemoveConst { typedef T type; };
template<typename T> struct RemoveConst<const T> { typedef T type; };

XTOOLS_NAMESPACE_END