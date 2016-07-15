// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant 
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#if   defined (WINDOWS_STORE_RT)
#include <windows.h>
#include <winsock.h>
#elif defined (_WIN32)
#include <WinSock2.h>
#include <windows.h>
#include <Ws2tcpip.h>

// Must always include Winsock2.h before windows.h
// or else:
// winsock2.h(99) : error C2011: 'fd_set' : 'struct' type redefinition
// winsock2.h(134) : warning C4005: 'FD_SET' : macro redefinition
// winsock.h(83) : see previous definition of 'FD_SET'
// winsock2.h(143) : error C2011: 'timeval' : 'struct' type redefinition
// winsock2.h(199) : error C2011: 'hostent' : 'struct' type redefinition
// winsock2.h(212) : error C2011: 'netent' : 'struct' type redefinition
// winsock2.h(219) : error C2011: 'servent' : 'struct' type redefinition 

#endif
