//////////////////////////////////////////////////////////////////////////
// Hash.h
//
// Utility for creating a hash from a string
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

uint64 HashString(const char* str);

uint64 HashBuffer(const char* buffer, uint64 count);

XTOOLS_NAMESPACE_END
