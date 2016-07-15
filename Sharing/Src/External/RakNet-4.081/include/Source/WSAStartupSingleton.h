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

#ifndef __WSA_STARTUP_SINGLETON_H
#define __WSA_STARTUP_SINGLETON_H

 /// MICROSOFT PROJECT B CHANGES BEGIN
#include <atomic>
 /// MICROSOFT PROJECT B CHANGES END

class WSAStartupSingleton
{
public:
	WSAStartupSingleton();
	~WSAStartupSingleton();
	static void AddRef(void);
	static void Deref(void);

protected:
	/// MICROSOFT PROJECT B CHANGES BEGIN
	/// making this static variable atomic so that peers destroyed in separate threads do
	/// not corrupt the reference count
	static std::atomic<int> refCount;
	/// MICROSOFT PROJECT B CHANGES END
};

#endif
