//////////////////////////////////////////////////////////////////////////
// SessionServer.h
//
// Main class for the session server
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <Common.h>
#include "ServiceBase.h"
#include "XSessionImpl.h"
#include "SessionChangeCallback.h"
#include "Private/Utils/FileLogWriter.h"
#include "BroadcastForwarder.h"
#include "PortPool.h"

XTOOLS_NAMESPACE_BEGIN

class SessionServer : public ServiceBase, public IncomingXSocketListener, public SessionChangeCallback, public NetworkConnectionListener
{
public:
	SessionServer(PWSTR pszServiceName);

	// ServiceBase Functions:

	// When implemented in a derived class, executes when a Start command is 
	// sent to the service by the SCM or when the operating system starts 
	// (for a service that starts automatically). Specifies actions to take 
	// when the service starts.
	virtual void OnStart(DWORD dwArgc, PWSTR *pszArgv) XTOVERRIDE;

	// When implemented in a derived class, executes when a Stop command is 
	// sent to the service by the SCM. Specifies actions to take when a 
	// service stops running.
	virtual void OnStop() XTOVERRIDE;

	virtual void OnDisconnected(const NetworkConnectionPtr& connection) XTOVERRIDE;
	virtual void OnMessageReceived(const NetworkConnectionPtr& connection, NetworkInMessage& message) XTOVERRIDE;

private:
	// SessionChangeCallback Functions:
    virtual void OnUserJoinedSession(uint32 sessionID, const std::string& userName, UserID userID, bool muteState) XTOVERRIDE;
	virtual void OnUserLeftSession(uint32 sessionID, UserID userID) XTOVERRIDE;
	virtual void OnSessionEmpty(const XSessionConstPtr& session) XTOVERRIDE;
    virtual void OnUserChanged(uint32 sessionID, const std::string& userName, UserID userId, bool muteState) XTOVERRIDE;

	// IncomingConnectionListener Functions:
	virtual void OnNewConnection(const XSocketPtr& newConnection) XTOVERRIDE;

    void OnHandshakeComplete(const XSocketPtr& newConnection, SocketID socketID, HandshakeResult result);

    virtual XSessionImplPtr CreateNewSession(const std::string& sessionName, SessionType type);

	virtual uint32 GetNewSessionId();

	// Callbacks for Json message types
	void OnNewSessionRequest(const NewSessionRequest& request, const NetworkConnectionPtr& connection);
	void OnListSessionsRequest(const ListSessionsRequest& request, const NetworkConnectionPtr& connection);

	void SendSessionMessageToAllClients(const std::string& message);

	// The entry point for the main thread of the session server
	void ServerThreadFunc();

    // Private Logging Helpers
    std::string GetCurrentDateTimeString(const tm& tm) const;
    std::string GetLogFileName(const tm& tm) const;
    void InitializeFileLogger();
    void TeardownFileLogger();


	struct Client
	{
		NetworkConnectionImplPtr	m_connection;
		ReceiptPtr					m_receipt;
	};

	NetworkMessagePoolPtr					m_messagePool;

	XSocketManagerPtr						m_socketMgr;
	SessionMessageRouter					m_messageRouter;
	ProfileManagerPtr						m_profileMgr;

#if defined(MSTEST)
	BroadcastForwarderPtr					m_broadcaster;
#endif

    ReceiptPtr                              m_listenerReceipt;
    ReceiptPtr                              m_netConnectionCallbackReceipt;
	std::vector<XSessionImplPtr>            m_sessions;
	std::map<XSessionImplPtr, ReceiptPtr>   m_sessionChangeListener;
	std::vector<Client>						m_clients;

	MemberFuncThreadPtr						m_serverThread;
	volatile int							m_stopping;

	PortPoolPtr								m_portPool;

	uint32									m_nextSessionId;

	Mutex									m_mutex;

	std::vector<XSessionImplPtr>			m_sessionsPendingDeletion;

	// list of the ongoing handshakes for remote clients that want to connect
	std::map<SocketID, NetworkHandshakePtr>		m_pendingConnections;

	LoggerPtr									m_logger;
    FileLogWriter*                              m_logWriter;

    static const std::string s_defaultLogBaseLocation;
};

XTOOLS_NAMESPACE_END