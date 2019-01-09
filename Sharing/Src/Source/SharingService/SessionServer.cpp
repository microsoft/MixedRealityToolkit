//////////////////////////////////////////////////////////////////////////
// SessionServer.cpp
//
// Main class for the session server
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SessionServer.h"
#include "Private/NetworkConnectionImpl.h"
#include "Private/json/cpprest/json.h"
#include "Private/JSONMessage.h" // CommonPrivate.h?
#include "Private/NewSessionRequest.h"
#include "Private/NewSessionReply.h"
#include "Private/JoinSessionRequest.h"
#include "Private/JoinSessionReply.h"
#include "Private/Utils/ScopedLock.h"
#include "Private/SessionDescriptorImpl.h"
#include "Private/Utils/FileLogWriter.h"
#include "Private/ProfileManagerImpl.h"

XTOOLS_NAMESPACE_BEGIN

const std::string SessionServer::s_defaultLogBaseLocation = "C:\\";

SessionServer::SessionServer(PWSTR pszServiceName)
: ServiceBase(pszServiceName, TRUE, TRUE, FALSE)
, m_messagePool(new NetworkMessagePool(kDefaultMessagePoolSize))
, m_socketMgr(XSocketManager::Create())
, m_stopping(FALSE)
, m_nextSessionId(0)
, m_logWriter(nullptr)
{
#if defined(XTOOLS_DEBUG)
	// Enable debug memory tracking and verification. 
	MemoryManager::EnableDebugAllocators();
#endif

	m_messageRouter.RegisterHandler(new MessageHandlerProxyT<const NewSessionRequest&>(CreateCallback2(this, &SessionServer::OnNewSessionRequest)));
	m_messageRouter.RegisterHandler(new MessageHandlerProxyT<const ListSessionsRequest&>(CreateCallback2(this, &SessionServer::OnListSessionsRequest)));

	m_profileMgr = new ProfileManagerImpl(m_socketMgr, SystemRole::SessionDiscoveryServerRole);

#if defined(MSTEST)
	m_broadcaster = new BroadcastForwarder();
#endif
}


void SessionServer::OnStart(DWORD dwArgc, PWSTR *pszArgv)
{
    InitializeFileLogger();

    // Log out an obvious piece of text to help distinguish between server sessions.
    LogInfo("***********************************");
    LogInfo("****** Sharing Service OnStart ******");
    LogInfo("***********************************");

	// Log a service start message to the Application log.
	WriteEventLogEntry(L"Sharing Server starting", EVENTLOG_INFORMATION_TYPE);
	LogInfo("Server Info: \n\tBuild Version: %ls \n\tSchema Version: %i", XTOOLS_VERSION_STRING, kXToolsSchemaVersion);

	// TODO: use different machines, etc, based on command line parameters
	XT_UNREFERENCED_PARAM(dwArgc);
	XT_UNREFERENCED_PARAM(pszArgv);

	// Start listening for new connections
	m_listenerReceipt = m_socketMgr->AcceptConnections(kSessionServerPort, kSessionServerMaxConnections, this);
	LogInfo("Listening for session list connections on port %i of all network devices of the local machine.", kSessionServerPort);
	LogInfo("Local IP addresses are:");

	IPAddressList addressList = m_socketMgr->GetLocalMachineAddresses();
	for (size_t i = 0; i < addressList.size(); ++i)
	{
		LogInfo("\t%s", addressList[i].ToString().c_str());
	}
	
	// Allocate a pool of ports to use for sessions
	m_portPool = new PortPool(kSessionServerPort-1, 256);

	// TODO: Read from a configuration file for persistent sessions. 
	XTVERIFY(CreateNewSession("Default", SessionType::PERSISTENT) != NULL);

	// Start a thread to run the main service logic. 
	m_serverThread = new MemberFuncThread(&SessionServer::ServerThreadFunc, this);
}


void SessionServer::OnStop()
{
    LogInfo("**********************************");
    LogInfo("****** Sharing Service OnStop ******");
    LogInfo("**********************************");

	// Log a service stop message to the Application log.
	WriteEventLogEntry(L"Sharing Server stopping", EVENTLOG_INFORMATION_TYPE);

	// Indicate that the service is stopping and wait for the finish of the 
	// main service function (ServerThreadFunc).
	m_stopping = TRUE;
	m_serverThread->WaitForThreadExit();

    TeardownFileLogger();

}


void SessionServer::OnUserJoinedSession(uint32 sessionID, const std::string& userName, UserID userID, bool muteState)
{
	// Hold the lock during callbacks from the sessions, which run in their own threads 
	ScopedLock lock(m_mutex);

    UserJoinedSessionMsg joinedMsg(sessionID, userName, userID, muteState);
	SendSessionMessageToAllClients(joinedMsg.ToJSONString());
}


void SessionServer::OnUserLeftSession(uint32 sessionID, UserID userID)
{
	// Hold the lock during callbacks from the sessions, which run in their own threads 
	ScopedLock lock(m_mutex);

	UserLeftSessionMsg joinedMsg(sessionID, userID);
	SendSessionMessageToAllClients(joinedMsg.ToJSONString());
}


void SessionServer::OnUserChanged(uint32 sessionID, const std::string& userName, UserID userID, bool muteState)
{
    ScopedLock lock(m_mutex);

    UserChangedSessionMsg changedMsg(sessionID, userName, userID, muteState);
    SendSessionMessageToAllClients(changedMsg.ToJSONString());
}


void SessionServer::OnSessionEmpty(const XSessionConstPtr& session)
{
	// Hold the lock during callbacks from the sessions, which run in their own threads 
	ScopedLock lock(m_mutex);

	XT_UNREFERENCED_PARAM(session);

	// Log a service stop message to the Application log.
	WriteEventLogEntry(L"Sharing Service session empty", EVENTLOG_INFORMATION_TYPE);

	if (session->GetType() == SessionType::ADHOC)
	{
		// Find the session in the list
		for (size_t i = 0; i < m_sessions.size(); ++i)
		{
			if (m_sessions[i] == session)
			{
				uint32 sessionID = session->GetId();

				// Queue the sessions the be deleted on the server's thread
				m_sessionsPendingDeletion.push_back(m_sessions[i]);

				// Erase the session from the list
				m_sessionChangeListener.erase(m_sessionChangeListener.find(m_sessions[i]));
				m_sessions.erase(m_sessions.begin() + i);
				
				// Notify the clients that the session has closed
				SessionClosedMsg closedMsg(sessionID);
				SendSessionMessageToAllClients(closedMsg.ToJSONString());

				break;
			}
		}
	}
}


void SessionServer::OnNewConnection(const XSocketPtr& newConnection)
{
	HandshakeCallback callback = CreateCallback3(this, &SessionServer::OnHandshakeComplete);

	m_pendingConnections[newConnection->GetID()] = new NetworkHandshake(newConnection, new SessionListHandshakeLogic(true), callback);
}


void SessionServer::OnHandshakeComplete(const XSocketPtr& newConnection, SocketID socketID, HandshakeResult result)
{
    if (newConnection && result == HandshakeResult::Success)
	{
		Client newClient;

		newClient.m_connection = new NetworkConnectionImpl(m_messagePool);
		newClient.m_connection->SetSocket(newConnection);

		// Begin to listen for SessionControl messages from this connection.
		newClient.m_connection->AddListener(MessageID::SessionControl, this);
		newClient.m_receipt = CreateRegistrationReceipt(newClient.m_connection, &NetworkConnectionImpl::RemoveListener, MessageID::SessionControl, this);

		// Keep a reference to this connection so that it stays open.
		m_clients.push_back(newClient);

#if defined(MSTEST)
		m_broadcaster->AddConnection(newClient.m_connection);
#endif
	}
    else
    {
        LogInfo("ListServer: Handshake failed with error %u", result);
    }
	
	XTVERIFY(m_pendingConnections.erase(socketID));
}


void SessionServer::OnDisconnected(const NetworkConnectionPtr& connection)
{
#if defined(MSTEST)
	m_broadcaster->RemoveConnection(connection);
#endif

	for (size_t i = 0; i < m_clients.size(); ++i)
	{
		if (m_clients[i].m_connection == connection)
		{
			m_clients.erase(m_clients.begin() + i);
			break;
		}
	}
}


void SessionServer::OnMessageReceived(const NetworkConnectionPtr& connection, NetworkInMessage& message)
{
    XStringPtr command = message.ReadString();

	JSONMessagePtr jMsg = JSONMessage::CreateFromMessage(command->GetString());

	// Route the incoming message to the appropriate function to handle it
	if (!m_messageRouter.CallHandler(jMsg, connection))
	{
		// We got a bad or unexpected message; break the connection
		connection->Disconnect();
	}
}



XSessionImplPtr SessionServer::CreateNewSession(const std::string& sessionName, SessionType type)
{
    XSessionImplPtr xsession;
	uint16 sessionPort;

    // Better way to check for error?
    if (m_portPool->GetPort(sessionPort))
    {
        uint32 id = GetNewSessionId();

        xsession = new XSessionImpl(sessionName, sessionPort, type, id);

		// Check that the session was able to set itself up, open a socket to listen on, etc
		for (int creationCount = 0; !xsession->IsInitialized() && creationCount < 10; ++creationCount)
		{
			m_portPool->ReleasePort(sessionPort);

			if (m_portPool->GetPort(sessionPort))
			{
				xsession = new XSessionImpl(sessionName, sessionPort, type, id);
			}
			else
			{
				break;
			}
		}

		if (xsession->IsInitialized())
		{
			m_sessionChangeListener[xsession] = xsession->RegisterCallback(this);
			m_sessions.push_back(xsession);
			LogInfo("Created Session \"%s\" with ID %u on port %i", sessionName.c_str(), id, sessionPort);

			// Notify the listeners about the new session
			for (size_t i = 0; i < m_clients.size(); ++i)
			{
				SessionAddedMsg sessionAddedMsg;
				sessionAddedMsg.SetSessionDescriptor(xsession->GetSessionDescription(m_clients[i].m_connection->GetSocket()));

				NetworkOutMessagePtr msg = m_clients[i].m_connection->CreateMessage((byte)MessageID::SessionControl);
				msg->Write(sessionAddedMsg.ToJSONString());

				m_clients[i].m_connection->Send(msg, MessagePriority::Medium, MessageReliability::ReliableOrdered, MessageChannel::Default, true);
			}
		}
		else
		{
			LogError("Failed to create new session %s", sessionName.c_str());
			xsession = NULL;
		}
    }

    return xsession;
}


uint32 SessionServer::GetNewSessionId()
{
    uint32 sessionId = m_nextSessionId;
    m_nextSessionId++;
    return sessionId;
}


void SessionServer::OnNewSessionRequest(const NewSessionRequest& request, const NetworkConnectionPtr& connection)
{
	std::string name = request.GetSessionName();
	SessionType sessionType = request.GetSessionType();

	XSessionImplPtr session;
	std::string failureReason;

	// Cannot create a session with a name that is too short
	if (name.length() < kMinSessionNameLength)
	{
		failureReason = "Session name must have at least " + std::to_string(kMinSessionNameLength) + " letters";
	}

	// Cannot create a session with a name that is too long
	else if (name.length() > kMaxSessionNameLength)
	{
		failureReason = "Session name cannot be more than " + std::to_string(kMaxSessionNameLength) + " letters";
	}
	else
	{
		// Check to make sure that the requested session name is not already taken
		for (size_t i = 0; i < m_sessions.size(); ++i)
		{
			if (m_sessions[i]->GetName() == name)
			{
				failureReason = "A session with that name already exists";
				break;
			}
		}
	}
	

	if (failureReason.empty())
	{
		session = CreateNewSession(name, sessionType);
	}
	
	// If the session was successfully created...
	if (session)
	{
		// Report success.
		std::string address = m_socketMgr->GetLocalAddressForRemoteClient(connection->GetSocket());
		uint16 port = session->GetPort();

		NewSessionReply reply(
			session->GetId(), 
			session->GetType(),
			name, 
			address, 
			port);

		NetworkOutMessagePtr response = connection->CreateMessage(MessageID::SessionControl);

		response->Write(reply.ToJSONString());

		connection->Send(response);
	}
	else
	{
		// Report failure
		NewSessionReply reply(failureReason);

		NetworkOutMessagePtr response = connection->CreateMessage(MessageID::SessionControl);

		response->Write(reply.ToJSONString());

		connection->Send(response);
	}
}


void SessionServer::OnListSessionsRequest(const ListSessionsRequest&, const NetworkConnectionPtr& connection)
{
	// Fill in a message with a list of all the sessions and send it

	ListSessionsReply reply;

	reply.SetSessionCount(static_cast<int32>(m_sessions.size()));

	for (size_t i = 0; i < m_sessions.size(); ++i)
	{
		XSessionImplPtr currentSession = m_sessions[i];

		reply.SetSessionDescriptor((int32)i, currentSession->GetSessionDescription(connection->GetSocket()));
	}

	NetworkOutMessagePtr msg = connection->CreateMessage(MessageID::SessionControl);
	msg->Write(reply.ToJSONString());

	connection->Send(msg);
}


void SessionServer::SendSessionMessageToAllClients(const std::string& message)
{
	NetworkOutMessagePtr msg = m_messagePool->AcquireMessage();
	msg->Write((byte)MessageID::SessionControl);
	msg->Write(message);

	for (size_t i = 0; i < m_clients.size(); ++i)
	{
		m_clients[i].m_connection->Send(msg, MessagePriority::Medium, MessageReliability::ReliableOrdered, MessageChannel::Default, false);
	}

	m_messagePool->ReturnMessage(msg);
}


// The entry point for the main thread of the session server
void SessionServer::ServerThreadFunc()
{
	// Periodically check if the service is stopping.
	while (!m_stopping)
	{
		{
			// Hold the lock while processing incoming messages, 
			// so we don't have race conditions with callbacks from other
			// the sessions, which run in their own threads
			ScopedLock lock(m_mutex);

			// Release the ports that the closing session were using
			for (size_t i = 0; i < m_sessionsPendingDeletion.size(); ++i)
			{
				m_portPool->ReleasePort(m_sessionsPendingDeletion[i]->GetPort());
			}

			// Clear any sessions pending deletion
			m_sessionsPendingDeletion.clear();

			m_socketMgr->Update();

			//reflection_cast<IUpdateable>(m_profileMgr)->Update();
		}

		m_socketMgr->GetMessageArrivedEvent().WaitTimeout(10);
	}
}

std::string SessionServer::GetCurrentDateTimeString(const tm& tm) const
{
    std::string dt;
    dt += std::to_string(tm.tm_hour) + ":";
    dt += std::to_string(tm.tm_min) + ":";
    dt += std::to_string(tm.tm_sec) + ", "; 
    dt += std::to_string(tm.tm_mon + 1) + "-"; 
    dt += std::to_string(tm.tm_mday) + "-"; 
    dt += std::to_string(tm.tm_year + 1900);
    return dt;
}

std::string SessionServer::GetLogFileName(const tm& tm) const
{
    // File names look like this: SessionServer_YYYYMMDD

    std::string fileName = "SessionServer_";
    fileName += std::to_string(tm.tm_year + 1900);

    // Add a leading zero to single digit months.
    fileName += tm.tm_mon + 1 < 10 ? "0" : "";
    fileName += std::to_string(tm.tm_mon + 1);

    // Add a leading zero to single digit days.
    fileName += tm.tm_mday + 1 < 10 ? "0" : "";
    fileName += std::to_string(tm.tm_mday);
    fileName += ".log";

    return fileName;
}

void SessionServer::InitializeFileLogger()
{
    time_t curTime = time(NULL);
    tm curTM;
    localtime_s(&curTM, &curTime);

    if (!m_logWriter)
    {
        std::string filePath = s_defaultLogBaseLocation;
        filePath += "SharingServiceLogs";
        filePath += "\\";

        BOOL dirResult = CreateDirectoryA(filePath.c_str(), NULL);

        if (dirResult || GetLastError() == ERROR_ALREADY_EXISTS)
        {
            // Either we succeeded in creating the directory or it already exists.
            std::string fileName = "SharingService_";
            fileName += std::to_string(curTM.tm_year + 1900);
            fileName += std::to_string(curTM.tm_mon + 1);
            fileName += std::to_string(curTM.tm_mday);
            fileName += ".log";

            std::string fullPath = filePath + GetLogFileName(curTM);

            m_logWriter = new FileLogWriter();
            m_logWriter->AddTargetFile(fullPath);
        }
    }


	m_logger = new Logger();

	m_logger->SetWriter(m_logWriter);
    std::string curTimeString = GetCurrentDateTimeString(curTM);
    LogInfo(" ** Logging Session Began at %s", curTimeString.c_str());

}


void SessionServer::TeardownFileLogger()
{
    time_t curTime = time(NULL);
    tm curTM;
    localtime_s(&curTM, &curTime);
    
	m_logger->ClearWriter();

    std::string curTimeString = GetCurrentDateTimeString(curTM);

    LogInfo(" ** Logging Session Ended at %s", curTimeString.c_str());
    if (m_logWriter)
    {
        delete m_logWriter;
        m_logWriter = NULL;
    }
}



XTOOLS_NAMESPACE_END
