// Copyright (C) Microsoft. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

#pragma once
#ifndef	_MUTEX_Z_H
#define	_MUTEX_Z_H

#include <Types_Z.h>
#include <Assert_Z.h>
#include <BeginDef_Z.h>

class  SharedResource_Z;

// a guard object permits shared access to the guarded region
// it guards a critical section
// multiple guards may be entered from the same thread - it is recursively safe
class  SharedResourceGuard_Z
{
public:
	explicit SharedResourceGuard_Z(SharedResource_Z &sharedResource);
	~SharedResourceGuard_Z();

	private:
	// the shared resource that this guard protects
	SharedResource_Z &sharedResource;

	// neither assignment nor copy construction make any sense at all - guards cannot be cloned
	SharedResourceGuard_Z &operator =(const SharedResourceGuard_Z&);
	SharedResourceGuard_Z (const SharedResourceGuard_Z&);
};

// the basic critical section object representing a shared resource
class  SharedResource_Z
{
public:
	// create and destroy a shared resource mutex
	SharedResource_Z();
	~SharedResource_Z();

	// manual use of the object
	void Lock();
	void Unlock();
	Bool TryLock();

private:
	// private data for mutex - keep windows out of our header file
	class Private;
	Private *p;

	// neither assignment nor copy construction are defined
	SharedResource_Z &operator =(const SharedResource_Z&);
	SharedResource_Z (const SharedResource_Z&);

	// mutex guards call enter and leave directly
	friend class SharedResourceGuard_Z;
};


//
// Class Event
//
 
class  Event_Z
{
private:
	Event_Z& operator = ( const Event_Z&)
	{
		return *this; 
	}

public:
	Event_Z()
	{
		if (!Create())
		{
		  EXCEPTIONC_Z(FALSE,"Event::Event - unable to create event");
		}
	}
	~Event_Z()	
	{
	    Delete();
	}

	// Signal the event
	void Signal();
	void Reset();

	// Wait for the event for a specific number of ms
	Bool Wait(S32 time = -1, Bool reset=TRUE);

	// Poll the event, returns TRUE if signalled
	Bool Poll();

	// Get Event handle
	inline U64 GetHandle() const { return eventHandle; }

	// Wait for a bunch of events for a number of ms. Return a negative code on error (see ThreadWait_Z), or the id of the first event to enter in signaled state if waitAll is FALSE
	static S32 WaitEvents( const Event_Z* events, const U8 count, const Bool waitAll = TRUE, const S32 time = -1 );
	static S32 WaitEvents( Event_Z*const* events, const U8 count, const Bool waitAll = TRUE, const S32 time = -1 );
	// Poll a bunch of events
	static Bool PollEvents( const Event_Z* events, const U8 count );
	static Bool PollEvents( Event_Z*const* events, const U8 count );

private:
	// Create the event
	Bool Create();

	// Delete the event
	void Delete();

	// the operating system's event handle
	U64 eventHandle;
};


//
// Semaphore

class Semaphore_Z
{
public:
	Semaphore_Z() : handle(0) {}
	~Semaphore_Z() { Destroy(); }

	void Create(U32 initialCount, U32 maxCount);
	void Destroy();

	void Release(U32 addValue);

	Bool Wait(S32 time = -1);

	// Get Event handle
	inline U64 GetHandle() const { return handle; }

private:
	// the operating system's event handle
	U64 handle;
};

#endif
