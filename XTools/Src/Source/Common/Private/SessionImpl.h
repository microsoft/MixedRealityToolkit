//////////////////////////////////////////////////////////////////////////
// SessionImpl.h
//
// Implementation of the Session interface
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "TunnelBridge.h"

XTOOLS_NAMESPACE_BEGIN

class SessionManagerImpl;

class SessionImpl : public Session, public NetworkConnectionListener
{
public:
	SessionImpl(SessionManagerImpl* mgr, const ClientContextConstPtr& context, uint32 sessionID, SessionType sessionType, const std::string& name, const std::string& address, uint16 port);

    virtual MachineSessionState GetMachineSessionState() const XTOVERRIDE;

	virtual void AddListener(SessionListener* newListener) XTOVERRIDE;
	virtual void RemoveListener(SessionListener* oldListener) XTOVERRIDE;

	// Returns true if we have joined this session
	virtual bool IsJoined() const XTOVERRIDE;

    // Initiate a join process.  This is async; the SessionListener is notified when the join succeeds or fails.
    // Returns false if this machine is already part of this session, or a join is already in progress.
    virtual bool Join() XTOVERRIDE;

    // Cause the machine to leave the session.  Async?  
    virtual void Leave() XTOVERRIDE;

    // Returns the number of users currently in the session
    virtual int32 GetUserCount() const XTOVERRIDE;

    // Returns the user at the given index
    virtual UserPtr GetUser(int32 i) XTOVERRIDE;

	virtual SessionType GetSessionType() const XTOVERRIDE;

	virtual const XStringPtr& GetName() const XTOVERRIDE;

    virtual NetworkConnectionPtr GetSessionNetworkConnection() const XTOVERRIDE;


    unsigned int GetSessionId() const;
    void SetSessionId(unsigned int id);

	void SetState(MachineSessionState newState);

	void AddUser(const UserPtr& newUser);
	UserPtr RemoveUser(UserID userID);

    void UpdateUser(const UserPtr& existingUser);

private: 

    UserPtr FindUser(UserID userID) const;

	// NetworkConnectionListener Functions:
	virtual void OnConnected(const NetworkConnectionPtr& connection) XTOVERRIDE;
	virtual void OnConnectFailed(const NetworkConnectionPtr& connection) XTOVERRIDE;
	virtual void OnDisconnected(const NetworkConnectionPtr& connection) XTOVERRIDE;
	virtual void OnMessageReceived(const NetworkConnectionPtr& connection, NetworkInMessage& message) XTOVERRIDE;

	void OnJoinSessionReply(const JoinSessionReply& reply, const NetworkConnectionPtr& connection);

    void OnHandshakeComplete(const XSocketPtr& newConnection, SocketID socketID, HandshakeResult result);

	void TeardownConnection();

	SessionManagerImpl*				m_sessionMgr;
	ClientContextConstPtr			m_context;

    MachineSessionState				m_curState;

	SessionMessageRouter			m_messageRouter;

	typedef ListenerList<SessionListener> ListenerList;
	DECLARE_PTR(ListenerList);

	ListenerListPtr					m_listenerList;
	std::vector<UserPtr>			m_users;

    NetworkConnectionPtr			m_sessionConnection;
    TunnelBridgePtr					m_tunnelBridge;
	NetworkHandshakePtr				m_handshake;

	uint32							m_id;
	SessionType						m_sessionType;
	XStringPtr						m_name;
	std::string						m_address;
	uint16							m_port;
};

DECLARE_PTR(SessionImpl)

XTOOLS_NAMESPACE_END