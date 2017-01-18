//////////////////////////////////////////////////////////////////////////
// SyncArrayTests.cpp
//
// Unit tests for the Sync system's array element operations
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
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

		TEST_METHOD(TestSync_ArrayInsert)
		{
			SyncTestbedPtr testBed = new SyncTestbed(true);

			for (int i = 0; i < 10; ++i)
			{
				DWORD startTime = GetTickCount();
				testBed->ResetTestBed();

				std::string childName1("TempChild1");
				SyncTestObjectPtr testObj = testBed->m_onsight1Root->AddChild(childName1);

				testObj->InsertIntArrayValue(0, 0);
				testObj->InsertIntArrayValue(1, 1);
				testObj->InsertIntArrayValue(2, 2);
				testObj->InsertIntArrayValue(3, 3);
				testObj->InsertIntArrayValue(4, 4);

				testBed->WaitTillFullySynced();
				testBed->ValidateSyncState();

				MicrosoftTest::LogMessageFormat("Loop %d took %dms\r\n", i, GetTickCount() - startTime);
			}

			for (int i = 0; i < 10; ++i)
			{
				DWORD startTime = GetTickCount();
				testBed->ResetTestBed();

				std::string childName1("TempChild1");
				SyncTestObjectPtr testObj = testBed->m_onsight1Root->AddChild(childName1);

				testObj->InsertFloatArrayValue(0, 0.f);
				testObj->InsertFloatArrayValue(1, 1.f);
				testObj->InsertFloatArrayValue(2, 2.f);
				testObj->InsertFloatArrayValue(3, 3.f);
				testObj->InsertFloatArrayValue(4, 4.f);

				testBed->WaitTillFullySynced();
				testBed->ValidateSyncState();

				MicrosoftTest::LogMessageFormat("Loop %d took %dms\r\n", i, GetTickCount() - startTime);
			}

			for (int i = 0; i < 10; ++i)
			{
				DWORD startTime = GetTickCount();
				testBed->ResetTestBed();

				std::string childName1("TempChild1");
				SyncTestObjectPtr testObj = testBed->m_onsight1Root->AddChild(childName1);

				testObj->InsertStringArrayValue(0, new XString("foo"));
				testObj->InsertStringArrayValue(1, new XString("bar"));
				testObj->InsertStringArrayValue(2, new XString("baz"));
				testObj->InsertStringArrayValue(3, new XString("biz"));
				testObj->InsertStringArrayValue(4, new XString("booz"));

				testBed->WaitTillFullySynced();
				testBed->ValidateSyncState();

				MicrosoftTest::LogMessageFormat("Loop %d took %dms\r\n", i, GetTickCount() - startTime);
			}
		}

		TEST_METHOD(TestSync_ArrayMerge)
		{
			SyncTestbedPtr testBed = new SyncTestbed(true);

			for (int i = 0; i < 10; ++i)
			{
				DWORD startTime = GetTickCount();
				testBed->ResetTestBed();

				std::string childName1("TempChild1");
				SyncTestObjectPtr testObj1 = testBed->m_onsight1Root->AddChild(childName1);
				SyncTestObjectPtr testObj2 = testBed->m_onsight2Root->AddChild(childName1);

				testObj1->InsertIntArrayValue(0, 0);
				testObj1->InsertIntArrayValue(1, 1);
				testObj1->InsertIntArrayValue(2, 2);
				testObj2->InsertIntArrayValue(0, 3);
				testObj2->InsertIntArrayValue(1, 4);
				testObj2->InsertIntArrayValue(2, 5);

				testBed->WaitTillFullySynced();
				testBed->ValidateSyncState();

				testObj1->SetIntArrayValue(1, 7);
				testObj1->RemoveIntArrayValue(0);
				testObj2->RemoveIntArrayValue(1);
				testObj2->SetIntArrayValue(3, 45);

				testBed->WaitTillFullySynced();
				testBed->ValidateSyncState();

				MicrosoftTest::LogMessageFormat("Loop %d took %dms\r\n", i, GetTickCount() - startTime);
			}

			for (int i = 0; i < 10; ++i)
			{
				DWORD startTime = GetTickCount();
				testBed->ResetTestBed();

				std::string childName1("TempChild1");
				SyncTestObjectPtr testObj1 = testBed->m_onsight1Root->AddChild(childName1);
				SyncTestObjectPtr testObj2 = testBed->m_onsight2Root->AddChild(childName1);

				testObj1->InsertFloatArrayValue(0, 0);
				testObj1->InsertFloatArrayValue(1, 1);
				testObj1->InsertFloatArrayValue(2, 2);
				testObj2->InsertFloatArrayValue(0, 3);
				testObj2->InsertFloatArrayValue(1, 4);
				testObj2->InsertFloatArrayValue(2, 5);

				testBed->WaitTillFullySynced();
				testBed->ValidateSyncState();

				testObj1->SetFloatArrayValue(1, 7);
				testObj1->RemoveFloatArrayValue(0);
				testObj2->RemoveFloatArrayValue(1);
				testObj2->SetFloatArrayValue(3, 45);

				testBed->WaitTillFullySynced();
				testBed->ValidateSyncState();

				MicrosoftTest::LogMessageFormat("Loop %d took %dms\r\n", i, GetTickCount() - startTime);
			}

			for (int i = 0; i < 10; ++i)
			{
				DWORD startTime = GetTickCount();
				testBed->ResetTestBed();

				std::string childName1("TempChild1");
				SyncTestObjectPtr testObj1 = testBed->m_onsight1Root->AddChild(childName1);
				SyncTestObjectPtr testObj2 = testBed->m_onsight2Root->AddChild(childName1);

				testObj1->InsertStringArrayValue(0, new XString("foo"));
				testObj1->InsertStringArrayValue(1, new XString("bar"));
				testObj1->InsertStringArrayValue(2, new XString("baz"));
				testObj2->InsertStringArrayValue(0, new XString("biz"));
				testObj2->InsertStringArrayValue(1, new XString("boz"));
				testObj2->InsertStringArrayValue(2, new XString("fuz"));

				testBed->WaitTillFullySynced();
				testBed->ValidateSyncState();

				testObj1->SetStringArrayValue(1, new XString("bleach"));
				testObj1->RemoveStringArrayValue(0);
				testObj2->RemoveStringArrayValue(1);
				testObj2->SetStringArrayValue(3, new XString("Blagh"));

				testBed->WaitTillFullySynced();
				testBed->ValidateSyncState();

				MicrosoftTest::LogMessageFormat("Loop %d took %dms\r\n", i, GetTickCount() - startTime);
			}
		}
	};
}
