//////////////////////////////////////////////////////////////////////////
// SharingManager.h
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

DECLARE_PTR_PRE(SharingManager)

/// Main class responsible for initializing the primary systems and allowing
/// access to the creation of other objects
class SharingManager : public AtomicRefCounted
{
public:
	/// Create a new SharingManager instance.  There should only be one per app
	static ref_ptr<SharingManager>			Create(const ClientConfigPtr& config);

	/// Returns the \ref SessionManager, which keeps track of available sessions and allows
	/// this client to create, join, and leave sessions
	virtual const SessionManagerPtr&		GetSessionManager() const = 0;

	virtual const UserPresenceManagerPtr&	GetUserPresenceManager() const = 0;

	virtual const AudioManagerPtr&			GetAudioManager() const = 0;

	/// Returns the PairingManager, which allows two clients to connect to each other
	virtual const PairingManagerPtr&		GetPairingManager() const = 0;

	/// Returns the RoomManager, which allows users to define which physical real-world rooms they
	/// are in, which allows for appropriate sharing of spatial tracking anchors on HoloLens and 
	/// ensures voice audio gets sent to only people who are not in the same room.  
	virtual const RoomManagerPtr&			GetRoomManager() const = 0;

	/// Returns the root element in the sync data set.  There is always a root object
	virtual ObjectElementPtr				GetRootSyncObject() = 0;

	/// Register a listener object to receive notifications before and after the sync system
	/// sends notifications that sync elements have been changed by remote peers.
	/// Returns true if successful, false if registration fails
	virtual bool							RegisterSyncListener(SyncListener* listener) = 0;

	/// Ticks the entire XTools system.  Causes any network messages that have been received to be processed and
	/// any registered callbacks to be called.  This function should be called frequently (eg: once per frame).  
	virtual void							Update() = 0;

	/// Get the connection to the Baraboo visualizer for this app, so that custom
	/// messages can be sent back and forth.  
	/// Note that the visualizer may not be connected yet, but the NetworkConnection
	/// provides hooks to notify you when that happens.  
	virtual NetworkConnectionPtr			GetPairedConnection() = 0;

	/// Get the connection to the server.  This may or may not already be connected.
	/// Not currently expected to be used by desktop app code, but exposed just in case
	virtual NetworkConnectionPtr			GetServerConnection() = 0;

#if defined(MSTEST)
	/// Expose the connect to the session list server on in builds for Test.  This connection is
	/// used to tell multiple remote clients to join and leave sessions by automated tests. 
	virtual NetworkConnectionPtr			GetListServerConnection() = 0;
#endif

	/// Get the settings that were loaded in from the config file
	virtual const SettingsPtr&				GetSettings() = 0;

	virtual void							SetServerConnectionInfo(const XStringPtr& address, uint32 port) = 0;

	/// Returns an object representing the local user.  
	/// Note that this will not return null, but the user can be invalid.  
	/// Check with User.IsValid() before using.  
	virtual UserPtr							GetLocalUser() = 0;

	virtual void							SetUserName(const XStringPtr& name) = 0;
};

DECLARE_PTR_POST(SharingManager)

XTOOLS_NAMESPACE_END
