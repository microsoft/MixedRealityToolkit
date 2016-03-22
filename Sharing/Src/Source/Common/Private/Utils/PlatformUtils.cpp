//////////////////////////////////////////////////////////////////////////
// PlatformUtils.cpp
//
// Collection of wrappers for platform-specific functions
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <chrono>
#include <thread>
#include <Public/Utils/PlatformUtils.h>

#if defined(XTOOLS_PLATFORM_WINRT)
#include <codecvt>
#include <vccorlib.h>
#endif


XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Platform)

// Wrap platform-specific versions of the sleep function in the XTools namespace to avoid confusion
void SleepMS(uint32 milliseconds)
{
    std::chrono::milliseconds duration(milliseconds);
    std::this_thread::sleep_for(duration);
}


std::string GetLocalMachineNetworkName()
{
#if defined(XTOOLS_PLATFORM_WINRT)

	auto hostNamesList = Windows::Networking::Connectivity::NetworkInformation::GetHostNames();

	std::wstring name;

	for (unsigned int i = 0; i < hostNamesList->Size; ++i)
	{
		auto hostName = hostNamesList->GetAt(i);
		if (hostName->Type == Windows::Networking::HostNameType::DomainName)
		{
			name = hostName->CanonicalName->Data();
			break;
		}
	}

	typedef std::codecvt_utf8<wchar_t> convert_type;
	std::wstring_convert<convert_type, wchar_t> converter;

	//use converter (.to_bytes: wstr->str, .from_bytes: str->wstr)
	return converter.to_bytes(name);

#elif defined(XTOOLS_PLATFORM_WINDOWS_DESKTOP)
	TCHAR nameWide[250];
	char nameString[250];

	DWORD hstSize = sizeof(nameWide);
	GetComputerNameEx(ComputerNamePhysicalDnsFullyQualified, nameWide, &hstSize);
	size_t charactersConverted = 0;
	wcstombs_s(&charactersConverted, nameString, nameWide, sizeof(nameString));

	return std::string(nameString);

#elif defined(XTOOLS_PLATFORM_OSX)
    char nameBuffer[512];
    if(gethostname(nameBuffer, sizeof(nameBuffer)) == 0)
    {
        return std::string(nameBuffer);
    }
    else
    {
        return std::string();
    }
    
#else
#error Unsupported Platform
#endif
}


uint64 GetThreadID()
{
#if defined(XTOOLS_PLATFORM_WINDOWS_ANY)
	return GetCurrentThreadId();

#elif defined(XTOOLS_PLATFORM_OSX)
	pthread_t ptid = pthread_self();
	uint64 threadId = 0;
	memcpy(&threadId, &ptid, std::min(sizeof(threadId), sizeof(ptid)));
	return threadId;

#else
# error Unsupported Platform

#endif
}

NAMESPACE_END(Platform)
XTOOLS_NAMESPACE_END
