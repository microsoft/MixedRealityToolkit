#include "stdafx.h"
#include <Tests/Utilities.h>
#include "../Common/Private/NetworkConnectionImpl.h"
#include "../Common/Private/NetworkInMessageImpl.h"
#include "../Common/Private/NetworkOutMessageImpl.h"
#include "TestLogWriter.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
using namespace XTools;

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
	static const byte kSpeedTestMessageID = XTools::MessageID::UserMessageIDStart;

	//////////////////////////////////////////////////////////////////////////
	class ConnectionListener : public XTools::AtomicRefCounted, public XTools::IncomingXSocketListener
	{
	public:
		virtual void OnNewConnection(const XTools::XSocketPtr& newConnection) XTOVERRIDE
		{
			m_connections.push_back(newConnection);
		}

		std::vector<XTools::XSocketPtr> m_connections;
	};
	DECLARE_PTR(ConnectionListener)

	//////////////////////////////////////////////////////////////////////////
	class EchoResponder : public AtomicRefCounted, public NetworkConnectionListener
	{
	public:
		virtual void OnMessageReceived(const NetworkConnectionPtr& connection, NetworkInMessage& message) XTOVERRIDE
		{
			// Extract the time from the message and send it back
			uint64 time = message.ReadUInt64();

			NetworkOutMessagePtr outMsg = connection->CreateMessage(kSpeedTestMessageID);
			outMsg->Write(time);
			connection->Send(outMsg, Immediate);
		}
	};
	DECLARE_PTR(EchoResponder)

	//////////////////////////////////////////////////////////////////////////
	class RoundTripTester : public AtomicRefCounted, public NetworkConnectionListener
	{
	public:
		RoundTripTester() : m_bInProgress(false) {}

		bool IsPingInProgress() const { return m_bInProgress; }

		uint64 GetAverageDuration() const
		{
			uint64 sum = 0;
			for (size_t i = 0; i < m_durations.size(); ++i)
			{
				sum += m_durations[i];
			}

			return sum / m_durations.size();
		}

		void SendPing(const NetworkConnectionPtr& connection)
		{
			if (!m_bInProgress)
			{
				m_bInProgress = true;

				std::chrono::nanoseconds currentTime = std::chrono::high_resolution_clock::now().time_since_epoch();

				int64 startTime = currentTime.count();

				NetworkOutMessagePtr outMsg = connection->CreateMessage(kSpeedTestMessageID);
				outMsg->Write(startTime);
				connection->Send(outMsg, Immediate);
			}
		}

		virtual void OnMessageReceived(const NetworkConnectionPtr& , NetworkInMessage& message) XTOVERRIDE
		{
			if (m_bInProgress)
			{
				m_bInProgress = false;

				std::chrono::nanoseconds currentTime = std::chrono::high_resolution_clock::now().time_since_epoch();

				// Extract the time from the message
				std::chrono::nanoseconds startTime(message.ReadInt64());

				uint64 delta = std::chrono::duration_cast<std::chrono::nanoseconds>((currentTime - startTime) / 2).count();

				LogInfo("Ping time: %f ms", delta / 1000000.0);

				m_durations.push_back(delta);
			}
		}

	private: 
		std::vector<uint64> m_durations;
		bool m_bInProgress;
	};
	DECLARE_PTR(RoundTripTester)

	//////////////////////////////////////////////////////////////////////////
	TEST_CLASS(NetworkSpeedTests)
	{
	public:

		TEST_METHOD(TestRoundTripLatency)
		{
			TestLogWriter::Init();

			XSocketManagerPtr xsocketMgr1 = XSocketManager::Create();
			XSocketManagerPtr xsocketMgr2 = XSocketManager::Create();

			NetworkConnectionPtr networkConnection1 = new NetworkConnectionImpl(new NetworkMessagePool(10));
			NetworkConnectionPtr networkConnection2 = new NetworkConnectionImpl(new NetworkMessagePool(10));

			ConnectionListenerPtr connectionListener = new ConnectionListener();
			ReceiptPtr listenerReceipt = xsocketMgr1->AcceptConnections(kSessionServerPort, 1, connectionListener.get());

			XSocketPtr connection1 = xsocketMgr2->OpenConnection("localhost", kSessionServerPort);

			MicrosoftTest::WaitForCompletion(
				[&]() {
				return
					connection1->GetStatus() != XSocket::Status::Connecting &&
					connectionListener->m_connections.size() == 1
					;
			},
				[&]() {
				xsocketMgr1->Update();
				xsocketMgr2->Update();
				},
				5000);

			// Set the connections on the NetworkConnection Objects
			networkConnection1->SetSocket(connectionListener->m_connections[0]);
			networkConnection2->SetSocket(connection1);

			EchoResponderPtr responder = new EchoResponder();
			RoundTripTesterPtr roundTripTester = new RoundTripTester();

			networkConnection1->AddListener(kSpeedTestMessageID, responder.get());
			networkConnection2->AddListener(kSpeedTestMessageID, roundTripTester.get());

			for (int i = 0; i < 10; ++i)
			{
				roundTripTester->SendPing(networkConnection2);

				MicrosoftTest::WaitForCompletion(
					[&]() {
					return
						!roundTripTester->IsPingInProgress()
						;
				},
					[&]() {
					xsocketMgr1->Update();
					xsocketMgr2->Update();
				},
					5000);
			}

			uint64 averagePing = roundTripTester->GetAverageDuration();
			Assert::IsTrue(averagePing != 0);
			Assert::IsTrue(averagePing < 1000000);
		}
	};
}

