// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// Profile.h
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class Profile
{
public:
	static void BeginRange(const std::string& name);
	static void EndRange();
};

XTOOLS_NAMESPACE_END
