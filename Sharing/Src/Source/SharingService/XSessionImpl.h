//////////////////////////////////////////////////////////////////////////
// XSession.h
//
// The XTools server's representation of an active session
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "BroadcastForwarder.h"
#include "SendToForwarder.h"
#include "SessionChangeCallback.h"
#include "AudioSessionProcessorServer.h"
#include "ServerRoomManager.h"
#include <Private/NetworkConnectionImpl.h>

XTOOLS_NAMESPACE_BEGIN

class XSessionImpl : public XSession, public NetworkConnectionListener, public IncomingXSocketListener
{
public:
	explicit XSessionImpl(const std::string& name, uint16 port, SessionType type, unsigned int id);
	virtual ~XSessionImpl();

	// Register to receive callbacks when the session changes.  
	virtual ReceiptPtr RegisterCallback(SessionChangeCallback* cb) XTOVERRIDE;

	virtual uint32 GetId() const XTOVERRIDE;

	virtual std::string GetName() const XTOVERRIDE;

	virtual SessionType GetType() const XTOVERRIDE;

	virtual int32		GetUserCount() const XTOVERRIDE;
	virtual std::string GetSessionUserName(int32 i) const XTOVERRIDE;
	virtual uint32		GetSessionUserID(int32 i) const XTOVERRIDE;
    virtual bool        GetUserMuteState(int32 i) const XTOVERRIDE;

    uint16 GetPort() const;

	SessionDescriptorImplPtr GetSessionDescription(const XSocketPtr& targetRemoteSystem) const;

	// Returns false if the session was unable to initialize itself correctly
	bool IsInitialized() const;

private:
	// NetworkConnectionListener Functions:
	virtual void OnDisconnected(const NetworkConnectionPtr& connection) XTOVERRIDE;
	virtual void OnMessageReceived(const NetworkConnectionPtr& connection, NetworkInMessage& message) XTOVERRIDE;

	// Callback for when a new connection has been received from a remote machine on
	// the port that this listener registered for.  A ref-counted Connection is provided;
	// if a reference to this connection is not retained when this function returns then the
	// connection is closed.  
	virtual void OnNewConnection(const XSocketPtr& newConnection) XTOVERRIDE;

    void OnHandshakeComplete(const XSocketPtr& newConnection, SocketID socketID, HandshakeResult result);

	void AddConnection(const XSocketPtr& connection);

	// The entry point for the main thread of this session
	void ServerThreadFunc();

	// Stop receiving callbacks from this message type.
	void UnregisterCallback(byte message);

	void OnJoinSessionRequest(const JoinSessionRequest& request, const NetworkConnectionPtr& connection);
	void OnUserChanged(const UserChangedSessionMsg& request, const NetworkConnectionPtr& connection);


	struct RemoteClient;
	DECLARE_PTR(RemoteClient)

	RemoteClientPtr GetPendingClientForConnection(const NetworkConnectionPtr& connection);
	RemoteClientPtr GetExistingClientForConnection(const NetworkConnectionPtr& connection);

	void CheckIfEmpty(bool resetImmediately);

	NetworkMessagePoolPtr			m_messagePool;
	BroadcastForwarderPtr			m_broadcaster;
	SendToForwarderPtr				m_sendToForwarder;
	AudioSessionProcessorPtr		m_audioSessionProcessor;
	std::vector<RemoteClientPtr>	m_clients;
	std::vector<RemoteClientPtr>	m_pendingClients;

	// list of the ongoing handshakes for remote clients that want to connect
	std::map<SocketID, NetworkHandshakePtr>		m_pendingConnections;

	SessionMessageRouter			m_messageRouter;

    // TODO: Define a session member.
	std::string						m_name;
	uint32							m_id;
	SessionType						m_TypeOfSession;
	ReceiptPtr						m_listenerReceipt;
	XSocketManagerPtr				m_socketMgr;
	Mutex							m_mutex;
	SessionChangeCallback*			m_callback;
	uint16							m_port;
	Sync::SyncManagerPtr			m_syncMgr;
	Sync::SyncManagerPtr			m_internalSyncMgr;
	ServerRoomManagerPtr			m_roomMgr;

	MemberFuncThreadPtr				m_serverThread;

	std::chrono::high_resolution_clock::time_point	m_lastEmptyCheckTime;
	uint64							m_emptyTime;
	bool							m_bEmptyCheckApplied;

	volatile int					m_stopping;
};

DECLARE_PTR(XSessionImpl)

XTOOLS_NAMESPACE_END
