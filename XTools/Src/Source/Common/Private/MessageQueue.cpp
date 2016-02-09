//////////////////////////////////////////////////////////////////////////
// MessageQueue.cpp
//
// Single buffer that holds a queue of network messages to process.
// Avoids making frequent allocations and is thread safe
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MessageQueue.h"
#include "PacketWrapper.h"

XTOOLS_NAMESPACE_BEGIN

// Allocate 3 mb for the message buffer. Messages bigger than this will be split
static const uint32 kMessageQueueSize = 3 * (1024 * 1024);

// The maximum number of message to process from a single peer connection per update.  
static const uint32 kMaxIncomingMessages = 100;

//////////////////////////////////////////////////////////////////////////
struct MessageQueue::MessageHeader
{
	RakNet::SystemAddress	m_address;
	RakNet::RakNetGUID		m_raknetGuid;
	PeerID					m_peerID;
	uint32					m_payloadSize;
};

//////////////////////////////////////////////////////////////////////////
struct MessageQueue::PeerSettings
{
	PeerPtr								m_peer;
	std::vector<MessageInterceptorPtr>	m_interceptors;
};

//////////////////////////////////////////////////////////////////////////
MessageQueue::MessageQueue()
: m_stopping(0)
, m_messageQueue(kMessageQueueSize)
, m_messageBufferIn(kMessageQueueSize)
, m_messageBufferOut(kMessageQueueSize)
{	
	// Start a thread to run the update loop. 
	m_networkThread = new MemberFuncThread(&MessageQueue::ThreadFunc, this);
}


MessageQueue::~MessageQueue()
{
	// trigger the thread exit and wait for it...
	m_stopping = 1;
	m_networkThread->WaitForThreadExit();

	XTASSERT(m_peers.empty());
}


void MessageQueue::AddConnection(const PeerPtr& peer)
{
	ScopedLock lock(m_mutex);

	PeerSettings settings;
	settings.m_peer = peer;
	m_peers.push_back(settings);
}


void MessageQueue::RemoveConnection(const PeerPtr& peer)
{
	ScopedLock lock(m_mutex);

	for (size_t i = 0; i < m_peers.size(); ++i)
	{
		if (m_peers[i].m_peer == peer)
		{
			m_peers.erase(m_peers.begin() + i);
			break;
		}
	}
}


void MessageQueue::AddMessageInterceptor(const MessageInterceptorPtr& interceptor)
{
	ScopedLock lock(m_mutex);

	for (size_t i = 0; i < m_peers.size(); ++i)
	{
		if (m_peers[i].m_peer == interceptor->GetPeer())
		{
			m_peers[i].m_interceptors.push_back(interceptor);
			break;
		}
	}
}


void MessageQueue::RemoveMessageInterceptor(const MessageInterceptorPtr& interceptor)
{
	ScopedLock lock(m_mutex);

	for (size_t peerIndex = 0; peerIndex < m_peers.size(); ++peerIndex)
	{
		if (m_peers[peerIndex].m_peer == interceptor->GetPeer())
		{
			std::vector<MessageInterceptorPtr>& interceptors = m_peers[peerIndex].m_interceptors;

			for (size_t interceptorIndex = 0; interceptorIndex < interceptors.size(); ++interceptorIndex)
			{
				if (interceptors[interceptorIndex] == interceptor)
				{
					interceptors.erase(interceptors.begin() + interceptorIndex);
					return;
				}
			}
		}
	}
}


bool MessageQueue::TryGetMessage(Message& msg)
{
	auto startTime = std::chrono::high_resolution_clock::now();

	bool result = false;
	if (m_messageQueue.TryPop(m_messageBufferOut))
	{
		MessageHeader* header = reinterpret_cast<MessageHeader*>(m_messageBufferOut.GetData());
		msg.m_address		= header->m_address;
		msg.m_rakNetGuid	= header->m_raknetGuid;
		msg.m_peerID		= header->m_peerID;

		const uint32 payloadSize		= header->m_payloadSize;
		const uint32 messagePayloadSize = m_messageBufferOut.GetSize() - sizeof(MessageHeader);
		XTASSERT(payloadSize >= messagePayloadSize);

		msg.m_payload.Reset(payloadSize);
		msg.m_payload.Append(m_messageBufferOut.GetData() + sizeof(MessageHeader), messagePayloadSize);

		// The packet may be broken into several pops if it is bigger than the queue,
		// so keep popping and appending it to the final message
		uint32 accumulatedPacket = messagePayloadSize;
		while (accumulatedPacket < payloadSize)
		{
			if (m_messageQueue.TryPop(m_messageBufferOut))
			{
				XTASSERT(m_messageBufferOut.GetSize() + accumulatedPacket <= payloadSize);

				msg.m_payload.Append(m_messageBufferOut.GetData(), m_messageBufferOut.GetSize());
				accumulatedPacket += m_messageBufferOut.GetSize();
			}
		} 

		XTASSERT(accumulatedPacket == payloadSize);

		result = true;
	}

	auto endTime = std::chrono::high_resolution_clock::now();

	uint64 timeDelta = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();

	if (timeDelta > 5)
	{
		LogWarning("MessageQueue::TryGetMessage() stall: %d ms", timeDelta);
	}

	return result;
}


void MessageQueue::ProcessMessages()
{
	ScopedLock lock(m_mutex);

	// Send the messages that did not fit in the queue last update
	while (!m_backupQueue.empty())
	{
		std::shared_ptr<Buffer> buffer = m_backupQueue.front();
		if (m_messageQueue.TryPush(*buffer))
		{
			// Message put on the lfqueue, clear it from the backup buffer
			m_backupQueue.pop();
		}
		else
		{
			// Buffer is still full.  Exit the function and try again later after the main thread has had
			// more time to consume the packets.  
			return;
		}
	}

	const uint32 maxPushSize = m_messageQueue.GetAllocatedSize() - LFQueue::kElementOverhead - 1;

	bool bQueueFull = false;

	uint32 messageProcessed = 0;

	for (size_t peerIndex = 0; peerIndex < m_peers.size(); ++peerIndex)
	{
		PeerPtr peer = m_peers[peerIndex].m_peer;

		uint32 peerMessagesProcessed = 0;

		// Loop through all the packets that have arrived on this peer since the last update.
		for (PacketWrapper packet(peer, peer->Receive()); packet.IsValid() && peerMessagesProcessed < kMaxIncomingMessages; packet = peer->Receive())
		{
			// Don't push empty packets.  This shouldn't happen anyway
			if (packet->length > 0)
			{
				// First check to see if any of the interceptors want to handle it
				if (!InterceptPacket(packet.get(), m_peers[peerIndex].m_interceptors))
				{
					// Ok, lets push this message into the queue to be consumed later on the main thread
					const uint32 totalMessageSize = sizeof(MessageHeader) + packet->length;
					uint32 initialSize = std::min(maxPushSize, totalMessageSize);
					uint32 initialPayloadSize = initialSize - sizeof(MessageHeader);

					m_messageBufferIn.Reset(initialSize);

					// Push the header on the queue
					MessageHeader header;
					header.m_address = packet->systemAddress;
					header.m_raknetGuid = packet->guid;
					header.m_peerID = (*peer).GetPeerID();
					header.m_payloadSize = packet->length;

					m_messageBufferIn.Append(reinterpret_cast<const byte*>(&header), sizeof(MessageHeader));
					m_messageBufferIn.Append(packet->data, initialPayloadSize);

					if (bQueueFull || !m_messageQueue.TryPush(m_messageBufferIn))
					{
						// The queue is too full to push this packet; store it on the backup queue to be sent later
						m_backupQueue.push(std::shared_ptr<Buffer>(new Buffer(m_messageBufferIn)));
						bQueueFull = true;
					}

					uint32 remainingPayloadSize = packet->length - initialPayloadSize;
					uint32 packetOffset = initialPayloadSize;

					// If the packet is too big it is split up.  This way we avoid complicated allocations with the LFQueue
					while (remainingPayloadSize > 0)
					{
						uint32 copySize = std::min(remainingPayloadSize, maxPushSize);

						if (bQueueFull || !m_messageQueue.TryPush(packet->data + packetOffset, copySize))
						{
							// The queue is too full to push this packet; store it on the backup queue to be sent later
							m_backupQueue.push(std::shared_ptr<Buffer>(new Buffer(packet->data + packetOffset, copySize)));
							bQueueFull = true;
						}

						remainingPayloadSize -= copySize;
						packetOffset += copySize;
					}

					XTASSERT(packetOffset == packet->length);
				}

				++peerMessagesProcessed;
				++messageProcessed;
			}
		}
	}
}


bool MessageQueue::InterceptPacket(RakNet::Packet* packet, std::vector<MessageInterceptorPtr>& interceptors)
{
	for (size_t interceptorIndex = 0; interceptorIndex < interceptors.size(); ++interceptorIndex)
	{
		if (interceptors[interceptorIndex]->OnReceive(packet))
		{
			return true;
		}
	}

	return false;
}


void MessageQueue::ThreadFunc()
{
	while (m_stopping == 0)
	{
		ProcessMessages();
		Platform::SleepMS(10);
	}
}

XTOOLS_NAMESPACE_END
