//////////////////////////////////////////////////////////////////////////
// MemoryManager.cpp
//
// 
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Public/Utils/MemoryManager.h>

#if defined(XTOOLS_PLATFORM_WINDOWS_ANY)
#include <Winbase.h>
#include <Dbghelp.h>
#else
#include <execinfo.h>
#endif

XTOOLS_NAMESPACE_BEGIN


MemoryManager::MemoryManager()
: m_head(NULL)
, m_totalAlloc(0)
{

}


//static 
void MemoryManager::EnableDebugAllocators()
{
#if defined(XTOOLS_PLATFORM_WINDOWS_ANY)
	int dbgFlag = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);
	dbgFlag |= _CRTDBG_ALLOC_MEM_DF;	// Turns on debug allocation. 

#if defined(XTOOLS_DEBUG)
	dbgFlag |= _CRTDBG_CHECK_ALWAYS_DF;	// Causes _CrtCheckMemory to be called at every allocation and deallocation. This slows execution, but catches errors quickly.
#endif

	dbgFlag |= _CRTDBG_CHECK_CRT_DF;	// Causes blocks marked as type _CRT_BLOCK to be included in leak-detection and state-difference operations. 
	dbgFlag |= _CRTDBG_LEAK_CHECK_DF;	// Causes leak checking to be performed at program exit via a call to _CrtDumpMemoryLeaks.An error report is generated if the application has failed to free all the memory that it allocated.
	_CrtSetDbgFlag(dbgFlag);
#endif
}


//static 
void MemoryManager::AddAllocation(void* memory, size_t size)
{
	//MemoryManager::ValidateMemory();
	//MemoryManager::ValidateTracking();

	if(memory)
	{
		MemoryManager* instance = GetInstance();

		ScopedLock lock(instance->m_mutex);

		Alloc* newAlloc = new(malloc(sizeof(Alloc))) Alloc;
#pragma warning(suppress: 6011)
		newAlloc->m_memPtr = memory;
		newAlloc->m_size = size;
		newAlloc->m_next = instance->m_head;

		const int kMaxCallers = 62; 
		void* callers[kMaxCallers];
#if defined(XTOOLS_PLATFORM_WINDOWS_ANY)
		int count = CaptureStackBackTrace(0, kMaxCallers, callers, NULL);
#else
		int count = backtrace(callers, kMaxCallers);
#endif
		newAlloc->m_stackDepth = (count < kMaxStackDepth) ? count : kMaxStackDepth;
		for (int i = 0; i < newAlloc->m_stackDepth; ++i)
		{
			newAlloc->m_stack[i] = callers[i];
		}

		instance->m_head = newAlloc;

		instance->m_totalAlloc += size;
	}
}


//static 
void MemoryManager::RemoveAllocation(void* memory)
{
	//MemoryManager::ValidateMemory();
	//MemoryManager::ValidateTracking();

	if(memory)
	{
		MemoryManager* instance = GetInstance();

		ScopedLock lock(instance->m_mutex);

		Alloc* prevAlloc = NULL;
		Alloc* currentAlloc = instance->m_head;
		bool found = false;

		while(currentAlloc != NULL)
		{
			XTASSERT(currentAlloc->m_headCheck1 == 0xDEADBEEF && 
					currentAlloc->m_headCheck2 == 0xEFEFEFEF);

			if((size_t)memory - (size_t)currentAlloc->m_memPtr <= 16)
			{
				if(prevAlloc)
				{
					prevAlloc->m_next = currentAlloc->m_next;
				}

				if(currentAlloc == instance->m_head)
				{
					instance->m_head = currentAlloc->m_next;
				}

				instance->m_totalAlloc -= currentAlloc->m_size;

				free(currentAlloc);

				found = true;
				break;
			}

			prevAlloc = currentAlloc;
			currentAlloc = currentAlloc->m_next;
		}

		//ASSERT(found);
	}
}


//static 
size_t MemoryManager::GetMemoryUsed()
{
	return GetInstance()->m_totalAlloc;
}


#if defined(XTOOLS_PLATFORM_WINDOWS_DESKTOP)
//static
void MemoryManager::DumpMemory()
{
	static int dumpCount = 0;
	MemoryManager* instance = GetInstance();
	DWORD64  dwDisplacement = 0;
	DWORD64  dwAddress;
	IMAGEHLP_LINE64 line;
	DWORD	 dwLineDisplacement;
	DWORD  error;
	HANDLE hProcess;

	char buffer[sizeof(SYMBOL_INFO) + MAX_SYM_NAME * sizeof(TCHAR)];
	PSYMBOL_INFO pSymbol = (PSYMBOL_INFO)buffer;

	SymSetOptions(SYMOPT_UNDNAME | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES);

	hProcess = GetCurrentProcess();

	if (!SymInitialize(hProcess, NULL, TRUE))
	{
		error = GetLastError();
		LogError("SymInitialize returned error : %d\n", error);
	}

	std::stringstream filename;
	filename << "MemDump" << dumpCount++ << ".txt";

	FILE *myFile = nullptr;
	errno_t openResult = fopen_s(&myFile, filename.str().c_str(), "w");
	XTVERIFY(openResult == 0);

	if (myFile)
	{
		fprintf(myFile, "Total Memory Used:%u\n\n", (uint32)instance->m_totalAlloc);


		Alloc* currentAlloc = instance->m_head;
		while(currentAlloc != NULL)
		{
			dwAddress = (DWORD64)currentAlloc->m_stack[2];
			line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);

			fprintf(myFile, "Size: %u\n", (uint32)currentAlloc->m_size);

			if (currentAlloc->m_stackDepth >= 3 && SymGetLineFromAddr64(hProcess, dwAddress, &dwLineDisplacement, &line))
			{
				fprintf(myFile, "%s:%u\n", line.FileName, line.LineNumber);
			}
			else
			{
				fprintf(myFile, "Unknown File\n");
			}

			fprintf(myFile, "Callstack:\n");
		
			for(int i = 0; i < currentAlloc->m_stackDepth; i++)
			{
				dwAddress = (DWORD64)currentAlloc->m_stack[i];

				pSymbol->SizeOfStruct = 88; //sizeof(SYMBOL_INFO);
				pSymbol->MaxNameLen = MAX_SYM_NAME;

				if (SymFromAddr(hProcess, dwAddress, &dwDisplacement, pSymbol))
				{
					const char* name = pSymbol->Name;
					fprintf(myFile, "\t%i: %s\n", i, name);
				}
				else
				{
					fprintf(myFile, "\t%i: Unknown\n", i);
				}
			}

			fprintf(myFile, "\n");

			currentAlloc = currentAlloc->m_next;
		}

		_flushall();

		CONTEXT Context;
		memset(&Context, 0, sizeof(Context));
		RtlCaptureContext(&Context);


		fclose(myFile);
	}

	SymCleanup(hProcess);
}
#endif // #if defined(XTOOLS_PLATFORM_WINDOWS_DESKTOP)


//static 
void MemoryManager::ValidateMemory()
{
	// Check to make sure that hasn't been a memory stomp
#if defined(XTOOLS_DEBUG)
# if defined(XTOOLS_PLATFORM_WINDOWS_ANY)

	if (_CrtCheckMemory() != TRUE)
	{
		XTASSERT(false);
	}
# endif
#endif
}


//static 
bool MemoryManager::ValidateTracking()
{
	MemoryManager* instance = GetInstance();

	ScopedLock lock(instance->m_mutex);

	size_t totalSize = 0;

	for(Alloc* currentAlloc = instance->m_head; currentAlloc != NULL; currentAlloc = currentAlloc->m_next)
	{
		if(currentAlloc->m_headCheck1 != 0xDEADBEEF)
		{
			return false;
		}

		if( currentAlloc->m_headCheck2 != 0xEFEFEFEF)
		{
			return false;
		}

		totalSize += currentAlloc->m_size;
	}

	return (instance->m_totalAlloc == totalSize);
}


//static 
MemoryManager* MemoryManager::GetInstance()
{
	static MemoryManager memMgr;
	return &memMgr;
}

XTOOLS_NAMESPACE_END
