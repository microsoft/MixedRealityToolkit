//////////////////////////////////////////////////////////////////////////
// MessageQueue.h
//
// Checks for incoming messages in a separate thread and adds them to a 
// lock-free queue to by read by the main thread
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "Message.h"
#include "Utils/LFQueue.h"
#include "MessageInterceptor.h"
#include <queue>

XTOOLS_NAMESPACE_BEGIN

class MessageQueue
{
public:
	MessageQueue();
	~MessageQueue();

	void AddConnection(const PeerPtr& peer);
	void RemoveConnection(const PeerPtr& peer);

	void AddMessageInterceptor(const MessageInterceptorPtr& interceptor);
	void RemoveMessageInterceptor(const MessageInterceptorPtr& interceptor);

	// Returns true if a message was successfully popped from the queue
	bool TryGetMessage(Message& msg);

private:
	void ProcessMessages();

	// Check to see if any of the interceptors want to handle this packet themselves
	bool InterceptPacket(RakNet::Packet* packet, std::vector<MessageInterceptorPtr>& interceptors);

	void ThreadFunc();

	struct MessageHeader;
	struct PeerSettings;

	// Queue of messages
	LFQueue					m_messageQueue;

	// Buffer to hold the message data as its being reassembled on the main thread
	Buffer					m_messageBufferIn;
	Buffer					m_messageBufferOut;

	std::vector<PeerSettings>	m_peers;

	MemberFuncThreadPtr		m_networkThread;
	volatile int			m_stopping;

	Mutex					m_mutex;

	// If the main lock-free queue is full, store the messages here to be sent later
	std::queue<std::shared_ptr<Buffer> > m_backupQueue;
};

XTOOLS_NAMESPACE_END