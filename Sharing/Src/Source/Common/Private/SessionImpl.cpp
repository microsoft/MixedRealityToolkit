//////////////////////////////////////////////////////////////////////////
// SessionImpl.cpp
//
// 
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NetworkConnectionImpl.h"
#include "SessionImpl.h"
#include "SessionMessageRouter.h"
#include "SessionManagerImpl.h"

XTOOLS_NAMESPACE_BEGIN

SessionImpl::SessionImpl(SessionManagerImpl* mgr, const ClientContextConstPtr& context, uint32 sessionID, SessionType sessionType, const std::string& name, const std::string& address, uint16 port)
: m_sessionMgr(mgr)
, m_context(context)
, m_curState(MachineSessionState::DISCONNECTED)
, m_listenerList(ListenerList::Create())
, m_id(sessionID)
, m_sessionType(sessionType)
, m_name(new XString(name))
, m_address(address)
, m_port(port)
{
	m_sessionConnection = m_context->GetSessionConnection();

	m_messageRouter.RegisterHandler(new MessageHandlerProxyT<const JoinSessionReply&>(CreateCallback2(this, &SessionImpl::OnJoinSessionReply)));
}


MachineSessionState SessionImpl::GetMachineSessionState() const
{
    return m_curState;
}


void SessionImpl::AddListener(SessionListener* newListener)
{
	m_listenerList->AddListener(newListener);
}


void SessionImpl::RemoveListener(SessionListener* oldListener)
{
	m_listenerList->RemoveListener(oldListener);
}


bool SessionImpl::IsJoined() const
{
	return m_curState == MachineSessionState::JOINED;
}


bool SessionImpl::Join()
{
    // Cannot join a session while we're joined or currently attempting to join.
	if (MachineSessionState::DISCONNECTED == m_curState) 
	{
		LogInfo("Attempting to join session %s at %s:%i", m_name->GetString().c_str(), m_address.c_str(), m_port);

        // Verify that no other sessions are already in a joining state.
        int sessionCount = m_sessionMgr->GetSessionCount();
        for (int i = 0; i < sessionCount; i++)
        {
            SessionPtr curSession = m_sessionMgr->GetSession(i);

            // Fail if any sessions are in a pending state.
            // TODO: Instead, we should just auto leave all non-disconnected sessions.
            if (curSession->GetMachineSessionState() == MachineSessionState::JOINING)
            {
                LogError("SessionImpl::Join called while another session %s was in a joining state", curSession->GetName()->GetString().c_str());

				curSession->Leave();

				XTASSERT(curSession->GetMachineSessionState() == MachineSessionState::DISCONNECTED);

                return false;
            }
        }

        SessionPtr oldSession = m_sessionMgr->GetCurrentSession();

        if (oldSession)
        {
            LogInfo("Automatically leaving existing session %s", oldSession->GetName()->GetString().c_str());
            oldSession->Leave();

			XTASSERT(oldSession->GetMachineSessionState() == MachineSessionState::DISCONNECTED);
			XTASSERT(m_sessionMgr->GetCurrentSession() == NULL);
        }

		// Set out state to joining
		SetState(MachineSessionState::JOINING);

		// Set this as the current session for this machine
		m_sessionMgr->SetCurrentSession(this);

		// Attempting to join a new session; notify the listeners
		m_listenerList->NotifyListeners(&SessionListener::OnJoiningSession);

		// Attempt to open a connection to this session
		XSocketPtr socket = m_context->GetXSocketManager()->OpenConnection(m_address, m_port);
		if (XTVERIFY(socket))
		{
			// Initiate the handshake
			HandshakeCallback callback = CreateCallback3(this, &SessionImpl::OnHandshakeComplete);
			m_handshake = new NetworkHandshake(socket, new SessionHandshakeLogic(false), callback);

			return true;
		}
        else
        {
            LogError("Join failed we failed to create a socket to the session at %s:%u.", m_address.c_str(), m_port);
        }
	}
    else
    {
        LogError("Join failed because the current state is %s instead of DISCONNECTED", (m_curState == MachineSessionState::JOINING) ? "JOINING" : "JOINED");
    }
	
	return false;
}

void SessionImpl::Leave()
{
	LogInfo("Leaving session %s at %s:%i", m_name->GetString().c_str(), m_address.c_str(), m_port);

	SessionPtr currentSession = m_sessionMgr->GetCurrentSession();
	if (currentSession == this)
	{
		bool isConnected = m_sessionConnection->IsConnected();
		if (isConnected)
		{
			// Will cause OnDisconnected to be called
			m_sessionConnection->Disconnect();
		}

		TeardownConnection();

		XTASSERT(m_sessionMgr->GetCurrentSession() == NULL);
	}

	XTASSERT(GetMachineSessionState() == MachineSessionState::DISCONNECTED);
}


int32 SessionImpl::GetUserCount() const
{
	return static_cast<int32>(m_users.size());
}


UserPtr SessionImpl::GetUser(int32 i)
{
	if (i >= 0 && i < GetUserCount())
	{
		return m_users[i];
	}
	else
	{
		return NULL;
	}
}


SessionType SessionImpl::GetSessionType() const
{
	return m_sessionType;
}


const XStringPtr& SessionImpl::GetName() const
{
	return m_name;
}


unsigned int SessionImpl::GetSessionId() const
{
    return m_id;
}


void SessionImpl::SetSessionId(unsigned int id)
{
    m_id = id;
}


NetworkConnectionPtr SessionImpl::GetSessionNetworkConnection() const
{
	return m_sessionConnection;
}


void SessionImpl::SetState(MachineSessionState newState)
{
	m_curState = newState;
}


void SessionImpl::AddUser(const UserPtr& newUser)
{
    UserPtr existingUser = FindUser(newUser->GetID());
    if (!existingUser)
    {
        m_users.push_back(newUser);
    }
}


UserPtr SessionImpl::RemoveUser(UserID userID)
{
	for (size_t i = 0; i < m_users.size(); ++i)
	{
		if (m_users[i]->GetID() == userID)
		{
			UserPtr user = m_users[i];
			m_users.erase(m_users.begin() + i);
			
			return user;
		}
	}

	return NULL;
}


void SessionImpl::UpdateUser(const UserPtr& updatedUser)
{
    // Expect this only to be called on sessions we are joined to.
    XTASSERT(IsJoined());

    UserPtr user = FindUser(updatedUser->GetID());

    // Assert that we aren't updating for a user who's not present.
    if (XTVERIFY(user))
    {
        UserChangedSessionMsg userUpdateMsg(m_id, user->GetName()->GetString(), user->GetID(), user->GetMuteState());
        NetworkOutMessagePtr msg = m_sessionConnection->CreateMessage(MessageID::SessionControl);
        
        msg->Write(new XString(userUpdateMsg.ToJSONString()));
        m_sessionConnection->Send(msg);
    }
}


UserPtr SessionImpl::FindUser(UserID userID) const
{
    for (size_t i = 0; i < m_users.size(); ++i)
    {
        if (m_users[i]->GetID() == userID)
        {
            return m_users[i];
        }
    }

    return NULL;
}


void SessionImpl::OnConnected(const NetworkConnectionPtr& connection)
{
	// We have successfully connected to this session.  
	// Now send it information about us so it lets us in all the way

	XTASSERT(m_curState == MachineSessionState::JOINING);

	JoinSessionRequest request(
		m_context->GetLocalUser()->GetName()->GetString(),
		m_context->GetLocalUser()->GetID(),
        m_context->GetLocalUser()->GetMuteState()
		);

	NetworkOutMessagePtr msg = connection->CreateMessage(MessageID::SessionControl);

	msg->Write(new XString(request.ToJSONString()));

	connection->Send(msg);
}


void SessionImpl::OnConnectFailed(const NetworkConnectionPtr&)
{
	XTASSERT(m_curState == MachineSessionState::JOINING);

	TeardownConnection();
}


void SessionImpl::OnDisconnected(const NetworkConnectionPtr&)
{
	// Connection to the session has been lost; we are no longer joined

	TeardownConnection();
}


void SessionImpl::OnMessageReceived(const NetworkConnectionPtr& connection, NetworkInMessage& message)
{
	XStringPtr command = message.ReadString();

	JSONMessagePtr jMsg = JSONMessage::CreateFromMessage(command->GetString());

	// Route the incoming message to the appropriate function to handle it
	if (!m_messageRouter.CallHandler(jMsg, connection))
	{
		// We got a bad or unexpected message from the server; break the connection
		TeardownConnection();
	}
}


void SessionImpl::OnJoinSessionReply(const JoinSessionReply& reply, const NetworkConnectionPtr&)
{
	XTASSERT(m_curState == MachineSessionState::JOINING);

	if (reply.GetResult())
	{
		LogInfo("Session Join Succeeded");

		SetState(MachineSessionState::JOINED);

		// Add the connection to the session to the Sync manager
		m_context->GetSyncManager()->AddConnection(m_sessionConnection);
		m_context->GetInternalSyncManager()->AddConnection(m_sessionConnection);

		// Connect the tunnel between the secondary client and the session
		m_tunnelBridge = new TunnelBridge(m_sessionConnection, m_context->GetPairedConnection());

        AddUser(m_context->GetLocalUser());

        m_sessionMgr->NotifyOnUserJoinedSessionListeners(this, m_context->GetLocalUser());

		// Tell all listeners that we've successfully created the session.
		m_listenerList->NotifyListeners(&SessionListener::OnJoinSucceeded);
	}
	else
	{
		LogInfo("Session Join Failed!");

		Leave();
	}
}


void SessionImpl::OnHandshakeComplete(const XSocketPtr& newConnection, SocketID, HandshakeResult result)
{
	m_handshake = NULL;

	if (newConnection && result == HandshakeResult::Success)
	{
		// Register to listen for SessionControl messages from the session
		m_sessionConnection->AddListener(MessageID::SessionControl, this);

		m_sessionConnection->SetSocket(newConnection);
	}
	else
	{
        LogInfo("Session: Handshake for session %s failed with error %u", m_name->GetString().c_str(), result);
		XTASSERT(m_curState == MachineSessionState::JOINING);

		Leave();
	}
}


void SessionImpl::TeardownConnection()
{
	MachineSessionState oldState = m_curState;

	SetState(MachineSessionState::DISCONNECTED);

	ref_ptr<SessionImpl> thisPtr(this);

	m_handshake = nullptr;
	m_tunnelBridge = nullptr;
	m_sessionConnection->RemoveListener(MessageID::SessionControl, this);
	m_context->GetSyncManager()->RemoveConnection(m_sessionConnection);
	m_context->GetInternalSyncManager()->RemoveConnection(m_sessionConnection);
	m_sessionMgr->SetCurrentSession(NULL);

	// Notify the listeners
	if (oldState == MachineSessionState::JOINED)
	{
		m_listenerList->NotifyListeners(&SessionListener::OnSessionDisconnected);
	}
	else if (oldState == MachineSessionState::JOINING)
	{
		m_listenerList->NotifyListeners(&SessionListener::OnJoinFailed);
	}
}


XTOOLS_NAMESPACE_END
