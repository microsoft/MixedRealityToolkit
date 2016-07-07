// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// XToolsTypes.h
// Contains definitions for simple or typedef'd types
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

// Unique identifier for each socket connection that is made.  
// Note that this CANNOT be guaranteed to uniquely identify NetworkConnections, as
// they can be bound to multiple different connection sockets over their lifetime.  
typedef uint64 SocketID;

// Value to use for invalid or inactive sockets
const SocketID kInvalidSocketID = 0xFFFFFFFFFFFFFFFF;



// Unique identifier for each network connection that is made.  
typedef uint64 ConnectionGUID;

// Value to use for invalid or inactive connections
const ConnectionGUID kInvalidConnectionGUID = 0xFFFFFFFFFFFFFFFF;

XTOOLS_NAMESPACE_END
