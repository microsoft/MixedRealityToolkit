//////////////////////////////////////////////////////////////////////////
// MuteManagerFollowerImpl.h
//
// Authority implementation of the MuteManager.  
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN


class UserPresenceManagerFollowerImpl : public UserPresenceManager, NetworkConnectionListener
{
public:

    UserPresenceManagerFollowerImpl(const NetworkConnectionPtr& desktopConnection);

	virtual void AddListener(UserPresenceManagerListener* newListener) XTOVERRIDE;

	virtual void RemoveListener(UserPresenceManagerListener* oldListener) XTOVERRIDE;

    virtual bool GetMuteState() const XTOVERRIDE;

    virtual void SetMuteState(bool muteState) XTOVERRIDE;

    virtual XStringPtr GetName() const XTOVERRIDE;

    virtual void SetName(const XStringPtr& name) XTOVERRIDE;

    virtual void SetUser(const UserPtr& localUser) XTOVERRIDE;

private: 

    virtual void UpdateListeners() const;
	virtual void OnMessageReceived(const NetworkConnectionPtr& connection, NetworkInMessage& message) XTOVERRIDE;
    virtual void RequestPresenceUpdate();

	typedef ListenerList<UserPresenceManagerListener> ListenerList;
	DECLARE_PTR(ListenerList);

    UserPtr                                     m_localUser;
	ListenerListPtr								m_listenerList;
    NetworkConnectionPtr                        m_desktopConnection;

    bool                                        m_lastRequestedMuteState;
    XStringPtr                                  m_lastRequestedName;

};

DECLARE_PTR(UserPresenceManagerFollowerImpl)

XTOOLS_NAMESPACE_END