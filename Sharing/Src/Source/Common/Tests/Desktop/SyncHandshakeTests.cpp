//////////////////////////////////////////////////////////////////////////
// SyncHandshakeTests.cpp
//
// Unit tests for the Sync system
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Tests/Utilities.h>

using namespace XTools;
using namespace XTools::Sync;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Microsoft {
	namespace VisualStudio {
		namespace CppUnitTestFramework {
			template<> std::wstring ToString<XTools::uint16>(const XTools::uint16& t) { RETURN_WIDE_STRING(t); }
			template<> std::wstring ToString<XTools::int64>(const XTools::int64& t) { RETURN_WIDE_STRING(t); }
			template<> std::wstring ToString<XSocket::Status>(const XSocket::Status& t) { RETURN_WIDE_STRING((int)t); }
		}
	}
}

// Disable warning about LINE_INFO() "nonstandard extension used : class rvalue used as lvalue"
#pragma warning( disable : 4238 )

namespace CommonDesktopTests
{
	class SyncConnectionListener : public RefCounted, public IncomingXSocketListener
	{
	public:
		XSocketPtr m_connection = nullptr;

		virtual void OnNewConnection(const XSocketPtr& newConnection) XTOVERRIDE
		{
			Microsoft::VisualStudio::CppUnitTestFramework::Logger::WriteMessage(L"New Connection!\r\n");

			m_connection = newConnection;
		}
	};

	DECLARE_PTR(SyncConnectionListener)

	TEST_CLASS(SyncHandshakeTests)
	{
	public:

		TEST_METHOD(TestSyncConnectSuccess)
		{
			XSocketManagerPtr server = XSocketManager::Create();
			XSocketManagerPtr client = XSocketManager::Create();

			NetworkMessagePoolPtr serverMessagePool = new NetworkMessagePool(10);
			NetworkMessagePoolPtr clientMessagePool = new NetworkMessagePool(10);

			NetworkConnectionImplPtr serverConnection = new NetworkConnectionImpl(serverMessagePool);
			NetworkConnectionImplPtr clientConnection = new NetworkConnectionImpl(clientMessagePool);

			{
				// Start the server listening for connections
				SyncConnectionListenerPtr listener = new SyncConnectionListener();
				ReceiptPtr listenerReceipt = server->AcceptConnections(kSessionServerPort, 2, listener.get());
				server->Update();

				// Make the client connect to the server
				XSocketPtr connection = client->OpenConnection("localhost", kSessionServerPort);
				MicrosoftTest::WaitForCompletion(
					[&]() {
					return connection->GetStatus() != XSocket::Status::Connecting && listener->m_connection != NULL;
				},
					[&]() {
					server->Update();
					client->Update();
				},
					5000);

				Assert::AreEqual(XSocket::Status::Connected, connection->GetStatus(), L"The client connection status was not \"Connected\"", LINE_INFO());

				// Set the connections on the NetworkConnection Objects
				serverConnection->SetSocket(listener->m_connection);
				clientConnection->SetSocket(connection);
			}

			UserPtr user1 = new UserImpl("User1", 1, false);
			UserPtr user2 = new UserImpl("User2", 2, true);

			SyncManagerPtr serverSyncMgr = SyncManager::Create(MessageID::SyncMessage, AuthorityLevel::High, user1);
			SyncManagerPtr clientSyncMgr = SyncManager::Create(MessageID::SyncMessage, AuthorityLevel::Medium, user2);

			serverSyncMgr->AddConnection(serverConnection);
			clientSyncMgr->AddConnection(clientConnection);

			// allow the handshake messages to go back and forth
			for (int i = 0; i < 10; ++i)
			{
				server->Update();
				client->Update();
				Platform::SleepMS(5);
			}

			Assert::IsTrue(serverConnection->IsConnected(), L"The server connection is not connected", LINE_INFO());
			Assert::IsTrue(clientConnection->IsConnected(), L"The client connection is not connected", LINE_INFO());
		}

		TEST_METHOD(TestSyncConnectFailure)
		{
			XSocketManagerPtr server = XSocketManager::Create();
			XSocketManagerPtr client = XSocketManager::Create();

			NetworkMessagePoolPtr serverMessagePool = new NetworkMessagePool(10);
			NetworkMessagePoolPtr clientMessagePool = new NetworkMessagePool(10);

			NetworkConnectionImplPtr serverConnection = new NetworkConnectionImpl(serverMessagePool);
			NetworkConnectionImplPtr clientConnection = new NetworkConnectionImpl(clientMessagePool);

			{
				// Start the server listening for connections
				SyncConnectionListenerPtr listener = new SyncConnectionListener();
				ReceiptPtr listenerReceipt = server->AcceptConnections(kSessionServerPort, 2, listener.get());
				server->Update();

				// Make the client connect to the server
				XSocketPtr connection = client->OpenConnection("localhost", kSessionServerPort);
				MicrosoftTest::WaitForCompletion(
					[&]() {
					return connection->GetStatus() != XSocket::Status::Connecting && listener->m_connection != NULL;
				},
					[&]() {
					server->Update();
					client->Update();
				},
					5000);

				Assert::AreEqual(XSocket::Status::Connected, connection->GetStatus(), L"The client connection status was not \"Connected\"", LINE_INFO());

				// Set the connections on the NetworkConnection Objects
				serverConnection->SetSocket(listener->m_connection);
				clientConnection->SetSocket(connection);
			}

			// connections using the same auth level should disconnect themselves
			UserPtr user1 = new UserImpl("User1", 1, false);
			UserPtr user2 = new UserImpl("User2", 2, true);

			SyncManagerPtr serverSyncMgr = SyncManager::Create(MessageID::SyncMessage, AuthorityLevel::Medium, user1);
			SyncManagerPtr clientSyncMgr = SyncManager::Create(MessageID::SyncMessage, AuthorityLevel::Medium, user2);

			serverSyncMgr->AddConnection(serverConnection);
			clientSyncMgr->AddConnection(clientConnection);

			// allow the handshake messages to go back and forth
			for (int i = 0; i < 10 && (serverConnection->IsConnected() || clientConnection->IsConnected()); ++i)
			{
				server->Update();
				client->Update();
				Platform::SleepMS(5);
			}

			Assert::IsFalse(serverConnection->IsConnected(), L"The server connection was \"Connected\"", LINE_INFO());
			Assert::IsFalse(clientConnection->IsConnected(), L"The client connection was \"Connected\"", LINE_INFO());
		}
	};
}
