//////////////////////////////////////////////////////////////////////////
// SyncTests.cpp
//
// Unit tests for the Sync system
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SyncTestObject.h"
#include "SyncTestbed.h"
#include <Tests/Utilities.h>

using namespace XTools;
using namespace XTools::Sync;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Microsoft {
	namespace VisualStudio {
		namespace CppUnitTestFramework {
			template<> std::wstring ToString<XTools::uint16>(const XTools::uint16& t) { RETURN_WIDE_STRING(t); }
			template<> std::wstring ToString<XTools::int64>(const XTools::int64& t) { RETURN_WIDE_STRING(t); }
		}
	}
}

namespace CommonDesktopTests
{
	class SyncMultiConnectionListener : public RefCounted, public IncomingXSocketListener
	{
	public:
		std::vector<XSocketPtr> m_connections;

		virtual void OnNewConnection(const XSocketPtr& newConnection) XTOVERRIDE
		{
			m_connections.push_back(newConnection);
		}
	};
	DECLARE_PTR(SyncMultiConnectionListener)

	TEST_CLASS(SyncTests)
	{
	public:

		TEST_METHOD(TestSingleClient)
		{
			XSocketManagerPtr server = XSocketManager::Create();
			XSocketManagerPtr client1 = XSocketManager::Create();

			NetworkConnectionImplPtr serverConnection1 = new NetworkConnectionImpl(new NetworkMessagePool(10));
			NetworkConnectionImplPtr clientConnection1 = new NetworkConnectionImpl(new NetworkMessagePool(10));

			{
				// Start the server listening for connections
				SyncMultiConnectionListenerPtr listener = new SyncMultiConnectionListener();
				ReceiptPtr listenerReceipt = server->AcceptConnections(kSessionServerPort, 1, listener.get());
				server->Update();

				// Make the client connect to the server
				XSocketPtr connection1 = client1->OpenConnection("localhost", kSessionServerPort);
				MicrosoftTest::WaitForCompletion(
					[&]() {
					return connection1->GetStatus() != XSocket::Status::Connecting && listener->m_connections.size() == 1;
				},
					[&]() {
					server->Update();
					client1->Update();
				},
					5000);

				// Set the connections on the NetworkConnection Objects
				serverConnection1->SetSocket(listener->m_connections[0]);
				clientConnection1->SetSocket(connection1);
			}

			SyncManagerPtr serverSyncMgr = SyncManager::Create(MessageID::SyncMessage, AuthorityLevel::High, new UserImpl("Server", User::kInvalidUserID, false));
			SyncManagerPtr client1SyncMgr = SyncManager::Create(MessageID::SyncMessage, AuthorityLevel::Medium, new UserImpl("Client", User::kInvalidUserID, false));

			serverSyncMgr->AddConnection(serverConnection1);
			client1SyncMgr->AddConnection(clientConnection1);

			// Give the server and client a chance to complete the handshake; though this might not be enough time
			for (int i = 0; i < 10; ++i)
			{
				server->Update();
				client1->Update();
				Platform::SleepMS(5);
			}

			// Setup objects to wrap the root of the synced data set
			SyncTestObjectPtr serverRoot = new SyncTestObject(serverSyncMgr->GetRootObject());
			SyncTestObjectPtr client1Root = new SyncTestObject(client1SyncMgr->GetRootObject());

			// Add a new (but different) child object to both the server and client at the same time
			serverRoot->AddChild("ServerChild_0");
			client1Root->AddChild("Client1Child_0");

			MicrosoftTest::WaitForCompletion(
				[&]() {
				return serverSyncMgr->IsFullySynced() && client1SyncMgr->IsFullySynced();
			},
				[&]() {
				server->Update();
				serverSyncMgr->Update();

				client1->Update();
				client1SyncMgr->Update();
			},
				5000);

			Assert::IsTrue(serverRoot->Equals(client1Root));

			// Add several extra children at different levels of the hierarchy on each side
			serverRoot->GetChild(0)->AddChild("ServerChild_0_0");
			serverRoot->GetChild(0)->AddChild("ServerChild_0_1");
			serverRoot->GetChild(1)->AddChild("ServerChild_1_0");

			client1Root->AddChild("Client1Child_2")->AddChild("Client1Child_2_0");;
			client1Root->GetChild(1)->AddChild("Client1Child_1_0");

			MicrosoftTest::WaitForCompletion(
				[&]() {
				return serverSyncMgr->IsFullySynced() && client1SyncMgr->IsFullySynced();
			},
				[&]() {
				server->Update();
				serverSyncMgr->Update();

				client1->Update();
				client1SyncMgr->Update();
			},
				5000);

			Assert::IsTrue(serverRoot->Equals(client1Root));

			// Test that the sync works when there are no conflicting changes
			client1Root->GetChild(0)->SetFloatValue(1234.f);

			MicrosoftTest::WaitForCompletion(
				[&]() {
				return serverSyncMgr->IsFullySynced() && client1SyncMgr->IsFullySynced();
			},
				[&]() {
				server->Update();
				serverSyncMgr->Update();

				client1->Update();
				client1SyncMgr->Update();
			},
				5000);

			Assert::IsTrue(serverRoot->Equals(client1Root));
		}


		TEST_METHOD(TestSyncCreate)
		{
			SyncTestbedPtr testBed = new SyncTestbed(false);

			for (int i = 0; i < 10; ++i)
			{
				DWORD startTime = GetTickCount();
				testBed->ResetTestBed();

				// Start everyone out with a copy of the same object, so it has to be merged when they connect
				testBed->m_serverRoot->AddChild("PreExistingTest")->SetFloatValue(1.f);
				testBed->m_client1Root->AddChild("PreExistingTest")->SetFloatValue(2.f);
				//client2Root->AddChild("PreExistingTest")->SetFloatValue(3.f);

				testBed->WaitTillFullySynced();
				testBed->ValidateSyncState();

				// Add the same child at all places. This child should be merged so that there is only one with this name on all systems, 
				// and the value as set by the server.
				// Also test nested creates of items with the same name
				testBed->m_serverRoot->AddChild("CommonName")->SetFloatValue(128.f);
				testBed->m_serverRoot->GetChild("CommonName")->AddChild("CommonDecendent");

				testBed->m_client1Root->AddChild("CommonName")->SetFloatValue(25.f);
				testBed->m_client1Root->GetChild("CommonName")->AddChild("CommonDecendent");

				testBed->m_client2Root->AddChild("CommonName")->SetFloatValue(256.f);
				testBed->m_client2Root->GetChild("CommonName")->AddChild("CommonDecendent");

				testBed->WaitTillFullySynced();
				testBed->ValidateSyncState();

				// Add a new (but different) child object to both the server and client at the same time
				testBed->m_serverRoot->AddChild("ServerChild_0");
				testBed->m_client1Root->AddChild("Client1Child_0");
				testBed->m_client2Root->AddChild("Client2Child_0");

				testBed->WaitTillFullySynced();
				testBed->ValidateSyncState();

				// Add several extra children at different levels of the hierarchy on each side
				testBed->m_serverRoot->GetChild(0)->AddChild("ServerChild_0_0");
				testBed->m_serverRoot->GetChild(0)->AddChild("ServerChild_0_1");
				testBed->m_serverRoot->GetChild(1)->AddChild("ServerChild_1_0");

				testBed->m_client1Root->AddChild("Client1Child_2")->AddChild("Client1Child_2_0");;
				testBed->m_client1Root->GetChild(1)->AddChild("Client1Child_1_0");

				testBed->m_client2Root->AddChild("Client2Child_2")->AddChild("Client2Child_2_0");;
				testBed->m_client2Root->GetChild(0)->AddChild("Client2Child_0_0");
				testBed->m_client2Root->GetChild(2)->AddChild("Client2Child_2_0");

				testBed->WaitTillFullySynced();
				testBed->ValidateSyncState();
				MicrosoftTest::LogMessageFormat("Loop %d took %dms\r\n", i, GetTickCount() - startTime);
			}
		}


		TEST_METHOD(TestSyncConflict)
		{
			XSocketManagerPtr server = XSocketManager::Create();
			XSocketManagerPtr mslice1 = XSocketManager::Create();
			XSocketManagerPtr mslice2 = XSocketManager::Create();
			XSocketManagerPtr onsight1 = XSocketManager::Create();
			XSocketManagerPtr onsight2 = XSocketManager::Create();

			NetworkConnectionImplPtr serverConnection1 = new NetworkConnectionImpl(new NetworkMessagePool(10));
			NetworkConnectionImplPtr serverConnection2 = new NetworkConnectionImpl(new NetworkMessagePool(10));

			NetworkConnectionImplPtr clientConnection1A = new NetworkConnectionImpl(new NetworkMessagePool(10));
			NetworkConnectionImplPtr clientConnection1B = new NetworkConnectionImpl(new NetworkMessagePool(10));

			NetworkConnectionImplPtr clientConnection2A = new NetworkConnectionImpl(new NetworkMessagePool(10));
			NetworkConnectionImplPtr clientConnection2B = new NetworkConnectionImpl(new NetworkMessagePool(10));

			NetworkConnectionImplPtr onsightConnection1 = new NetworkConnectionImpl(new NetworkMessagePool(10));
			NetworkConnectionImplPtr onsightConnection2 = new NetworkConnectionImpl(new NetworkMessagePool(10));

			{
				// Start the server listening for connections
				SyncMultiConnectionListenerPtr serverListener = new SyncMultiConnectionListener();
				ReceiptPtr serverListenerReceipt = server->AcceptConnections(kSessionServerPort, 2, serverListener.get());
				server->Update();

				SyncMultiConnectionListenerPtr msliceListener1 = new SyncMultiConnectionListener();
				ReceiptPtr msliceListenerReceipt1 = mslice1->AcceptConnections(kAppPluginPort, 1, msliceListener1.get());

				SyncMultiConnectionListenerPtr msliceListener2 = new SyncMultiConnectionListener();
				ReceiptPtr msliceListenerReceipt2 = mslice2->AcceptConnections(kAppPluginPort+1, 1, msliceListener2.get());

				// Make the client connect to the server
				XSocketPtr connection1 = mslice1->OpenConnection("localhost", kSessionServerPort);
				XSocketPtr connection2 = mslice2->OpenConnection("localhost", kSessionServerPort);

				XSocketPtr connection3 = onsight1->OpenConnection("localhost", kAppPluginPort);
				XSocketPtr connection4 = onsight2->OpenConnection("localhost", kAppPluginPort+1);

				MicrosoftTest::WaitForCompletion(
					[&]() {
					return connection1->GetStatus() != XSocket::Status::Connecting &&
						connection2->GetStatus() != XSocket::Status::Connecting &&
						connection3->GetStatus() != XSocket::Status::Connecting &&
						connection4->GetStatus() != XSocket::Status::Connecting &&
						serverListener->m_connections.size() == 2 &&
						msliceListener1->m_connections.size() == 1 &&
						msliceListener2->m_connections.size() == 1;
				},
					[&]() {
					server->Update();
					mslice1->Update();
					mslice2->Update();
					onsight1->Update();
					onsight2->Update();
				},
					5000);

				// Set the connections on the NetworkConnection Objects
				serverConnection1->SetSocket(serverListener->m_connections[0]);
				serverConnection2->SetSocket(serverListener->m_connections[1]);

				clientConnection1A->SetSocket(connection1);
				clientConnection1B->SetSocket(msliceListener1->m_connections[0]);

				clientConnection2A->SetSocket(connection2);
				clientConnection2B->SetSocket(msliceListener2->m_connections[0]);

				onsightConnection1->SetSocket(connection3);
				onsightConnection2->SetSocket(connection4);
			}

			SyncManagerPtr serverSyncMgr = SyncManager::Create(MessageID::SyncMessage, AuthorityLevel::High, new UserImpl("Server", User::kInvalidUserID, false));
			SyncManagerPtr client1SyncMgr = SyncManager::Create(MessageID::SyncMessage, AuthorityLevel::Medium, new UserImpl("Client1", 1, false));
			SyncManagerPtr client2SyncMgr = SyncManager::Create(MessageID::SyncMessage, AuthorityLevel::Medium, new UserImpl("Client2", 2, false));
			SyncManagerPtr onsight1SyncMgr = SyncManager::Create(MessageID::SyncMessage, AuthorityLevel::Low, new UserImpl("OnSight1", 1, false));
			SyncManagerPtr onsight2SyncMgr = SyncManager::Create(MessageID::SyncMessage, AuthorityLevel::Low, new UserImpl("OnSight2", 2, false));

			serverSyncMgr->AddConnection(serverConnection1);
			serverSyncMgr->AddConnection(serverConnection2);

			client1SyncMgr->AddConnection(clientConnection1A);
			client1SyncMgr->AddConnection(clientConnection1B);

			client2SyncMgr->AddConnection(clientConnection2A);
			client2SyncMgr->AddConnection(clientConnection2B);

			onsight1SyncMgr->AddConnection(onsightConnection1);

			onsight2SyncMgr->AddConnection(onsightConnection2);

			// Give the server and client a chance to complete the handshake; though this might not be enough time
			for (int i = 0; i < 10; ++i)
			{
				server->Update();
				mslice1->Update();
				mslice2->Update();
				onsight1->Update();
				onsight2->Update();
				Platform::SleepMS(5);
			}

			// Setup objects to wrap the root of the synced data set
			SyncTestObjectPtr serverRoot = new SyncTestObject(serverSyncMgr->GetRootObject());
			SyncTestObjectPtr client1Root = new SyncTestObject(client1SyncMgr->GetRootObject());
			SyncTestObjectPtr client2Root = new SyncTestObject(client2SyncMgr->GetRootObject());
			SyncTestObjectPtr onsight1Root = new SyncTestObject(onsight1SyncMgr->GetRootObject());
			SyncTestObjectPtr onsight2Root = new SyncTestObject(onsight2SyncMgr->GetRootObject());

			// Add a single child to test on
			onsight1Root->AddChild("Client1Child_0");

			MicrosoftTest::WaitForCompletion(
				[&]() {
				return serverSyncMgr->IsFullySynced() &&
					client1SyncMgr->IsFullySynced() &&
					client2SyncMgr->IsFullySynced() &&
					onsight1SyncMgr->IsFullySynced() &&
					onsight2SyncMgr->IsFullySynced();
			},
				[&]() {
				server->Update();
				serverSyncMgr->Update();

				mslice1->Update();
				client1SyncMgr->Update();

				mslice2->Update();
				client2SyncMgr->Update();

				onsight1->Update();
				onsight1SyncMgr->Update();

				onsight2->Update();
				onsight2SyncMgr->Update();
			},
				5000);

			Assert::IsTrue(serverRoot->Equals(client1Root));
			Assert::IsTrue(serverRoot->Equals(client2Root));
			Assert::IsTrue(client1Root->Equals(onsight1Root));
			Assert::IsTrue(client2Root->Equals(onsight2Root));

			// Test that the sync works when there are conflicting changes
			onsight1Root->GetChild(0)->SetFloatValue(1234.f);
			onsight2Root->GetChild(0)->SetFloatValue(6789.f);

			onsight1Root->GetChild(0)->SetIntValue(51);
			onsight2Root->GetChild(0)->SetIntValue(50);

			onsight1Root->GetChild(0)->SetStringValue("Go");
			onsight2Root->GetChild(0)->SetStringValue("Bears");

			onsight1Root->GetChild(0)->SetLongValue(0xDEADBEEFDEADBEEF);
			onsight2Root->GetChild(0)->SetLongValue(0xEFEFEFEFEFEFEFEF);

			onsight1Root->GetChild(0)->SetDoubleValue(567890.0);
			onsight2Root->GetChild(0)->SetDoubleValue(12345678.0);

			MicrosoftTest::WaitForCompletion(
				[&]() {
				return serverSyncMgr->IsFullySynced() &&
					client1SyncMgr->IsFullySynced() &&
					client2SyncMgr->IsFullySynced() &&
					onsight1SyncMgr->IsFullySynced() &&
					onsight2SyncMgr->IsFullySynced();
			},
				[&]() {
				server->Update();
				serverSyncMgr->Update();

				mslice1->Update();
				client1SyncMgr->Update();

				mslice2->Update();
				client2SyncMgr->Update();

				onsight1->Update();
				onsight1SyncMgr->Update();

				onsight2->Update();
				onsight2SyncMgr->Update();
			},
				5000);

			Assert::IsTrue(serverRoot->Equals(client1Root));
			Assert::IsTrue(serverRoot->Equals(client2Root));
			Assert::IsTrue(client1Root->Equals(onsight1Root));
			Assert::IsTrue(client2Root->Equals(onsight2Root));
		}

		TEST_METHOD(TestSync_AddsBeforeConnect)
		{
			SyncTestbedPtr testBed = new SyncTestbed(true);

			for (int i = 0; i < 10; ++i)
			{
				DWORD startTime = GetTickCount();
				testBed->ResetTestBed();

				std::string childName("TempChild");

				testBed->m_onsight1Root->AddChild(childName);

				testBed->m_onsight1SyncMgr->Update();

				testBed->m_onsight1Root->GetChild(childName)->AddChild(childName);

				testBed->WaitTillFullySynced();
				testBed->ValidateSyncState();
				MicrosoftTest::LogMessageFormat("Loop %d took %dms\r\n", i, GetTickCount() - startTime);
			}
		}
	};
}
