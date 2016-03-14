//////////////////////////////////////////////////////////////////////////
// SyncDeleteTests.cpp
//
// Unit tests for the Sync system's delete operation
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SyncTestbed.h"
#include <Tests/Utilities.h>

using namespace XTools;
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

	TEST_CLASS(SyncDeleteTests)
	{
	public:

		TEST_METHOD(TestSync_DeleteSimple)
		{
			SyncTestbedPtr testBed = new SyncTestbed(true);

			for (int i = 0; i < 10; ++i)
			{
				DWORD startTime = GetTickCount();
				testBed->ResetTestBed();

				std::string childName("TempChild");

				testBed->m_onsight1Root->AddChild(childName);

				testBed->WaitTillFullySynced();

				testBed->m_onsight2Root->RemoveChild(childName);

				testBed->WaitTillFullySynced();

				testBed->ValidateSyncState();
				MicrosoftTest::LogMessageFormat("Loop %d took %dms\r\n", i, GetTickCount() - startTime);
			}
		}

		TEST_METHOD(TestSync_DeleteCreateModify)
		{
			SyncTestbedPtr testBed = new SyncTestbed(true);

			for (int i = 0; i < 10; ++i)
			{
				DWORD startTime = GetTickCount();
				testBed->ResetTestBed();

				std::string childName("TempChild");

				testBed->m_onsight1Root->AddChild(childName);

				testBed->WaitTillFullySynced();

				// User 1 deletes the element, User 2 deletes then re-adds it
				testBed->m_onsight2Root->RemoveChild(childName);

				testBed->m_onsight1Root->RemoveChild(childName);
				testBed->m_onsight1Root->AddChild(childName);
				testBed->m_onsight1Root->GetChild(childName)->SetFloatValue(128.f);

				testBed->WaitTillFullySynced();
				testBed->ValidateSyncState();

				// NOTE: The deletes should cancel each other out, making way for the add
				Assert::IsTrue(testBed->m_onsight1Root->GetChild(childName) != NULL);
				MicrosoftTest::LogMessageFormat("Loop %d took %dms\r\n", i, GetTickCount() - startTime);
			}
		}

		TEST_METHOD(TestSync_DeleteCreateChild)
		{
			SyncTestbedPtr testBed = new SyncTestbed(true);

			for (int i = 0; i < 10; ++i)
			{
				DWORD startTime = GetTickCount();
				testBed->ResetTestBed();

				std::string childName("TempChild");
				std::string childName2("TempChild2");

				testBed->m_onsight1Root->AddChild(childName);

				testBed->WaitTillFullySynced();

				// User 1 deletes the element, User 2 adds a child to it
				testBed->m_onsight1Root->RemoveChild(childName);

				testBed->m_onsight2Root->GetChild(childName)->AddChild("TempChild2");

				testBed->WaitTillFullySynced();
				testBed->ValidateSyncState();

				// Result: The child should have been deleted when the parent was deleted
				Assert::IsTrue(testBed->m_onsight2Root->GetChild(childName) == NULL);
				MicrosoftTest::LogMessageFormat("Loop %d took %dms\r\n", i, GetTickCount() - startTime);
			}
		}

		TEST_METHOD(TestSync_DeleteModifyChild)
		{
			SyncTestbedPtr testBed = new SyncTestbed(true);

			for (int i = 0; i < 10; ++i)
			{
				DWORD startTime = GetTickCount();
				testBed->ResetTestBed();

				std::string childName("TempChild");
				std::string childName2("TempChild2");

				testBed->m_onsight1Root->AddChild(childName);

				testBed->WaitTillFullySynced();

				// User 1 deletes the element, User 2 modifies a child of it
				testBed->m_onsight1Root->RemoveChild(childName);

				testBed->m_onsight2Root->GetChild(childName)->SetFloatValue(255.f);

				testBed->WaitTillFullySynced();
				testBed->ValidateSyncState();

				// Result: The child should have been deleted when the parent was deleted
				Assert::IsTrue(testBed->m_onsight2Root->GetChild(childName) == NULL);
				MicrosoftTest::LogMessageFormat("Loop %d took %dms\r\n", i, GetTickCount() - startTime);
			}
		}

		TEST_METHOD(TestSync_DeleteModify)
		{
			SyncTestbedPtr testBed = new SyncTestbed(true);

			for (int i = 0; i < 10; ++i)
			{
				DWORD startTime = GetTickCount();
				testBed->ResetTestBed();

				std::string childName("TempChild");

				testBed->m_onsight1Root->AddChild(childName);

				testBed->WaitTillFullySynced();

				// User 1 deletes the element, User 2 deletes it
				testBed->m_onsight1Root->GetChild(childName)->SetFloatValue(128.f);

				testBed->m_onsight2Root->GetChild(childName)->RemoveFloatValue();

				testBed->WaitTillFullySynced();
				testBed->ValidateSyncState();

				// NOTE: Conflicting adds and deletes will cause the create to always fail
				Assert::IsTrue(testBed->m_onsight1Root->GetChild(childName) != NULL);
				Assert::IsTrue(testBed->m_onsight1Root->GetChild(childName)->IsFloatValid() == false);
				MicrosoftTest::LogMessageFormat("Loop %d took %dms\r\n", i, GetTickCount() - startTime);
			}
		}

		TEST_METHOD(TestSync_ModifyModifyDelete)
		{
			SyncTestbedPtr testBed = new SyncTestbed(true);

			for (int i = 0; i < 10; ++i)
			{
				DWORD startTime = GetTickCount();
				testBed->ResetTestBed();

				std::string childName("TempChild");

				testBed->m_onsight1Root->AddChild(childName);

				testBed->WaitTillFullySynced();

				// User 1 deletes the element, User 2 deletes it
				testBed->m_onsight1Root->GetChild(childName)->SetFloatValue(128.f);

				testBed->m_onsight2Root->GetChild(childName)->SetFloatValue(256.f);
				testBed->m_onsight2Root->GetChild(childName)->RemoveFloatValue();

				testBed->WaitTillFullySynced();
				testBed->ValidateSyncState();

				// NOTE: Conflicting adds and deletes will cause the create to always fail
				Assert::IsTrue(testBed->m_onsight1Root->GetChild(childName) != NULL);
				Assert::IsTrue(testBed->m_onsight1Root->GetChild(childName)->IsFloatValid() == false);

				// The modify op will be invalidated by the subsequent delete, so the user code will never get the modify callback
				Assert::IsTrue(testBed->m_onsight1Root->GetChild(childName)->GetIncomingFloatChanges() == 0);
				Assert::IsTrue(testBed->m_onsight1Root->GetChild(childName)->GetIncomingRemoves() == 1);

				// OnSight1's modify should have been invalidated before it gets to OnSight2
				Assert::IsTrue(testBed->m_onsight2Root->GetChild(childName)->GetIncomingFloatChanges() == 0);
				Assert::IsTrue(testBed->m_onsight2Root->GetChild(childName)->GetIncomingRemoves() == 0);
				MicrosoftTest::LogMessageFormat("Loop %d took %dms\r\n", i, GetTickCount() - startTime);
			}
		}

		TEST_METHOD(TestSync_CreateCreateDelete)
		{
			SyncTestbedPtr testBed = new SyncTestbed(false);

			for (int i = 0; i < 10; ++i)
			{
				DWORD startTime = GetTickCount();
				testBed->ResetTestBed();

				// Wait for the handshake to complete
				testBed->WaitTillFullySynced();

				std::string childName("TempChild");

				//testBed->m_onsight1Root->AddChild(childName);

				//testBed->m_onsight2Root->AddChild(childName);
				//testBed->m_onsight2Root->RemoveChild(childName);

				testBed->m_client1Root->AddChild(childName);

				testBed->m_client2Root->AddChild(childName);
				testBed->m_client2Root->RemoveChild(childName);

				testBed->WaitTillFullySynced();
				testBed->ValidateSyncState();

				// NOTE: Conflicting adds and deletes will cause the create to always fail
				//Assert::IsTrue(testBed->m_onsight1Root->GetChild(childName) == NULL);
				Assert::IsTrue(testBed->m_client1Root->GetChild(childName) == NULL);
				MicrosoftTest::LogMessageFormat("Loop %d took %dms\r\n", i, GetTickCount() - startTime);
			}
		}

		TEST_METHOD(TestSync_ModifyDeleteCreate)
		{
			SyncTestbedPtr testBed = new SyncTestbed(true);

			for (int i = 0; i < 10; ++i)
			{
				DWORD startTime = GetTickCount();
				testBed->ResetTestBed();

				std::string childName("TempChild");

				testBed->m_onsight1Root->AddChild(childName);

				testBed->WaitTillFullySynced();

				// User 1 changes an element, User 2 deletes then re-adds it
				testBed->m_onsight1Root->GetChild(childName)->SetFloatValue(128.f);

				testBed->m_onsight2Root->RemoveChild(childName);
				testBed->m_onsight2Root->AddChild(childName)->SetFloatValue(256);

				testBed->WaitTillFullySynced();
				testBed->ValidateSyncState();

				// NOTE: Conflicting adds and deletes will cause the create to always fail
				Assert::IsTrue(testBed->m_onsight1Root->GetChild(childName) != NULL);
				Assert::IsTrue(testBed->m_onsight1Root->GetChild(childName)->GetFloatValue() == 256.f);
				MicrosoftTest::LogMessageFormat("Loop %d took %dms\r\n", i, GetTickCount() - startTime);
			}
		}

		TEST_METHOD(TestSync_DeleteNestedModify)
		{
			SyncTestbedPtr testBed = new SyncTestbed(true);

			for (int i = 0; i < 10; ++i)
			{
				DWORD startTime = GetTickCount();
				testBed->ResetTestBed();

				std::string childName1("TempChild1");
				std::string childName2("TempChild2");

				testBed->m_onsight1Root->AddChild(childName1)->AddChild(childName2);

				testBed->WaitTillFullySynced();

				// User 1 deletes the element, User 2 deletes then re-adds it
				testBed->m_onsight1Root->GetChild(childName1)->GetChild(childName2)->SetFloatValue(128.f);

				testBed->m_onsight2Root->GetChild(childName1)->RemoveFloatValue();

				testBed->WaitTillFullySynced();
				testBed->ValidateSyncState();

				// NOTE: Conflicting adds and deletes will cause the create to always fail
				Assert::IsTrue(testBed->m_onsight1Root->GetChild(childName1) != NULL);
				Assert::IsTrue(testBed->m_onsight1Root->GetChild(childName1)->IsFloatValid() == false);
				MicrosoftTest::LogMessageFormat("Loop %d took %dms\r\n", i, GetTickCount() - startTime);
			}
		}

		TEST_METHOD(TestSync_DeleteDeleteChild)
		{
			SyncTestbedPtr testBed = new SyncTestbed(true);

			for (int i = 0; i < 10; ++i)
			{
				DWORD startTime = GetTickCount();
				testBed->ResetTestBed();

				std::string childName("TempChild");
				std::string childName2("TempChild2");

				testBed->m_onsight1Root->AddChild(childName)->AddChild(childName2);

				testBed->WaitTillFullySynced();

				// User 1 deletes the element, User 2 adds a child to it
				testBed->m_onsight1Root->RemoveChild(childName);

				testBed->m_onsight2Root->GetChild(childName)->RemoveChild(childName2);

				testBed->WaitTillFullySynced();
				testBed->ValidateSyncState();

				// Result: The child should have been deleted when the parent was deleted
				Assert::IsTrue(testBed->m_onsight2Root->GetChild(childName) == NULL);
				MicrosoftTest::LogMessageFormat("Loop %d took %dms\r\n", i, GetTickCount() - startTime);
			}
		}
	};
}
