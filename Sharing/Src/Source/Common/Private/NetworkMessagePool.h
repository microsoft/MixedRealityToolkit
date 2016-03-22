//////////////////////////////////////////////////////////////////////////
// NetworkMessagePool.h
//
// Manages a pool of NetworkOutMessages, helping to avoid some frequent allocations
// when sending messages
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <queue>

XTOOLS_NAMESPACE_BEGIN

class NetworkMessagePool : public AtomicRefCounted
{
public:
	NetworkMessagePool(uint32 reserveMessages);

	// Returns a new message ready to start adding data for sending.
	// Will try and return a message already in the pool, or allocate a new one
	// if the pool is empty
	NetworkOutMessagePtr AcquireMessage();

	// Returns a message to the pool.  The contents are reset and the message
	// is stored in the pool
	void ReturnMessage(const NetworkOutMessagePtr& msg);

	// Lock and unlock the pool's mutex.  Called by ref_ptr_ts
	void Lock();
	void Unlock();

private:
	std::queue<NetworkOutMessagePtr>	m_messageQueue;
	Mutex								m_mutex;
};

DECLARE_THREADSAFE_PTR(NetworkMessagePool)

XTOOLS_NAMESPACE_END