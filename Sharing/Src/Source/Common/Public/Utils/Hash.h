// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// Hash.h
// Utility for creating a hash from a string
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

uint64 HashString(const char* str);

uint64 HashBuffer(const char* buffer, uint64 count);

XTOOLS_NAMESPACE_END
