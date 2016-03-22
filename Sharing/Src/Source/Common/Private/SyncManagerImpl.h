//////////////////////////////////////////////////////////////////////////
// SyncManagerImpl.h
//
// Implementation of the Sync Manager interface.  
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

class SyncManagerImpl : public SyncManager, public NetworkConnectionListener
{
	XTOOLS_REFLECTION_DECLARE(SyncManagerImpl)

public:
	SyncManagerImpl(MessageID messageID, AuthorityLevel authorityLevel, const UserPtr& localUser);

	// SyncManager Functions:
	virtual bool RegisterListener(SyncListener* listener) XTOVERRIDE;
	virtual void AddConnection(const NetworkConnectionPtr& newConnection) XTOVERRIDE;
	virtual void RemoveConnection(const NetworkConnectionPtr& connection) XTOVERRIDE;

	virtual ObjectElementPtr GetRootObject() XTOVERRIDE;

	// For debugging, means there are no pending local changes, and no outstanding messages we are waiting for 
	// from our remote peers
	virtual bool IsFullySynced() const XTOVERRIDE;
	virtual const UserPtr& GetLocalUser() const XTOVERRIDE;
	virtual void PrintSyncData() const XTOVERRIDE;

	// Parses all incoming changes, notifies listeners of the changes, and sends updates to the remote peers.
	// Should be called after each time the ConnectionManager updates.  
	virtual void Update() XTOVERRIDE;

	// Local Functions:
	const SyncContextPtr& GetContext() const;

private:
	struct OperationalState
	{
		OperationalState() : m_opsSent(0), m_opsReceived(0) {}

		// By convention these values should be relative to the machine it is referring to.  
		// Ie: 'sent' is the number of operations sent by that local machine
		int32	m_opsSent;				// The number of operations sent to the remote peer
		int32	m_opsReceived;			// The number of operations received from the remote peer
	};

	struct VersionedOp
	{
		VersionedOp() {}
		VersionedOp(const OperationConstPtr& op, OperationalState state) : m_op(op), m_state(state) {}

		OperationConstPtr	m_op;
		OperationalState	m_state;	// The state the machine was in before the op was applied
	};
	typedef std::list<VersionedOp> VersionedOpList;

	struct RemoteSyncPeer
	{
		RemoteSyncPeer() 
			: m_lastAcknowledgedVersion(0)
			, m_systemID(kUnknownSystemID)
			, m_authorityLevel(AuthorityLevel::Unknown)
			, m_userID(User::kInvalidUserID)
			, m_bHandshakeComplete(false)
			, m_bHasAddedStartingState(false)
		{}

		void SendOp(const OperationConstPtr& op);

		VersionedOpList				m_incoming;					// Operations that have been received but have not yet be applied to the local state
		VersionedOpList				m_outgoing;					// Operations that have been applied locally but have not been processed by the remote peer
		VersionedOpList				m_sendList;
		NetworkConnectionPtr		m_connection;				// Connection to the remote peer
		ReceiptPtr					m_listenerReceipt;			// Receipt for listening for events from the connection
		OperationalState			m_localState;				// Our view of the number of operations sent and received
		OperationalState			m_remoteState;				// What the remote peer has told us about the number of operations it has sent and received
		int32						m_lastAcknowledgedVersion;
		SystemID					m_systemID;					// The Unique ID of the remote system
		AuthorityLevel				m_authorityLevel;			// The level of authority of the remote peer for resolving conflicts.  Cannot match our auth level
		std::string					m_userName;					// The name of the remote user
		UserID						m_userID;					// The ID of this remote user
		bool						m_bHandshakeComplete;		// Track whether this peer has completed its handshake
		bool						m_bHasAddedStartingState;	// Track whether we have sent our starting state when this remote peer connects to us
	};

	class UserDiscriminator;
	class OnlyUser;
	class AllButUser;


	// NetworkConnectionListener functions:
	virtual void OnConnected(const NetworkConnectionPtr& connection) XTOVERRIDE;
	virtual void OnDisconnected(const NetworkConnectionPtr& connection) XTOVERRIDE;
	virtual void OnMessageReceived(const NetworkConnectionPtr& connection, NetworkInMessage& message) XTOVERRIDE;

	// Local Functions:
	void UnregisterListener(SyncListener* listener);
	void OnIdentityInfoReceived(const NetworkConnectionPtr& connection, NetworkInMessage& msg);
	void OnSyncChangeReceived(const NetworkConnectionPtr& connection, NetworkInMessage& msg);
	bool GetIndexOfConnection(const NetworkConnectionPtr& connection, size_t& indexOut) const;

	void SendSyncMessage(RemoteSyncPeer& remotePeer);

	void SendHandshakeMessage(RemoteSyncPeer& remotePeer);

	void AddCreateOpForElementRecurs(RemoteSyncPeer& peer, const ElementPtr& element);

	void DeleteDataFromUser(UserID userID);
	void DeleteDataFromUserRecurs(const ElementPtr& element, const UserDiscriminator& discriminator);

	// List of the systems we are syncing data with.  We implicitly act a a bridge;
	// whenever we receive changes from a remote system, we forward it to everyone else
	std::vector<RemoteSyncPeer>		m_remoteHosts;
	
	// Holds all the objects that are used by the different parts of the sync system
	SyncContextPtr					m_syncContext;

	// Queue up all the remote operations that have been applied locally so that we can notify the 
	// listeners after they have all been applied
	std::queue<OperationPtr>		m_pendingNotifications;

	// Object to notify before and after we apply remote sync changes
	SyncListener*					m_listener;

	// The ID to use for messages passed between remote devices
	MessageID						m_messageID;
};

DECLARE_PTR(SyncManagerImpl)

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
