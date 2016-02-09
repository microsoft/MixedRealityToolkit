//////////////////////////////////////////////////////////////////////////
// UserPresenceManagerFollowerImpl.cpp
//
// Implementation of the Session List
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "UserPresenceManagerFollowerImpl.h"

XTOOLS_NAMESPACE_BEGIN

UserPresenceManagerFollowerImpl::UserPresenceManagerFollowerImpl(const NetworkConnectionPtr& desktopConnection) 
: m_desktopConnection(desktopConnection)
, m_localUser(NULL)
, m_listenerList(ListenerList::Create())
, m_lastRequestedMuteState(false)
, m_lastRequestedName(new XString(""))
{
	m_desktopConnection->AddListener(MessageID::UserPresenceChange, this);
}


void UserPresenceManagerFollowerImpl::AddListener(UserPresenceManagerListener* newListener)
{
	m_listenerList->AddListener(newListener);
}


void UserPresenceManagerFollowerImpl::RemoveListener(UserPresenceManagerListener* oldListener)
{
	m_listenerList->RemoveListener(oldListener);
}


bool UserPresenceManagerFollowerImpl::GetMuteState() const
{

    return m_localUser ? m_localUser->GetMuteState() : false;
}


void UserPresenceManagerFollowerImpl::SetMuteState(bool muteState)
{
    m_lastRequestedMuteState = muteState;
    RequestPresenceUpdate();
}


XStringPtr UserPresenceManagerFollowerImpl::GetName() const
{
    return m_localUser ? m_localUser->GetName() : new XString("");
}


void UserPresenceManagerFollowerImpl::SetName(const XStringPtr& name)
{
	if (!name)
	{
		LogError("Null name passed to UserPresenceManager::SetName");
		return;
	}

    m_lastRequestedName = name;
    RequestPresenceUpdate();
}

void UserPresenceManagerFollowerImpl::SetUser(const UserPtr& localUser)
{
    m_localUser = localUser;

    if (XTVERIFY(m_localUser))
    {
        m_lastRequestedMuteState = m_localUser->GetMuteState();
        m_lastRequestedName = m_localUser->GetName();
    }
    else
    {
        m_lastRequestedMuteState = false;
        m_lastRequestedName = new XString("");
    }
}

void UserPresenceManagerFollowerImpl::RequestPresenceUpdate()
{
    if (m_desktopConnection->IsConnected())
    {
        // Followers don't set mute directly. It requests a mute change to 
        // the desktop and waits for the subsequent round-trip update.
        NetworkOutMessagePtr message = m_desktopConnection->CreateMessage(MessageID::UserPresenceChange);
        message->Write(m_lastRequestedName);
        message->Write(m_lastRequestedMuteState ? 1 : 0);
        m_desktopConnection->Send(message);
        
        // We will call UpdateListeners() once we get the confirmation in 
        // OnMessageReceived that the request was honored.
    }
    else
    {
        if (XTVERIFY(m_localUser))
        {
            // We're not connected to a desktop. Just update the local copy of the state machine.
            m_localUser->SetName(m_lastRequestedName);
            m_localUser->SetMuteState(m_lastRequestedMuteState);
        }

        UpdateListeners();
    }
}


void UserPresenceManagerFollowerImpl::OnMessageReceived(const NetworkConnectionPtr&, NetworkInMessage& message)
{
    // Messages to the MuteManagerLeader are authoritative.
    XStringPtr userName = message.ReadString();
    int32 muteValue = message.ReadInt32();

    // We should never get called until after the Handshake complete has set a user.
    if (XTVERIFY(m_localUser))
    {
        m_localUser->SetMuteState(muteValue > 0 ? true : false);
        m_localUser->SetName(userName);
    }

    UpdateListeners();
}


void UserPresenceManagerFollowerImpl::UpdateListeners() const
{
	m_listenerList->NotifyListeners(&UserPresenceManagerListener::OnUserPresenceChanged, m_localUser);
}

XTOOLS_NAMESPACE_END