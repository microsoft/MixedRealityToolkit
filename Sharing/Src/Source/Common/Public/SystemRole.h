// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// SystemRole.h
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

enum SystemRole : byte
{
	SessionDiscoveryServerRole = 0,
	SessionServerRole,
	PrimaryClientRole,
	SecondaryClientRole
};

XTOOLS_NAMESPACE_END
