/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant 
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include "WSAStartupSingleton.h"





#if   defined(_WIN32) && !defined(WINDOWS_STORE_RT)
#include <winsock2.h>
#include <ws2tcpip.h>





#endif
#include "RakNetDefines.h"
#include <stdio.h>

/// MICROSOFT PROJECT B CHANGES BEGIN
std::atomic<int> WSAStartupSingleton::refCount(0);
/// MICROSOFT PROJECT B CHANGES END

WSAStartupSingleton::WSAStartupSingleton() {}
WSAStartupSingleton::~WSAStartupSingleton() {}
void WSAStartupSingleton::AddRef(void)
{
#if defined(_WIN32) && !defined(WINDOWS_STORE_RT)

	/// MICROSOFT PROJECT B CHANGES BEGIN
	if (++refCount != 1)
		return;
	/// MICROSOFT PROJECT B CHANGES END

	WSADATA winsockInfo;
	if ( WSAStartup( MAKEWORD( 2, 2 ), &winsockInfo ) != 0 )
	{
#if  defined(_DEBUG) && !defined(WINDOWS_PHONE_8)
		DWORD dwIOError = GetLastError();
		LPVOID messageBuffer;
		FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL, dwIOError, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),  // Default language
			( LPTSTR ) & messageBuffer, 0, NULL );
		// something has gone wrong here...
		RAKNET_DEBUG_PRINTF( "WSAStartup failed:Error code - %d\n%s", dwIOError, messageBuffer );
		//Free the buffer.
		LocalFree( messageBuffer );
#endif
	}

#endif
}
void WSAStartupSingleton::Deref(void)
{
#if defined(_WIN32) && !defined(WINDOWS_STORE_RT)

	/// MICROSOFT PROJECT B CHANGES BEGIN
	if (--refCount == 0)
		WSACleanup();
	/// MICROSOFT PROJECT B CHANGES END
		
#endif
}
