//////////////////////////////////////////////////////////////////////////
// SyncTestbed.cpp
//
// Contains all the setup and objects for testing scenarios with the sync system
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SyncTestbed.h"
#include <Tests/Utilities.h>

using namespace XTools;
using namespace XTools::Sync;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

// Disable warning about LINE_INFO() "nonstandard extension used : class rvalue used as lvalue"
#pragma warning( disable : 4238 )

namespace CommonDesktopTests
{
	void SyncTestbedConnectionListener::OnNewConnection(const XTools::XSocketPtr& newConnection)
	{
		m_connections.push_back(newConnection);
	}

	

	SyncTestbed::SyncTestbed(bool bConnectOnSight)
		: m_bConnectOnSight(bConnectOnSight)
	{
		TestLogWriter::Init();

		m_userServer = new UserImpl("Server", User::kInvalidUserID, false);
		m_userClient1 = new UserImpl("Client1", 1, false);
		m_userOnSight1 = new UserImpl("OnSight1", 1, false);
		m_userClient2 = new UserImpl("Client2", 2, true);
		m_userOnSight2 = new UserImpl("OnSight2", 2, true);

		m_serverSocketMgr = XSocketManager::Create();
		m_mslice1SocketMgr = XSocketManager::Create();
		m_mslice2SocketMgr = XSocketManager::Create();
		m_onsight1SocketMgr = XSocketManager::Create();
		m_onsight2SocketMgr = XSocketManager::Create();

		m_serverConnection1 = new NetworkConnectionImpl(new NetworkMessagePool(10));
		m_serverConnection2 = new NetworkConnectionImpl(new NetworkMessagePool(10));

		m_clientConnection1A = new NetworkConnectionImpl(new NetworkMessagePool(10));
		m_clientConnection1B = new NetworkConnectionImpl(new NetworkMessagePool(10));

		m_clientConnection2A = new NetworkConnectionImpl(new NetworkMessagePool(10));
		m_clientConnection2B = new NetworkConnectionImpl(new NetworkMessagePool(10));

		m_onsightConnection1 = new NetworkConnectionImpl(new NetworkMessagePool(10));
		m_onsightConnection2 = new NetworkConnectionImpl(new NetworkMessagePool(10));

		{
			// Start the server listening for connections
			SyncTestbedConnectionListenerPtr serverListener = new SyncTestbedConnectionListener();
			ReceiptPtr serverListenerReceipt = m_serverSocketMgr->AcceptConnections(kSessionServerPort, 2, serverListener.get());

			SyncTestbedConnectionListenerPtr msliceListener1 = new SyncTestbedConnectionListener();
			ReceiptPtr msliceListenerReceipt1 = m_mslice1SocketMgr->AcceptConnections(kAppPluginPort, 1, msliceListener1.get());

			SyncTestbedConnectionListenerPtr msliceListener2 = new SyncTestbedConnectionListener();
			ReceiptPtr msliceListenerReceipt2 = m_mslice2SocketMgr->AcceptConnections(kAppPluginPort+1, 1, msliceListener2.get());

			// Make the client connect to the server
			XSocketPtr connection1 = m_mslice1SocketMgr->OpenConnection("localhost", kSessionServerPort);
			XSocketPtr connection2 = m_mslice2SocketMgr->OpenConnection("localhost", kSessionServerPort);

			XSocketPtr connection3 = m_onsight1SocketMgr->OpenConnection("localhost", kAppPluginPort);
			XSocketPtr connection4 = m_onsight2SocketMgr->OpenConnection("localhost", kAppPluginPort+1);

			MicrosoftTest::WaitForCompletion(
				[&]() {
				return 
					connection1->GetStatus() != XSocket::Status::Connecting &&
					connection2->GetStatus() != XSocket::Status::Connecting &&
					connection3->GetStatus() != XSocket::Status::Connecting &&
					connection4->GetStatus() != XSocket::Status::Connecting &&
					serverListener->m_connections.size() == 2 &&
					msliceListener1->m_connections.size() == 1 &&
					msliceListener2->m_connections.size() == 1
					;
			},
				[&]() {
				m_serverSocketMgr->Update();
				m_mslice1SocketMgr->Update();
				m_mslice2SocketMgr->Update();
				m_onsight1SocketMgr->Update();
				m_onsight2SocketMgr->Update();
			},
				5000);

			// Set the connections on the NetworkConnection Objects
			m_serverConnection1->SetSocket(serverListener->m_connections[0]);
			m_serverConnection2->SetSocket(serverListener->m_connections[1]);

			m_clientConnection1A->SetSocket(connection1);
			m_clientConnection1B->SetSocket(msliceListener1->m_connections[0]);

			m_clientConnection2A->SetSocket(connection2);
			m_clientConnection2B->SetSocket(msliceListener2->m_connections[0]);

			m_onsightConnection1->SetSocket(connection3);
			m_onsightConnection2->SetSocket(connection4);
		}
	}

	void SyncTestbed::ResetTestBed()
	{
		Assert::IsTrue(m_serverConnection1->IsConnected(), L"Server connection to client 1 is not connected.", LINE_INFO());
		Assert::IsTrue(m_serverConnection2->IsConnected(), L"Server connection to client 1 is not connected.", LINE_INFO());

		Assert::IsTrue(m_clientConnection1A->IsConnected(), L"Client 1 connection to server is not connected.", LINE_INFO());
		Assert::IsTrue(m_clientConnection2A->IsConnected(), L"Client 2 connection to server is not connected.", LINE_INFO());

		if (m_bConnectOnSight)
		{
			Assert::IsTrue(m_clientConnection1B->IsConnected(), L"Client 1 connection to server is not connected.", LINE_INFO());
			Assert::IsTrue(m_clientConnection2B->IsConnected(), L"Client 2 connection to server is not connected.", LINE_INFO());

			Assert::IsTrue(m_onsightConnection1->IsConnected(), L"Timed out waiting for the operation to complete.", LINE_INFO());
			Assert::IsTrue(m_onsightConnection2->IsConnected(), L"Timed out waiting for the operation to complete.", LINE_INFO());
		}

		m_serverSyncMgr = SyncManager::Create(MessageID::SyncMessage, AuthorityLevel::High, m_userServer);
		m_client1SyncMgr = SyncManager::Create(MessageID::SyncMessage, AuthorityLevel::Medium, m_userClient1);
		m_client2SyncMgr = SyncManager::Create(MessageID::SyncMessage, AuthorityLevel::Medium, m_userClient2);
		m_onsight1SyncMgr = SyncManager::Create(MessageID::SyncMessage, AuthorityLevel::Low, m_userOnSight1);
		m_onsight2SyncMgr = SyncManager::Create(MessageID::SyncMessage, AuthorityLevel::Low, m_userOnSight2);

		m_serverSyncMgr->AddConnection(m_serverConnection1);
		m_serverSyncMgr->AddConnection(m_serverConnection2);

		m_client1SyncMgr->AddConnection(m_clientConnection1A);
		m_client2SyncMgr->AddConnection(m_clientConnection2A);

		if (m_bConnectOnSight)
		{
			m_client1SyncMgr->AddConnection(m_clientConnection1B);
			m_client2SyncMgr->AddConnection(m_clientConnection2B);

			m_onsight1SyncMgr->AddConnection(m_onsightConnection1);
			m_onsight2SyncMgr->AddConnection(m_onsightConnection2);
		}

		// Give the server and client a chance to complete the handshake; though this might not be enough time
		for (int i = 0; i < 10; ++i)
		{
			m_serverSocketMgr->Update();
			m_mslice1SocketMgr->Update();
			m_mslice2SocketMgr->Update();
			m_onsight1SocketMgr->Update();
			m_onsight2SocketMgr->Update();
			Platform::SleepMS(5);
		}

		// Setup objects to wrap the root of the synced data set
		m_serverRoot = new SyncTestObject(m_serverSyncMgr->GetRootObject());
		m_client1Root = new SyncTestObject(m_client1SyncMgr->GetRootObject());
		m_client2Root = new SyncTestObject(m_client2SyncMgr->GetRootObject());
		m_onsight1Root = new SyncTestObject(m_onsight1SyncMgr->GetRootObject());
		m_onsight2Root = new SyncTestObject(m_onsight2SyncMgr->GetRootObject());
	}

	void SyncTestbed::WaitTillFullySynced()
	{
		DWORD startTime = GetTickCount();

		MicrosoftTest::WaitForCompletion(
			[&]() {
			return m_serverSyncMgr->IsFullySynced() &&
				m_client1SyncMgr->IsFullySynced() &&
				m_client2SyncMgr->IsFullySynced() &&
				m_onsight1SyncMgr->IsFullySynced() &&
				m_onsight2SyncMgr->IsFullySynced();
		},
			[&]() {
			m_serverSocketMgr->Update();
			m_serverSyncMgr->Update();

			m_mslice1SocketMgr->Update();
			m_client1SyncMgr->Update();

			m_mslice2SocketMgr->Update();
			m_client2SyncMgr->Update();

			if (m_bConnectOnSight)
			{
				m_onsight1SocketMgr->Update();
				m_onsight1SyncMgr->Update();

				m_onsight2SocketMgr->Update();
				m_onsight2SyncMgr->Update();
			}
		},
			5000);

		MicrosoftTest::LogMessageFormat("WaitTillFullySynced took %dms\r\n", GetTickCount() - startTime);
	}

	void SyncTestbed::ValidateSyncState()
	{
		// First compare the internal values in the sync system
		CompareElementRecurs(m_serverRoot->GetElement(), m_client1Root->GetElement());
		CompareElementRecurs(m_serverRoot->GetElement(), m_client2Root->GetElement());
		if (m_bConnectOnSight)
		{
			CompareElementRecurs(m_client1Root->GetElement(), m_onsight1Root->GetElement());
			CompareElementRecurs(m_client2Root->GetElement(), m_onsight2Root->GetElement());
		}

		// Then compare the values in the synced objects, to ensure callbacks worked correctly
		ValidatePair(m_serverSyncMgr, m_serverRoot, m_client1SyncMgr, m_client1Root);
		ValidatePair(m_serverSyncMgr, m_serverRoot, m_client2SyncMgr, m_client2Root);

		if (m_bConnectOnSight)
		{
			ValidatePair(m_client1SyncMgr, m_client1Root, m_onsight1SyncMgr, m_onsight1Root);
			ValidatePair(m_client2SyncMgr, m_client2Root, m_onsight2SyncMgr, m_onsight2Root);
		}
	}

	void SyncTestbed::ValidatePair(const Sync::SyncManagerPtr& syncMgr1, const SyncTestObjectPtr& syncObj1, const Sync::SyncManagerPtr& syncMgr2, const SyncTestObjectPtr& syncObj2) const
	{
		if (!syncObj1->Equals(syncObj2))
		{
			WriteLogEntry("===========================================\n");
			syncMgr1->PrintSyncData();

			WriteLogEntry("\n===========================================\n");
			syncMgr2->PrintSyncData();

			Assert::Fail(L"Sync pair does not match");
		}
	}

	void SyncTestbed::CompareElementRecurs(const ElementPtr& element1, const ElementPtr& element2) const
	{
		Assert::AreEqual(element1->GetName()->GetString(), element2->GetName()->GetString());
		Assert::IsTrue(element1->GetGUID() == element2->GetGUID());
		Assert::IsTrue(element1->GetElementType() == element2->GetElementType());
		Assert::IsTrue(element1->GetXValue() == element2->GetXValue());

		if (element1->GetElementType() == ElementType::ObjectType)
		{
			ObjectElementPtr objElement1 = ObjectElement::Cast(element1);
			Assert::IsNotNull(objElement1.get());
			ObjectElementPtr objElement2 = ObjectElement::Cast(element2);
			Assert::IsNotNull(objElement2.get());

			Assert::AreEqual(objElement1->GetElementCount(), objElement2->GetElementCount());

			for (int32 i = 0; i < objElement1->GetElementCount(); ++i)
			{
				ElementPtr childElement1 = objElement1->GetElementAt(i);

				ElementPtr childElement2 = objElement2->GetElement(childElement1->GetName());
				Assert::IsNotNull(childElement2.get());

				CompareElementRecurs(childElement1, childElement2);
			}
		}
		else if (IsFrom<ArrayElement>(element1))
		{
			ArrayElement* arrayElement1 = reflection_cast<ArrayElement>(element1);
			Assert::IsNotNull(arrayElement1);

			ArrayElement* arrayElement2 = reflection_cast<ArrayElement>(element2);
			Assert::IsNotNull(arrayElement2);

			Assert::AreEqual(arrayElement1->GetCount(), arrayElement2->GetCount());

			for (int32 i = 0; i < arrayElement1->GetCount(); ++i)
			{
				Assert::IsTrue(arrayElement1->GetXValue(i) == arrayElement2->GetXValue(i));
			}
		}
	}

	void SyncTestbed::WriteLogEntry(const std::string& message) const
	{
		Microsoft::VisualStudio::CppUnitTestFramework::Logger::WriteMessage(message.c_str());
	}
}
