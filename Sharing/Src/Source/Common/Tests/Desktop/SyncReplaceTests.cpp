//////////////////////////////////////////////////////////////////////////
// SyncReplaceTests.cpp
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
	TEST_CLASS(SyncReplaceTests)
	{
	public:

		TEST_METHOD(TestSync_ReplaceObjectSimple)
		{
			SyncTestbedPtr testBed = new SyncTestbed(true);

			for (int i = 0; i < 10; ++i)
			{
				DWORD startTime = GetTickCount();
				testBed->ResetTestBed();

				XStringPtr childName = new XString("TempChild");

				testBed->m_onsight1Root->GetElement()->CreateObjectElement(childName, new XString("foo"));
				testBed->m_onsight2Root->GetElement()->CreateObjectElement(childName, new XString("bar"));

				testBed->WaitTillFullySynced();

				testBed->ValidateSyncState();
				MicrosoftTest::LogMessageFormat("Loop %d took %dms\r\n", i, GetTickCount() - startTime);
			}
		}

		TEST_METHOD(TestSync_ReplaceObjectCompound)
		{
			SyncTestbedPtr testBed = new SyncTestbed(true);

			for (int i = 0; i < 10; ++i)
			{
				DWORD startTime = GetTickCount();
				testBed->ResetTestBed();

				XStringPtr childName = new XString("FirstChild");
				XStringPtr childName2 = new XString("SecondChild");

				ObjectElementPtr parentObj1 = testBed->m_onsight1Root->GetElement()->CreateObjectElement(childName, new XString("foo"));
				FloatElementPtr floatChild1 = parentObj1->CreateFloatElement(childName2, 25.f);
				floatChild1->SetValue(24.f);


				ObjectElementPtr parentObj2 = testBed->m_onsight2Root->GetElement()->CreateObjectElement(childName, new XString("bar"));
				IntElementPtr intChild1 = parentObj2->CreateIntElement(childName2, 36);
				intChild1->SetValue(37);

				testBed->WaitTillFullySynced();

				testBed->ValidateSyncState();
				MicrosoftTest::LogMessageFormat("Loop %d took %dms\r\n", i, GetTickCount() - startTime);
			}
		}

		TEST_METHOD(TestSync_ReplaceObjectDelete)
		{
			SyncTestbedPtr testBed = new SyncTestbed(true);

			for (int i = 0; i < 10; ++i)
			{
				DWORD startTime = GetTickCount();
				testBed->ResetTestBed();

				XStringPtr childName = new XString("FirstChild");
				XStringPtr childName2 = new XString("SecondChild");

				ObjectElementPtr parentObj1 = testBed->m_onsight1Root->GetElement()->CreateObjectElement(childName, new XString("foo"));
				FloatElementPtr floatChild1 = parentObj1->CreateFloatElement(childName2, 25.f);
				floatChild1->SetValue(24.f);
				testBed->m_onsight1Root->GetElement()->RemoveElement(parentObj1);


				ObjectElementPtr parentObj2 = testBed->m_onsight2Root->GetElement()->CreateObjectElement(childName, new XString("bar"));
				IntElementPtr intChild1 = parentObj2->CreateIntElement(childName2, 36);
				intChild1->SetValue(37);

				testBed->WaitTillFullySynced();

				testBed->ValidateSyncState();
				MicrosoftTest::LogMessageFormat("Loop %d took %dms\r\n", i, GetTickCount() - startTime);
			}
		}

		TEST_METHOD(TestSync_ReplacePrimitiveSimple)
		{
			SyncTestbedPtr testBed = new SyncTestbed(true);

			for (int i = 0; i < 10; ++i)
			{
				DWORD startTime = GetTickCount();
				testBed->ResetTestBed();

				XStringPtr childName = new XString("TempChild");

				testBed->m_onsight1Root->GetElement()->CreateFloatElement(childName, 25.f);
				testBed->m_onsight2Root->GetElement()->CreateIntElement(childName, 36);

				testBed->WaitTillFullySynced();

				testBed->ValidateSyncState();
				MicrosoftTest::LogMessageFormat("Loop %d took %dms\r\n", i, GetTickCount() - startTime);
			}
		}
	};
}