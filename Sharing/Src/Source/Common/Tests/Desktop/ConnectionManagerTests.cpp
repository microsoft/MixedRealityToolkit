#include "stdafx.h"
#include <Tests\Utilities.h>

using namespace XTools;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Microsoft {
	namespace VisualStudio {
		namespace CppUnitTestFramework {
			template<> std::wstring ToString<XSocket::Status>(const XSocket::Status& t) { RETURN_WIDE_STRING((int)t); }
		}
	}
}

// Disable warning about LINE_INFO() "nonstandard extension used : class rvalue used as lvalue"
#pragma warning( disable : 4238 )

namespace CommonDesktopTests
{
	class SimpleConnectionListener : public IncomingXSocketListener
	{
	public:
		bool RejectConnection = false;
		XSocketPtr Connection = nullptr;

		void OnNewConnection(const XSocketPtr& newConnection)
		{
			Microsoft::VisualStudio::CppUnitTestFramework::Logger::WriteMessage(L"New Connection!\r\n");

			if (!RejectConnection)
			{
				Connection = newConnection;
			}
			else
			{
				Microsoft::VisualStudio::CppUnitTestFramework::Logger::WriteMessage(L"Ignoring Connection!\r\n");
			}
		}
	};

	TEST_CLASS(NetworkMessageTests)
	{
	public:

		TEST_METHOD(VerifySimpleConnection)
		{
			SimpleConnectionListener *listener = new SimpleConnectionListener();
			XSocketManagerPtr server = XSocketManager::Create();

			Microsoft::VisualStudio::CppUnitTestFramework::Logger::WriteMessage(L"Beginning listening...\r\n");
			ReceiptPtr listenerReceipt = server->AcceptConnections(kSessionServerPort, 2, listener);
			server->Update();

			Assert::IsTrue(nullptr == listener->Connection, L"The listener already has a connection", LINE_INFO());

			XSocketManagerPtr client = XSocketManager::Create();
			XSocketPtr connection = client->OpenConnection("localhost", kSessionServerPort);
			Microsoft::VisualStudio::CppUnitTestFramework::Logger::WriteMessage(L"Client connecting...\r\n");

			MicrosoftTest::WaitForCompletion(
				[&]() {
				return connection->GetStatus() != XSocket::Status::Connecting;
			},
				[&]() {
				server->Update();
				client->Update();
			},
				5000);

			Assert::AreEqual(XSocket::Status::Connected, connection->GetStatus(), L"The connection status was not connected", LINE_INFO());

			DWORD startTicks = GetTickCount();

			MicrosoftTest::WaitForCompletion(
				[&]() {
				return nullptr != listener->Connection;
			},
				[&]() {
				server->Update();
				client->Update();
			},
				5000);

			LPWSTR message = new WCHAR[512];
			wsprintf(message, L"Time to notification: %dms\r\n", GetTickCount() - startTicks);
			Microsoft::VisualStudio::CppUnitTestFramework::Logger::WriteMessage(message);
			Assert::IsTrue(nullptr != listener->Connection, L"The listener never was notified of the connection", LINE_INFO());

			delete listener;
		}

		TEST_METHOD(VerifyServerClosesConnection)
		{
			SimpleConnectionListener *listener = new SimpleConnectionListener();
			listener->RejectConnection = true;
			XSocketManagerPtr server = XSocketManager::Create();

			Microsoft::VisualStudio::CppUnitTestFramework::Logger::WriteMessage(L"Beginning listening...\r\n");
			ReceiptPtr listenerReceipt = server->AcceptConnections(kSessionServerPort, 2, listener);
			server->Update();

			Assert::IsTrue(nullptr == listener->Connection, L"The listener already has a connection", LINE_INFO());

			XSocketManagerPtr client = XSocketManager::Create();
			XSocketPtr connection = client->OpenConnection("localhost", kSessionServerPort);
			Microsoft::VisualStudio::CppUnitTestFramework::Logger::WriteMessage(L"Client connecting...\r\n");

			MicrosoftTest::WaitForCompletion(
				[&]() {
				return connection->GetStatus() != XSocket::Status::Connecting;
			},
				[&]() {
				server->Update();
				client->Update();
			},
				5000);

			Assert::AreEqual(XSocket::Status::Connected, connection->GetStatus(), L"The connection status was not connected", LINE_INFO());

			DWORD startTicks = GetTickCount();

			// Wait for about 5 seconds, make sure the server doesn't do anything bad...
			while (GetTickCount() - startTicks < 5000)
			{
				server->Update();
				client->Update();
			}

			delete listener;
		}
	};
}