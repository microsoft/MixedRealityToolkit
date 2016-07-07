// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// SyncAuthorityLevel.h
// Each machine in the sync system should be assigned an authority level.
// This is used to keep conflict resolution consistent as changes propagate.  
// Machines should never be connected to other machines with the same auth level;
// it should be a tree with the root having highest level and the leaves the lowest
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

enum AuthorityLevel : byte
{
	High = 0,	// Session server
	Medium,		// Primary Client
	Low,		// Secondary Client
	Unknown
};

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
