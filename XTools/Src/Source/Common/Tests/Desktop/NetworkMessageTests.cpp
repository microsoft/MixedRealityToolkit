#include "stdafx.h"
#include "../Common/Private/NetworkConnectionImpl.h"
#include "../Common/Private/NetworkInMessageImpl.h"
#include "../Common/Private/NetworkOutMessageImpl.h"
#include "../Common/Private/Buffer.h"
#include "../Common/Private/BufferQueue.h"
#include "../Common/Private/Utils/LFQueue.h"


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
	TEST_CLASS(NetworkMessageTests)
	{
	public:
		
		TEST_METHOD(TestReadWrite)
		{
			const XTools::byte		valueByte		= 123;
			const XTools::int16		valueInt16		= -12345;
			const XTools::int32		valueInt32		= -1073741824;
			const XTools::int64		valueInt64		= -4611686018427387904l;
			const float				valueFloat		= 1234.5678f;
			const double			valueDouble		= 1234567.8901234;
			const std::string		valueString	= "XToolsFTW";
			const XTools::byte		valueArray[5]	= { 1, 45, 78, 204, 129 };

			XTools::NetworkOutMessageImplPtr outMessage = new XTools::NetworkOutMessageImpl();

			outMessage->Write(valueByte);
			outMessage->Write(valueInt16);
			outMessage->Write(valueInt32);
			outMessage->Write(valueInt64);
			outMessage->Write(valueFloat);
			outMessage->Write(valueDouble);
			outMessage->Write(valueString);
			outMessage->WriteArray(valueArray, sizeof(valueArray));

			XTools::byte* tempData = new XTools::byte[outMessage->GetSize()];
			memcpy(tempData, outMessage->GetData(), outMessage->GetSize());

			XTools::NetworkInMessageImpl inMessage(tempData, outMessage->GetSize());

			XTools::byte		readArray[5];

			XTools::byte		readByte	= inMessage.ReadByte();
			XTools::int16		readInt16	= inMessage.ReadInt16();
			XTools::int32		readInt32	= inMessage.ReadInt32();
			XTools::int64		readInt64	= inMessage.ReadInt64();
			float				readFloat	= inMessage.ReadFloat();
			double				readDouble	= inMessage.ReadDouble();
			XTools::XStringPtr	readString	= inMessage.ReadString();
			inMessage.ReadArray(readArray, sizeof(readArray));

			Assert::AreEqual(valueByte, readByte);
			Assert::AreEqual(valueInt16, readInt16);
			Assert::AreEqual(valueInt32, readInt32);
			Assert::AreEqual(valueInt64, readInt64);
			Assert::AreEqual(valueFloat, readFloat);
			Assert::AreEqual(valueDouble, readDouble);
			Assert::AreEqual(valueString, readString->GetString());

			for (size_t i = 0; i < sizeof(readArray); ++i)
			{
				Assert::AreEqual(valueArray[i], readArray[i]);
			}

			delete[] tempData;
		}

		TEST_METHOD(TestCircularBuffer)
		{
			// Edge test
			{
				const uint32 bufferSize1 = 28;
				const uint32 bufferSize2 = 28;

				BufferQueue queue(32);

				WriteBuffer(queue, bufferSize1);
				WriteBuffer(queue, bufferSize2);

				ReadBuffer(queue, bufferSize1);
				ReadBuffer(queue, bufferSize2);
			}
			
			// within test
			{
				const uint32 bufferSize1 = 13;
				const uint32 bufferSize2 = 46;

				BufferQueue queue(256);

				WriteBuffer(queue, bufferSize1);
				WriteBuffer(queue, bufferSize2);

				ReadBuffer(queue, bufferSize1);
				ReadBuffer(queue, bufferSize2);
			}

			// wrap test
			{
				const uint32 bufferSize1 = 13;
				const uint32 bufferSize2 = 20;

				BufferQueue queue(32);

				WriteBuffer(queue, bufferSize1);
				ReadBuffer(queue, bufferSize1);

				WriteBuffer(queue, bufferSize2);
				ReadBuffer(queue, bufferSize2);
			}

			// large message test
			{
				const uint32 bufferSize1 = 13;
				const uint32 bufferSize2 = 1024;
				const uint32 bufferSize3 = 27;

				BufferQueue queue(32);

				WriteBuffer(queue, bufferSize1);
				WriteBuffer(queue, bufferSize2);
				WriteBuffer(queue, bufferSize3);

				ReadBuffer(queue, bufferSize1);
				ReadBuffer(queue, bufferSize2);
				ReadBuffer(queue, bufferSize3);
			}

			// Header Edge test
			{
				const uint32 bufferSize1 = 24;
				const uint32 bufferSize2 = 28;

				BufferQueue queue(32);

				WriteBuffer(queue, bufferSize1);
				WriteBuffer(queue, bufferSize2);

				ReadBuffer(queue, bufferSize1);
				ReadBuffer(queue, bufferSize2);
			}

			{
				const uint32 bufferSize1 = 1400;
				const uint32 bufferSize2 = 1954;
				const uint32 bufferSize3 = 86;

				BufferQueue queue(2048);

				WriteBuffer(queue, bufferSize1);
				ReadBuffer(queue, bufferSize1);

				WriteBuffer(queue, bufferSize2);
				WriteBuffer(queue, bufferSize3);
				
				ReadBuffer(queue, bufferSize2);
				ReadBuffer(queue, bufferSize3);
			}
		}


		class LFQueueTest
		{
		public:
			LFQueueTest(uint32 queueSize, uint32 maxMessageSize)
				: m_queue(queueSize)
				, m_maxMessageSize(maxMessageSize)
			{
				m_threadB = new MemberFuncThread(&LFQueueTest::ConsumerFunc, this);
				m_threadA = new MemberFuncThread(&LFQueueTest::ProducerFunc, this);
				
				// Wait for the threads to finish
				m_threadA->WaitForThreadExit();
				m_threadB->WaitForThreadExit();

				// Check the results
				Assert::AreEqual(m_listA.size(), m_listB.size());

				for (size_t i = 0; i < m_listA.size(); ++i)
				{
					Assert::IsTrue(m_listA[i].Equals(m_listB[i]));
				}
			}

			void ProducerFunc()
			{
				scoped_array<byte> buffer1(new byte[m_maxMessageSize]);

				for (uint32 i = 0; i < m_maxMessageSize; ++i)
				{
					buffer1[i] = (byte)i;
				}

				for (uint32 i = 0; i < m_maxMessageSize; ++i)
				{
					Buffer messageBuffer(i+1);
					messageBuffer.Set(buffer1.get(), i+1);

					while (!m_queue.TryPush(messageBuffer))
					{

					}

					m_listA.push_back(messageBuffer);
				}
			}

			void ConsumerFunc()
			{
				Buffer incomingBuffer(m_maxMessageSize);

				for (;;)
				{
					if (m_queue.TryPop(incomingBuffer))
					{
						m_listB.push_back(incomingBuffer);

						if (incomingBuffer.GetSize() == m_maxMessageSize)
						{
							break;
						}
					}
				}
			}

		private:
			LFQueue m_queue;
			MemberFuncThreadPtr m_threadA;
			MemberFuncThreadPtr m_threadB;
			std::vector<Buffer> m_listA;
			std::vector<Buffer> m_listB;
			uint32				m_maxMessageSize;
		};


		TEST_METHOD(TestLFQueue)
		{
			// Test Writing too little or too much
			{
				const uint32 bufferSize = 16;

				LFQueue queue(32);
				Buffer testBuffer(16);

				// Pop from an empty queue
				bool popResult = queue.TryPop(testBuffer);
				Assert::IsFalse(popResult);

				byte buffer[bufferSize];

				// Push data that will fit
				bool pushResult = queue.TryPush(buffer, sizeof(buffer));
				Assert::IsTrue(pushResult);

				// Push data that will not fit
				pushResult = queue.TryPush(buffer, sizeof(buffer));
				Assert::IsFalse(pushResult);

				// Pop the queued data
				popResult = queue.TryPop(testBuffer);
				Assert::IsTrue(popResult);
				Assert::AreEqual(queue.GetSize(), 0u);
			}

			// within test
			{
				const uint32 bufferSize1 = 13;
				const uint32 bufferSize2 = 46;

				LFQueue queue(256);

				WriteBuffer(queue, bufferSize1);
				WriteBuffer(queue, bufferSize2);

				ReadBuffer(queue, bufferSize1);
				ReadBuffer(queue, bufferSize2);
			}

			// wrap test
			{
				const uint32 bufferSize1 = 13;
				const uint32 bufferSize2 = 20;

				LFQueue queue(32);

				WriteBuffer(queue, bufferSize1);
				ReadBuffer(queue, bufferSize1);

				WriteBuffer(queue, bufferSize2);
				ReadBuffer(queue, bufferSize2);
			}

			{
				const uint32 bufferSize1 = 1400;
				const uint32 bufferSize2 = 1954;
				const uint32 bufferSize3 = 85;

				LFQueue queue(2048);

				WriteBuffer(queue, bufferSize1);
				ReadBuffer(queue, bufferSize1);

				WriteBuffer(queue, bufferSize2);
				WriteBuffer(queue, bufferSize3);

				ReadBuffer(queue, bufferSize2);
				ReadBuffer(queue, bufferSize3);
			}

			// Threading test
			{
				LFQueueTest asyncTest(2048, 2043);
			}
		}


		void WriteBuffer(BufferQueue& queue, uint32 sizeToWrite)
		{
			uint32 originalSize = queue.GetUsedSize();

			scoped_array<byte> buffer1(new byte[sizeToWrite]);

			for (uint32 i = 0; i < sizeToWrite; ++i)
			{
				buffer1[i] = (byte)i;
			}

			queue.Push(buffer1.get(), sizeToWrite);

			Assert::IsTrue(queue.GetUsedSize() == originalSize + sizeToWrite + 4);
		}

		
		void ReadBuffer(BufferQueue& queue, uint32 expectedSize)
		{
			Buffer readBuffer(13);

			queue.Pop(readBuffer);
			Assert::IsTrue(readBuffer.GetSize() == expectedSize);

			const byte* data = readBuffer.GetData();

			for (uint32 i = 0; i < readBuffer.GetSize(); ++i)
			{
				Assert::IsTrue(data[i] == (byte)i);
			}
		}

		void WriteBuffer(LFQueue& queue, uint32 sizeToWrite)
		{
			uint32 originalSize = queue.GetSize();

			scoped_array<byte> buffer1(new byte[sizeToWrite]);

			for (uint32 i = 0; i < sizeToWrite; ++i)
			{
				buffer1[i] = (byte)i;
			}

			bool pushResult = queue.TryPush(buffer1.get(), sizeToWrite);

			Assert::IsTrue(pushResult);
			Assert::IsTrue(queue.GetSize() == originalSize + sizeToWrite + 4);
		}


		void ReadBuffer(LFQueue& queue, uint32 expectedSize)
		{
			Buffer readBuffer(13);

			bool popResult = queue.TryPop(readBuffer);
			Assert::IsTrue(popResult);
			Assert::IsTrue(readBuffer.GetSize() == expectedSize);

			const byte* data = readBuffer.GetData();

			for (uint32 i = 0; i < readBuffer.GetSize(); ++i)
			{
				Assert::IsTrue(data[i] == (byte)i);
			}
		}
	};
}