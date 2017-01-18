//------------------------------------------------------------------------------
// <copyright file="Utilities.cpp" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

#include "stdafx.h"
#include "Utilities.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

// Disable warning about LINE_INFO() "nonstandard extension used : class rvalue used as lvalue"
#pragma warning( disable : 4238 )

namespace MicrosoftTest
{
	void WaitForCompletion(std::function<bool()> hasCompleted, std::function<void()> update, DWORD timeout)
	{
		DWORD startTime = GetTickCount();

		while (!hasCompleted())
		{
			Assert::IsTrue((GetTickCount() - startTime) <= timeout, L"Timed out waiting for the operation to complete.", LINE_INFO());

			Sleep(0);
			update();
		}
	}

	void LogMessageFormat(const char *format, ...)
	{
		char buffer[512];

		{
			va_list ap;
			va_start(ap, format);
			vsprintf_s(buffer, sizeof(buffer), format, ap);
			va_end(ap);
		}

		Microsoft::VisualStudio::CppUnitTestFramework::Logger::WriteMessage(buffer);
	}
}