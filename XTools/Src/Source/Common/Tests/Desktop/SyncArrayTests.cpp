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
		}
	};
}
