//////////////////////////////////////////////////////////////////////////
// UserPresenceManagerLeaderImpl.cpp
//
// Implementation of the Session List
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UserPresenceManagerLeaderImpl.h"

XTOOLS_NAMESPACE_BEGIN

UserPresenceManagerLeaderImpl::UserPresenceManagerLeaderImpl(const NetworkConnectionPtr& barabooConnection, const UserPtr localUser) 
: m_barabooConnection(barabooConnection)
, m_listenerList(ListenerList::Create())
{
    SetUser(localUser);
	m_barabooConnection->AddListener(MessageID::UserPresenceChange, this);
}


void UserPresenceManagerLeaderImpl::AddListener(UserPresenceManagerListener* newListener)
{
	m_listenerList->AddListener(newListener);
}


void UserPresenceManagerLeaderImpl::RemoveListener(UserPresenceManagerListener* oldListener)
{
	m_listenerList->RemoveListener(oldListener);
}


bool UserPresenceManagerLeaderImpl::GetMuteState() const
{
    return m_localUser->GetMuteState();
}


void UserPresenceManagerLeaderImpl::SetName(const XStringPtr& name)
{
	if (!name)
	{
		LogError("Null name passed to UserPresenceManagerLeaderImpl::SetName");
		return;
	}

    m_localUser->SetName(name);
    PushPresenceUpdate();
}


XStringPtr UserPresenceManagerLeaderImpl::GetName() const
{
    return m_localUser->GetName();
}


void UserPresenceManagerLeaderImpl::SetMuteState(bool muteState)
{
    // We should never be calling SetMuteState before we've set the local user.
    if (XTVERIFY(m_localUser))
    {
        m_localUser->SetMuteState(muteState);
    }

    PushPresenceUpdate();
}

void UserPresenceManagerLeaderImpl::SetUser(const UserPtr& localUser)
{
    XTASSERT(localUser);
    m_localUser = localUser;
}

void UserPresenceManagerLeaderImpl::PushPresenceUpdate()
{
    // Tell any connected baraboo about the new mute state.
    if (m_barabooConnection->IsConnected())
    {
        NetworkOutMessagePtr message = m_barabooConnection->CreateMessage(MessageID::UserPresenceChange);
        message->Write(m_localUser ? m_localUser->GetName() : NULL);
        message->Write(int32((m_localUser && m_localUser->GetMuteState()) ? 1 : 0));
        m_barabooConnection->Send(message);
    }

    UpdateListeners();
}


void UserPresenceManagerLeaderImpl::OnMessageReceived(const NetworkConnectionPtr&, NetworkInMessage& message)
{
    // Messages to the MuteManagerLeader are considered requests.
    XStringPtr nameValue = message.ReadString();
    int32 muteValue = message.ReadInt32();

    // Set the internal state and tell the baraboo about the new state.
    SetName(nameValue);
    SetMuteState(muteValue > 0 ? true : false);
}


void UserPresenceManagerLeaderImpl::UpdateListeners() const
{
    XTASSERT(m_localUser);

	m_listenerList->NotifyListeners(&UserPresenceManagerListener::OnUserPresenceChanged, m_localUser);
}


XTOOLS_NAMESPACE_END