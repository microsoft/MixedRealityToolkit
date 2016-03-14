//////////////////////////////////////////////////////////////////////////
// MemoryManager.h
//
// Tracks allocations, detects leaks and stomps, prints callstacks for 
// where allocations came from
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class MemoryManager
{
public:
	// Enable debug memory tracking and verification. 
	static void EnableDebugAllocators();

	static void AddAllocation(void* memory, size_t size);
	static void RemoveAllocation(void* memory);

	static size_t GetMemoryUsed();

#if defined(XTOOLS_PLATFORM_WINDOWS_DESKTOP)
	// Write out all active allocations, along with their call stack and size
	static void DumpMemory();
#endif

	// Check to see if there has been a stomp or buffer overrun
	// Does nothing in Release
	static void ValidateMemory();

	// Loop through all the recorded memory allocations and make sure that our own
	// structures are valid.  
	static bool ValidateTracking();

private:

	static const int kMaxStackDepth = 30;

	struct Alloc
	{
		inline Alloc() 
			: m_headCheck1(0xDEADBEEF)
			, m_next(NULL)
			, m_memPtr(NULL)
			, m_size(0)
			, m_stackDepth(0)
			, m_headCheck2(0xEFEFEFEF) {}

		unsigned int	m_headCheck1;
		Alloc*			m_next;
		void*			m_memPtr;
		size_t			m_size;
		void*			m_stack[kMaxStackDepth];
		int				m_stackDepth;
		unsigned int	m_headCheck2;
	};

	static MemoryManager* GetInstance();

	MemoryManager();

	Alloc* m_head;
	Mutex m_mutex;
	size_t m_totalAlloc;
};

XTOOLS_NAMESPACE_END


// Overload the global new and delete operators so that we can track each allocation.
// User code should put this define in the main cpp file of their exe or dll.  
#define XT_TRACK_MEMORY \
void* operator new(size_t size) \
{ \
	void* mem = malloc(size); \
	::XTools::MemoryManager::AddAllocation(mem, size); \
	return mem; \
} \
void* __cdecl operator new[](size_t size) \
{ \
	void* mem = malloc(size); \
	::XTools::MemoryManager::AddAllocation(mem, size); \
	return mem; \
} \
void operator delete(void* p) \
{ \
	::XTools::MemoryManager::RemoveAllocation(p); \
	free(p); \
} \
void operator delete[](void* p) \
{ \
	::XTools::MemoryManager::RemoveAllocation(p); \
	free(p); \
}

