//////////////////////////////////////////////////////////////////////////
// SystemRole.h
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
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
