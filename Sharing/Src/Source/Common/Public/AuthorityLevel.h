//////////////////////////////////////////////////////////////////////////
// SyncAuthorityLevel.h
//
// Each machine in the sync system should be assigned an authority level.
// This is used to keep conflict resolution consistent as changes propagate.  
// Machines should never be connected to other machines with the same auth level;
// it should be a tree with the root having highest level and the leaves the lowest
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
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
