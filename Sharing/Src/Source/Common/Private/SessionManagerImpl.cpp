//////////////////////////////////////////////////////////////////////////
// SessionManagerImpl.cpp
//
// Implementation of SessionManager
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SessionManagerImpl.h"
#include <Private/UserImpl.h>

XTOOLS_NAMESPACE_BEGIN

SessionManagerImpl::SessionManagerImpl(const ClientContextConstPtr& context, const UserPresenceManagerPtr& userPresenceMgr)
: m_context(context)
, m_userPresenceMgr(userPresenceMgr)
, m_listenerList(ListenerList::Create())
, m_bNewSessionRequestPending(false)
{
	m_context->GetSessionListConnection()->AddListener(MessageID::SessionControl, this);
	m_userPresenceMgr->AddListener(this);

	m_messageRouter.RegisterHandler(new MessageHandlerProxyT<const NewSessionReply&>(CreateCallback2(this, &SessionManagerImpl::OnNewSessionReply)));
	m_messageRouter.RegisterHandler(new MessageHandlerProxyT<const UserJoinedSessionMsg&>(CreateCallback2(this, &SessionManagerImpl::OnUserJoinedSession)));
	m_messageRouter.RegisterHandler(new MessageHandlerProxyT<const UserLeftSessionMsg&>(CreateCallback2(this, &SessionManagerImpl::OnUserLeftSession)));
	m_messageRouter.RegisterHandler(new MessageHandlerProxyT<const ListSessionsReply&>(CreateCallback2(this, &SessionManagerImpl::OnListSessionsReply)));
	m_messageRouter.RegisterHandler(new MessageHandlerProxyT<const SessionAddedMsg&>(CreateCallback2(this, &SessionManagerImpl::OnSessionAdded)));
	m_messageRouter.RegisterHandler(new MessageHandlerProxyT<const SessionClosedMsg&>(CreateCallback2(this, &SessionManagerImpl::OnSessionClosed)));
	m_messageRouter.RegisterHandler(new MessageHandlerProxyT<const UserChangedSessionMsg&>(CreateCallback2(this, &SessionManagerImpl::OnUserChanged)));
}


void SessionManagerImpl::AddListener(SessionManagerListener* newListener)
{
	m_listenerList->AddListener(newListener);
}


void SessionManagerImpl::RemoveListener(SessionManagerListener* oldListener)
{
	m_listenerList->RemoveListener(oldListener);
}


bool SessionManagerImpl::CreateSession(const XStringPtr& sessionName, SessionType type)
{
	if (!sessionName)
	{
		LogWarning("Null session name passed to CreateSettion");
		return false;
	}

	// Can't create a session if we aren't connected to the server
	if (!m_context->GetSessionListConnection()->IsConnected())
	{
		LogWarning("Cannot create a new session: not connected to session list server");
		return false;
	}

	// Can't have more then one request in flight at a time
	if (m_bNewSessionRequestPending)
	{
		LogWarning("Cannot create a new session: a new session request is already pending");
		return false;
	}

	// Cannot create a session with a name that is too short
	if (sessionName->GetLength() < kMinSessionNameLength)
	{
		LogWarning("Cannot create a new session: name must have at least %i letters", kMinSessionNameLength);
		return false;
	}

	// Cannot create a session with a name that is too long
	if (sessionName->GetLength() > kMaxSessionNameLength)
	{
		LogWarning("Cannot create a new session: name cannot be more than %i letters", kMaxSessionNameLength);
		return false;
	}

	m_bNewSessionRequestPending = true;

	size_t sessionCount = m_sessions.size();
	for (size_t i = 0; i < sessionCount; i++)
	{
		SessionPtr curSession = m_sessions[i];
		if (m_sessions[i]->GetMachineSessionState() != MachineSessionState::DISCONNECTED)
		{
			LogInfo("Leaving session %s before creating a new session.", curSession->GetName()->GetString().c_str());
			curSession->Leave();
			XTASSERT(curSession->GetMachineSessionState() == MachineSessionState::DISCONNECTED);
		}
	}

	// Create the request data
	NewSessionRequest request;
	request.SetSessionName(sessionName->GetString());
	request.SetSessionType(type);

	// Send the request to the session list server
	NetworkConnectionPtr serverConnection = m_context->GetSessionListConnection();
	NetworkOutMessagePtr message = serverConnection->CreateMessage(MessageID::SessionControl);

	message->Write(new XString(request.ToJSONString()));

	serverConnection->Send(message);

	return true;
}


int32 SessionManagerImpl::GetSessionCount() const
{
	return static_cast<int32>(m_sessions.size());
}


SessionPtr SessionManagerImpl::GetSession(int32 index)
{
	if (index >= 0 && index < GetSessionCount())
	{
		return m_sessions[index];
	}
	else
	{
		return NULL;
	}
}


SessionPtr SessionManagerImpl::GetCurrentSession()
{
	return m_currentSession;
}


const UserPtr&  SessionManagerImpl::GetCurrentUser() const
{
	return m_context->GetLocalUser();
}


void SessionManagerImpl::SetCurrentSession(const SessionImplPtr& session)
{
	m_currentSession = session;
}


void SessionManagerImpl::UpdateCurrentUserOnServer()
{
	UserPtr user = GetCurrentUser();
	if (m_currentSession && user)
	{
		m_currentSession->UpdateUser(user);
	}
}


// Returns true if we currently have a connection to the Session List Server
bool SessionManagerImpl::IsServerConnected() const
{
	if (m_context &&
		m_context->GetSessionListConnection() &&
		m_context->GetSessionListConnection()->IsConnected())
	{
		return true;
	}

	return false;
}

void SessionManagerImpl::OnConnected(const NetworkConnectionPtr& connection)
{
	// Request full list of sessions and who is in them
	ListSessionsRequest request;

	NetworkOutMessagePtr msg = connection->CreateMessage(MessageID::SessionControl);
	msg->Write(new XString(request.ToJSONString()));
	connection->Send(msg);

	// Notify the listeners that we have established a connection
	m_listenerList->NotifyListeners(&SessionManagerListener::OnServerConnected);
}


void SessionManagerImpl::OnDisconnected(const NetworkConnectionPtr&)
{
	// Clear our list of sessions, except for the one we are in.
	m_sessions.clear();
	
	// Notify the listeners that we have lost the connection
	m_listenerList->NotifyListeners(&SessionManagerListener::OnServerDisconnected);

	m_bNewSessionRequestPending = false;
}


void SessionManagerImpl::OnMessageReceived(const NetworkConnectionPtr& connection, NetworkInMessage& message)
{
	XStringPtr command = message.ReadString();

	JSONMessagePtr jMsg = JSONMessage::CreateFromMessage(command->GetString());

	// Route the incoming message to the appropriate function to handle it
	if (!m_messageRouter.CallHandler(jMsg, connection))
	{
		// We got a bad or unexpected message from the server; break the connection
		connection->Disconnect();
	}
}


void SessionManagerImpl::OnNewSessionReply(const NewSessionReply& reply, const NetworkConnectionPtr&)
{
	XTASSERT(m_bNewSessionRequestPending);

	m_bNewSessionRequestPending = false;

	// If the session was successfully created
	if (reply.GetResult())
	{
		const std::string address = reply.GetAddress();
		uint16 port = reply.GetPort();

		LogInfo("New Session created at %s|%u", address.c_str(), port);

		uint32 sessionId = reply.GetSessionID();
		SessionImplPtr newSession = GetLocalSessionById(sessionId);

		if (!newSession)
		{
			// Create the local representation of this session
			newSession = new SessionImpl(this, m_context, sessionId, reply.GetSessionType(), reply.GetSessionName(), address, port);
			m_sessions.push_back(newSession);
		}

		// Automatically initiate a join
		bool joinResult = newSession->Join();
		
		// HACK: TODO Ultimately, we must ensure that joins never fail after creates.
		if (!joinResult)
		{
			LogError("Failed to join newly created session %s", newSession->GetName()->GetString().c_str());
		}

		// Notify the listeners that the request succeeded
		m_listenerList->NotifyListeners(&SessionManagerListener::OnCreateSucceeded, newSession);
	}
	else
	{
		LogError("Session Creation Failed!");
	
		// Notify the listeners that the request failed
		XStringPtr failureReason = new XString(reply.GetFailureReason());
		m_listenerList->NotifyListeners(&SessionManagerListener::OnCreateFailed, failureReason);
	}
}


void SessionManagerImpl::OnUserJoinedSession(const UserJoinedSessionMsg& msg, const NetworkConnectionPtr&)
{
	uint32 sessionID = msg.GetSessionID();
	UserID userId = msg.GetSessionUserID();

	// If this session has the current user joined, use that object instead. 
	if (userId == m_context->GetLocalUser()->GetID())
	{
		// Just exit the function as if everything is fine.
		return;
	}

	SessionImplPtr session = GetLocalSessionById(sessionID);

	// NOTE: Its possible for this message to arrive while still waiting for the initial list of sessions.
	// This should be fixed for realz by putting the list of sessions in the handshake
	if (session)
	{
		UserPtr newUser = new UserImpl(msg.GetSessionUserName(), userId, msg.GetSessionUserMuteState());
		session->AddUser(newUser);

		NotifyOnUserJoinedSessionListeners(session, newUser);
	}
	else
	{
		LogError("OnUserJoinedSession: failed to find session %u for user %i", msg.GetSessionID(), msg.GetSessionUserID());
	}
}


void SessionManagerImpl::OnUserLeftSession(const UserLeftSessionMsg& msg, const NetworkConnectionPtr&)
{
	uint32 sessionID = msg.GetSessionID();
	UserID userID = msg.GetUserID();

	// Find the session that the user us leaving
	
	SessionImplPtr session = GetLocalSessionById(sessionID);

	// NOTE: Its possible for this message to arrive while still waiting for the initial list of sessions.
	// This should be fixed for realz by putting the list of sessions in the handshake
	if (session)
	{
		UserPtr user = session->RemoveUser(userID);

		if (user)
		{
			// Notify the listeners that the user left the session
			m_listenerList->NotifyListeners(&SessionManagerListener::OnUserLeftSession, session, user);
		}
		else
		{
			LogError("OnUserLeftSession: failed to find user %u for session %i", msg.GetUserID(), msg.GetSessionID());
		}
	}
	else
	{
		LogError("OnUserLeftSession: failed to find session %u for user %u", msg.GetSessionID(), msg.GetUserID());
	}
}


void SessionManagerImpl::OnListSessionsReply(const ListSessionsReply& msg, const NetworkConnectionPtr&)
{
	// Loop through the list sessions that we've received
	int32 numDescriptions = msg.GetSessionCount();
	int32 numStartingSessions = (int32)m_sessions.size();

	// List of each descriptor for the the session at the given index.
	// If there is no descriptor, then the session has been deleted and should be removed
	std::vector<SessionDescriptor*> descriptors;
	descriptors.resize(m_sessions.size(), NULL);

	// List of sessions that have not been added yet
	std::vector<SessionDescriptor*> newSessions;

	// Categorize all the incoming descriptors
	for (int32 descriptionIndex = 0; descriptionIndex < numDescriptions; ++descriptionIndex)
	{
		SessionDescriptor* desc = msg.GetSessionDescriptor(descriptionIndex);
		if (desc)
		{
			// Check to see if this session already exists
			bool sessionFound = false;
			for (size_t existingdescriptionIndex = 0; existingdescriptionIndex < m_sessions.size(); ++existingdescriptionIndex)
			{
				if (m_sessions[existingdescriptionIndex]->GetSessionId() == desc->GetID())
				{
					descriptors[existingdescriptionIndex] = desc;
					sessionFound = true;
					break;
				}
			}

			if (!sessionFound)
			{
				newSessions.push_back(desc);
			}
		}
	}


	// Pass 1: Update the users for the existing sessions
	for (size_t sessionIndex = 0; sessionIndex < descriptors.size(); ++sessionIndex)
	{
		if (descriptors[sessionIndex] != NULL)
		{
			SessionImplPtr currentSession = m_sessions[sessionIndex];
			SessionDescriptor* currentDesc = descriptors[sessionIndex];

			int32 startingNumUsers = currentSession->GetUserCount();

			UserID localUserId = m_context->GetLocalUser()->GetID();

			std::vector<bool> usersMatched;
			usersMatched.resize(startingNumUsers, false);

			for (int32 descUserIndex = 0; descUserIndex < currentDesc->GetUserCount(); ++descUserIndex)
			{
				User* descUser = currentDesc->GetUser(descUserIndex);
				UserID descId = descUser->GetID();

				XTASSERT(descId != User::kInvalidUserID);

				// Check to see if this user already exists
				bool userFound = false;
				for (int32 sessionUserIndex = 0; sessionUserIndex < startingNumUsers; ++sessionUserIndex)
				{
					if (descId == currentSession->GetUser(sessionUserIndex)->GetID())
					{
						usersMatched[sessionUserIndex] = true;
						userFound = true;
						break;
					}
				}

				if (!userFound)
				{
					User* newUser = NULL;
					if (localUserId == descId)
					{
						// This user is the local user. Use the existing local user object.
						newUser = m_context->GetLocalUser().get();
					}
					else
					{
						// This user is new: add them
						newUser = descUser;
					}

					currentSession->AddUser(newUser);

					// Notify the listeners
					m_listenerList->NotifyListeners(&SessionManagerListener::OnUserJoinedSession, currentSession, descUser);
				}
			}

			// Delete the users that were not matched
			for (int32 matchIndex = startingNumUsers-1; matchIndex >= 0; --matchIndex)
			{
				if (!usersMatched[matchIndex])
				{
					UserPtr leavingUser = currentSession->GetUser(matchIndex);
					currentSession->RemoveUser(leavingUser->GetID());

					// Notify the listeners
					m_listenerList->NotifyListeners(&SessionManagerListener::OnUserLeftSession, currentSession, leavingUser);
				}
			}
		}
	}

	// Pass 2: Remove the closed sessions
	for (int32 sessionIndex = numStartingSessions - 1; sessionIndex >= 0; --sessionIndex)
	{
		SessionImplPtr closingSession = m_sessions[sessionIndex];

		m_sessions.erase(m_sessions.begin() + sessionIndex);

		// Notify the listeners
		m_listenerList->NotifyListeners(&SessionManagerListener::OnSessionClosed, closingSession);
	}

	// Pass 3: Add the new sessions
	for (size_t i = 0; i < newSessions.size(); ++i)
	{
		SessionDescriptor* currentDesc = newSessions[i];

		// Create the local representation of this session
		SessionImplPtr newSession = new SessionImpl(
			this, 
			m_context, 
			currentDesc->GetID(), 
			currentDesc->GetSessionType(), 
			currentDesc->GetName(), 
			currentDesc->GetAddress(), 
			currentDesc->GetPortID());

		// We don't need to check if this session is already in the sessions list 
		// because we checked when populating the newSessions list. 
		m_sessions.push_back(newSession);

		// Add the list of users
		for (int32 userIndex = 0; userIndex < currentDesc->GetUserCount(); ++userIndex)
		{
			User* sessionUser = currentDesc->GetUser(userIndex);
			XTASSERT(sessionUser->GetID() != User::kInvalidUserID);
			newSession->AddUser(sessionUser);
		}

		// Notify the listeners that a new session has been added
		m_listenerList->NotifyListeners(&SessionManagerListener::OnSessionAdded, newSession);
	}
}


void SessionManagerImpl::OnSessionAdded(const SessionAddedMsg& msg, const NetworkConnectionPtr&)
{
	SessionDescriptor* currentDesc = msg.GetSessionDescriptor();

	uint32 sessionId = currentDesc->GetID();
	SessionImplPtr newSession = GetLocalSessionById(sessionId);

	if (!newSession)
	{
		// Create the local representation of this session
		newSession = new SessionImpl(
			this,
			m_context,
			sessionId,
			currentDesc->GetSessionType(),
			currentDesc->GetName(),
			currentDesc->GetAddress(),
			currentDesc->GetPortID());

		m_sessions.push_back(newSession);
	}

	// Add the list of users
	for (int32 userIndex = 0; userIndex < currentDesc->GetUserCount(); ++userIndex)
	{
		User* newUser = NULL;
		User* descUser = currentDesc->GetUser(userIndex);

		XTASSERT(descUser->GetID() != User::kInvalidUserID);

		// If the user we've found is the current user, use that object instead.
		if (m_context->GetLocalUser()->GetID() == descUser->GetID())
		{
			newUser = m_context->GetLocalUser().get();
		}
		else
		{
			newUser = descUser;
		}

		newSession->AddUser(newUser);
	}

	// Notify the listeners that a new session has been added
	m_listenerList->NotifyListeners(&SessionManagerListener::OnSessionAdded, newSession);
}


void SessionManagerImpl::OnSessionClosed(const SessionClosedMsg& msg, const NetworkConnectionPtr&)
{
	uint32 sessionID = msg.GetSessionID();
	for (size_t i = 0; i < m_sessions.size(); ++i)
	{
		if (m_sessions[i]->GetSessionId() == sessionID)
		{
			SessionImplPtr closedSession = m_sessions[i];
			
			// Clear out the current session in case this is it.  
			if (m_currentSession == closedSession)
			{
				m_currentSession = NULL;
			}

			// Force the session to be left and cleaned up in case it isn't already
			closedSession->Leave();

			// Leaving should cause the session to become disconnected.  Lets verify that:
			XTASSERT(closedSession->GetMachineSessionState() == MachineSessionState::DISCONNECTED);

			// Remove it from the list
			m_sessions.erase(m_sessions.begin() + i);

			// Notify listeners that the session has closed
			m_listenerList->NotifyListeners(&SessionManagerListener::OnSessionClosed, closedSession);

			break;
		}
	}
}


void SessionManagerImpl::OnUserChanged(const UserChangedSessionMsg& msg, const NetworkConnectionPtr&)
{
	uint32 sessionID = msg.GetSessionID();
	UserID userId = msg.GetSessionUserID();

	// Find the session that the user is joining
	UserPtr user = NULL;

	SessionImplPtr session = GetLocalSessionById(sessionID);
	
	// NOTE: Its possible for this message to arrive while still waiting for the initial list of sessions.
	// This should be fixed for realz by putting the list of sessions in the handshake
	if (session)
	{
		int32 userCount = session->GetUserCount();

		for (int32 i = 0; i < userCount; ++i)
		{
			UserPtr curUser = session->GetUser(i);
			if (userId == curUser->GetID())
			{
				user = curUser;
				break;
			}
		}
	}
	else
	{
		LogError("OnUserChanged: failed to find session %d for user %d", msg.GetSessionID(), msg.GetSessionUserID());
	}

	if (user)
	{
		// We found a user! 
		// Change the model, notify callbacks, and then bail.
		user->SetName(new XString(msg.GetSessionUserName()));
		user->SetMuteState(msg.GetSessionUserMuteState());

		m_listenerList->NotifyListeners(&SessionManagerListener::OnUserChanged, session, user);
	}
	else
	{
		LogError("OnUserChanged: failed to find user %d for session %d", msg.GetSessionUserID(), msg.GetSessionID());
	}
}


SessionImplPtr SessionManagerImpl::GetLocalSessionById(unsigned int id) const
{
	SessionImplPtr session = NULL;

	size_t sessionCount = m_sessions.size();

	for (size_t i = 0; i < sessionCount; i++)
	{
		SessionImplPtr curSession = m_sessions[i];
		if (curSession->GetSessionId() == id)
		{
			session = curSession;
			break;
		}
	}

	return session;
}


void SessionManagerImpl::OnUserPresenceChanged(const UserPtr&)
{
	UpdateCurrentUserOnServer();
}


void SessionManagerImpl::NotifyOnUserJoinedSessionListeners(SessionPtr session, UserPtr user) const
{
	// Notify the listeners that the user joined the session
	m_listenerList->NotifyListeners(&SessionManagerListener::OnUserJoinedSession, session, user);
}


XTOOLS_NAMESPACE_END
