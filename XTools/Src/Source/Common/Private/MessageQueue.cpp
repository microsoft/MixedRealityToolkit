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

// Allocate 3 mb for the message buffer
static const uint32 kMessageQueueSize = 3 * (1024 * 1024);

// The maximum number of message to process from a single peer connection per update.  
static const uint32 kMaxIncomingMessages = 30;


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


bool MessageQueue::TryGetMessage(MessagePtr& msg)
{
	auto startTime = std::chrono::high_resolution_clock::now();

	bool result = m_messageQueue.TryPop(msg);

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
		MessagePtr msg = m_backupQueue.front();
		if (m_messageQueue.TryPush(msg))
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
					// Create a message from this packet
					MessagePtr msg = new Message(packet->length);
					msg->m_address = packet->systemAddress;
					msg->m_rakNetGuid = packet->guid;
					msg->m_peerID = (*peer).GetPeerID();
					msg->m_payload.Append(packet->data, packet->length);

					if (bQueueFull || !m_messageQueue.TryPush(msg))
					{
						// The queue is too full to push this packet; store it on the backup queue to be sent later
						m_backupQueue.push(msg);
						bQueueFull = true;
					}
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
