// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// StringUtils.h
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(StringUtils)

std::vector<std::string> Tokenize(const char *str, char c = ' ');

NAMESPACE_END(StringUtils)
XTOOLS_NAMESPACE_END
