//////////////////////////////////////////////////////////////////////////
// SharingManagerImpl.h
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <Public/SideCarContext.h>

XTOOLS_NAMESPACE_BEGIN

class SharingManagerImpl : public SharingManager, public NetworkConnectionListener
{
public:
	SharingManagerImpl(const ClientConfigPtr& config);
	virtual ~SharingManagerImpl();

	virtual const SessionManagerPtr& GetSessionManager() const XTOVERRIDE;

	virtual const UserPresenceManagerPtr& GetUserPresenceManager() const XTOVERRIDE;

	virtual const AudioManagerPtr& GetAudioManager() const XTOVERRIDE;

	// Returns the PairingManager, which allows two clients to connect to each other
	virtual const PairingManagerPtr& GetPairingManager() const XTOVERRIDE;

	// Returns the root element in the sync data set.  There is always a root object
	virtual ObjectElementPtr GetRootSyncObject() XTOVERRIDE;

	/// Returns the RoomManager, which allows users to define which physical real-world rooms they
	/// are in, which allows for appropriate sharing of spatial tracking anchors on HoloLens and 
	/// ensures voice audio gets sent to only people who are not in the same room.  
	virtual const RoomManagerPtr& GetRoomManager() const XTOVERRIDE;

	// Register a listener object to receive notifications before and after the sync system
	// sends notifications that sync elements have been changed by remote peers.
	// Returns true if successful, false if registration fails
	virtual bool RegisterSyncListener(SyncListener* listener) XTOVERRIDE;

	// Process any received messages
	virtual void Update() XTOVERRIDE;

	// Get the connection to the Baraboo visualizer for this app, so that custom
	// messages can be sent back and forth.  
	// Note that the visualizer may not be connected yet, but the NetworkConnection
	// provides hooks to notify you when that happens.  
	virtual NetworkConnectionPtr GetPairedConnection() XTOVERRIDE;

	// Get the connection to the server.  This may or may not already be connected.
	// Not currently expected to be used by desktop app code, but exposed just in case
	virtual NetworkConnectionPtr GetServerConnection() XTOVERRIDE;

#if defined(MSTEST)
	// Expose the connect to the session list server on in builds for Test.  This connection is
	// used to tell multiple remote clients to join and leave sessions by automated tests. 
	virtual NetworkConnectionPtr			GetListServerConnection() XTOVERRIDE;
#endif

	// Get the settings that were loaded in from the config file
	virtual const SettingsPtr& GetSettings() XTOVERRIDE;

	virtual void SetServerConnectionInfo(const XStringPtr& address, uint32 port) XTOVERRIDE;

	// Returns an object representing the local user.  
	// Note that this will not return null, but the user can be invalid.  
	// Check with User.IsValid() before using.  
	virtual UserPtr GetLocalUser() XTOVERRIDE;

	virtual void SetUserName(const XStringPtr& name) XTOVERRIDE;

private:

	// NetworkConnectionListener functions.  Used to listen to custom messages from the session server
	virtual void OnConnected(const NetworkConnectionPtr& connection) XTOVERRIDE;
	virtual void OnConnectFailed(const NetworkConnectionPtr& connection) XTOVERRIDE;
	virtual void OnDisconnected(const NetworkConnectionPtr& connection) XTOVERRIDE;

	// Local Functions:
	void ConnectToServer();
	void SetupSideCars();

	void OnListServerHandshakeComplete(const XSocketPtr& newConnection, SocketID socketID, HandshakeResult result);

	std::vector<utility::string_t> GetLibrariesInFolder(const utility::string_t& folder) const;
	void LoadSideCar(const utility::string_t& sidecarFolder, const utility::string_t& library);

	// Keep a list of pointers to each of the subsystems that need to be updated
	// each time the SharingManager is updated.  
	std::vector<IUpdateable*>	m_updateableSubsystems;

	// Sidecar Related
	struct SideCarInfo
	{
		SideCarPtr				m_sidecar;
		utility::string_t		m_name;
	};

	std::vector<SideCarInfo>	m_sidecars;
	SideCarContextPtr			m_sidecarContext;

	// Session Related
	ClientContextConstPtr		m_clientContext;
	SessionManagerPtr			m_sessionManager;
	PairingManagerPtr			m_paringManager;
	ProfileManagerPtr			m_profileManager;
	RoomManagerPtr				m_roomManager;

	SettingsPtr					m_settings;
	LoggerPtr					m_logManager;

	NetworkHandshakePtr			m_listServerHandshake;

	bool                        m_retryServerConnectionRequired;

	clock_t                     m_lastServerRetryAttemptTick;

	static const int            s_serverConnectRetryRateTicks;
};

DECLARE_PTR(SharingManagerImpl)

XTOOLS_NAMESPACE_END
