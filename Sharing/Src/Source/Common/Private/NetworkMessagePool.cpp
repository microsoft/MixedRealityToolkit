// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// NetworkMessagePool.cpp
// Manages a pool of NetworkOutMessages, helping to avoid some frequent allocations
// when sending messages
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NetworkMessagePool.h"
#include "NetworkOutMessageImpl.h"

XTOOLS_NAMESPACE_BEGIN

NetworkMessagePool::NetworkMessagePool(uint32 reserveMessages)
{
	for (size_t i = 0; i < reserveMessages; ++i)
	{
		m_messageQueue.push(new NetworkOutMessageImpl());
	}
}


NetworkOutMessagePtr NetworkMessagePool::AcquireMessage()
{
	NetworkOutMessagePtr newMessage;

	if (!m_messageQueue.empty())
	{
		newMessage = m_messageQueue.front();
		m_messageQueue.pop();
	}
	else
	{
		LogWarning("Increasing size of network message pool");
		newMessage = new NetworkOutMessageImpl();
	}

	return newMessage;
}


void NetworkMessagePool::ReturnMessage(const NetworkOutMessagePtr& msg)
{
	XTASSERT(msg);

	msg->Reset();

	m_messageQueue.push(msg);
}


void NetworkMessagePool::Lock()
{
	m_mutex.Lock();
}


void NetworkMessagePool::Unlock()
{
	m_mutex.Unlock();
}

XTOOLS_NAMESPACE_END
