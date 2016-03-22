//////////////////////////////////////////////////////////////////////////
// UserPresenceManagerLeaderImpl.h
//
// Authority implementation of the UserPresenceManager.  
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class UserPresenceManagerLeaderImpl : public UserPresenceManager, NetworkConnectionListener
{
public:

    UserPresenceManagerLeaderImpl(const NetworkConnectionPtr& barabooConnection, const UserPtr localUser);

	virtual void AddListener(UserPresenceManagerListener* newListener) XTOVERRIDE;

	virtual void RemoveListener(UserPresenceManagerListener* oldListener) XTOVERRIDE;

    virtual bool GetMuteState() const XTOVERRIDE;

    virtual void SetMuteState(bool muteState) XTOVERRIDE;

    virtual void SetName(const XStringPtr& name) XTOVERRIDE;

    virtual XStringPtr GetName() const XTOVERRIDE;

    virtual void SetUser(const UserPtr& localUser) XTOVERRIDE;

private:

    // NetworkConnectionListener
	virtual void OnMessageReceived(const NetworkConnectionPtr& connection, NetworkInMessage& message) XTOVERRIDE;

    void UpdateListeners() const;

    void PushPresenceUpdate();

	typedef ListenerList<UserPresenceManagerListener> ListenerList;
	DECLARE_PTR(ListenerList);

    UserPtr						m_localUser;
	ListenerListPtr				m_listenerList;
    NetworkConnectionPtr        m_barabooConnection;
};

DECLARE_PTR(UserPresenceManagerLeaderImpl)

XTOOLS_NAMESPACE_END