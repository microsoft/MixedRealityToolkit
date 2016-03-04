//////////////////////////////////////////////////////////////////////////
// SyncManager.h
//
// Main class for the Operational Transform sync system.  Used by all 
// parts of the XTools system to keep arbitrary data in sync
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

class SyncManager : public RefCounted, public Reflection::XTObject, public IUpdateable
{
	XTOOLS_REFLECTION_DECLARE(SyncManager)

public:
	// Create a new SyncManager.  Pass in the level of authority that this sync manager should have
	// relative to the other sync managers it connects to
	static ref_ptr<SyncManager> Create(MessageID messageID, AuthorityLevel authorityLevel, const UserPtr& localUser);

	// Register a listener to receive notifications before and after remote changes to sync data are applied.
	// Returns true if successfully registered, false if another listener is already registered
	virtual bool RegisterListener(SyncListener* listener) = 0;

	// Add the given connect to the list of remote peers to sync data with.  
	// Syncing will not occur until the connection is fully connected
	virtual void AddConnection(const NetworkConnectionPtr& newConnection) = 0;

	// Remove the given connection from the list of remote peers to sync data with.  
	virtual void RemoveConnection(const NetworkConnectionPtr& connection) = 0;

	// Returns the top-level element in the hierarchy of synced data.  This object
	// is automatically created when the SyncManager is created, and is common across all 
	// synced systems
	virtual ObjectElementPtr GetRootObject() = 0;

	// For debugging, means there are no pending local changes, and no outstanding messages we are waiting for 
	// from our remote peers
	virtual bool IsFullySynced() const = 0;

	virtual const UserPtr& GetLocalUser() const = 0;

	// Print all of the elements currently in the shared sync state.  
	virtual void PrintSyncData() const = 0;
};

DECLARE_PTR(SyncManager)

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END