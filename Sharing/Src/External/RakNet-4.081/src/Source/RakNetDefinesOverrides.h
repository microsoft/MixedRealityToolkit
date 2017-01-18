/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant 
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

// USER EDITABLE FILE

//#if !defined(WINDOWS_STORE_RT)
//#define RAKNET_SUPPORT_IPV6 1
//#endif


#define RESEND_BUFFER_ARRAY_LENGTH 2048
#define RESEND_BUFFER_ARRAY_MASK 2047

#define RAKPEER_USER_THREADED 1

#define RakAssert(x) assert(x);