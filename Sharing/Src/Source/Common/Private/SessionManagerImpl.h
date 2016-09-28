//////////////////////////////////////////////////////////////////////////
// SessionManagerImpl.h
//
// Implementation of SessionManager
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <Private/SessionImpl.h>

XTOOLS_NAMESPACE_BEGIN

class SessionManagerImpl : public SessionManager, public UserPresenceManagerListener, public NetworkConnectionListener
{
public:
    explicit SessionManagerImpl(const ClientContextConstPtr& context, const UserPresenceManagerPtr& userPresenceMgr);

	// SessionManager Functions:
	virtual void			AddListener(SessionManagerListener* newListener) XTOVERRIDE;
	virtual void			RemoveListener(SessionManagerListener* oldListener) XTOVERRIDE;
	virtual bool			CreateSession(const XStringPtr& sessionName, SessionType type) XTOVERRIDE;
	virtual int32			GetSessionCount() const XTOVERRIDE;
	virtual SessionPtr		GetSession(int32 index) XTOVERRIDE;
	virtual SessionPtr		GetCurrentSession() XTOVERRIDE;
    virtual const UserPtr&	GetCurrentUser() const XTOVERRIDE;
    virtual void			UpdateCurrentUserOnServer() XTOVERRIDE;
	virtual bool			IsServerConnected() const XTOVERRIDE;

	/// Local Functions:

	// Called from SessionImpl when it successfully joins a session or leaves a session
	void					SetCurrentSession(const SessionImplPtr& session);

    // HACK: Allow local sessions to tell the session manager to notify all listeners for OnUserJoinedSession.
    void					NotifyOnUserJoinedSessionListeners(SessionPtr session, UserPtr user) const;
private:
	// Network Callback Functions:
	virtual void			OnConnected(const NetworkConnectionPtr& connection) XTOVERRIDE;
	virtual void			OnDisconnected(const NetworkConnectionPtr& connection) XTOVERRIDE;
	virtual void			OnMessageReceived(const NetworkConnectionPtr& connection, NetworkInMessage& message) XTOVERRIDE;

	// UserPresenceManagerListener Functions:
	void					OnUserPresenceChanged(const UserPtr& user) XTOVERRIDE;

	// Local Functions:
	void					OnNewSessionReply(const NewSessionReply& reply, const NetworkConnectionPtr& connection);
	void					OnUserJoinedSession(const UserJoinedSessionMsg& msg, const NetworkConnectionPtr& connection);
	void					OnUserLeftSession(const UserLeftSessionMsg& msg, const NetworkConnectionPtr& connection);
	void					OnListSessionsReply(const ListSessionsReply& msg, const NetworkConnectionPtr& connection);
	void					OnSessionAdded(const SessionAddedMsg& msg, const NetworkConnectionPtr& connection);
	void					OnSessionClosed(const SessionClosedMsg& msg, const NetworkConnectionPtr& connection);
	void					OnUserChanged(const UserChangedSessionMsg& msg, const NetworkConnectionPtr& connection);

    SessionImplPtr			GetLocalSessionById(unsigned int id) const;

	ClientContextConstPtr					m_context;
    UserPresenceManagerPtr                  m_userPresenceMgr;
	SessionMessageRouter					m_messageRouter;

	typedef ListenerList<SessionManagerListener> ListenerList;
	DECLARE_PTR(ListenerList);

	ListenerListPtr							m_listenerList;

	std::vector<SessionImplPtr>				m_sessions;

	SessionImplPtr							m_currentSession;

	bool									m_bNewSessionRequestPending;
};

DECLARE_PTR(SessionManagerImpl)

XTOOLS_NAMESPACE_END
