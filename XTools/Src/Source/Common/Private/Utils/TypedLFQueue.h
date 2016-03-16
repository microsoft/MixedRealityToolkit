//////////////////////////////////////////////////////////////////////////
// TypedLFQueue.h
//
// A wrapper around the lock-free queue that makes it easier to use with 
// objects with a valid copy constructor, and preserves RefCounted objects
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <Private/Buffer.h>
#include <Private/Utils/LFQueue.h>

XTOOLS_NAMESPACE_BEGIN

template<typename T>
class TypedLFQueue : protected LFQueue
{
public:
	explicit TypedLFQueue(uint32 cacheSize)
		: LFQueue(cacheSize)
		, m_readBuffer(sizeof(T))
	{

	}

	virtual ~TypedLFQueue()
	{
		// Pop all remaining items in the queue to ensure they are cleaned up correctly
		while (LFQueue::TryPop(m_readBuffer))
		{
			T* msg = reinterpret_cast<T*>(m_readBuffer.GetData());
			msg->~T();
		}
	}

	bool TryPush(const T& msg)
	{
		byte* buffer[sizeof(T)];

		{
			// Use the copy constructor to copy the msg to the buffer.  
			// This keeps reference counting in tact, while preventing the destructor from executing when this function exits
			T* proxyObj = new(buffer) T;
			*proxyObj = msg;
		}
		
		// Push the buffer 
		if (!LFQueue::TryPush(buffer, sizeof(T)))
		{
			reinterpret_cast<T*>(buffer)->~T();
			return false;
		}

		return true;
	}

	bool TryPop(T& msgOut)
	{
		// Try to pop a message off the queue
		if (LFQueue::TryPop(m_readBuffer))
		{
			T* msg = reinterpret_cast<T*>(m_readBuffer.GetData());

			// Copy to the out message
			msgOut = *msg;

			// Destruct the object that was passed through the queue
			msg->~T();

			return true;
		}

		return false;
	}

private:
	Buffer m_readBuffer;
};

XTOOLS_NAMESPACE_END
