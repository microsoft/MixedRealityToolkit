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

#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_LogCommandParser==1 && _RAKNET_SUPPORT_PacketLogger==1
#include "PacketConsoleLogger.h"
#include "LogCommandParser.h"
#include <stdio.h>

using namespace RakNet;

PacketConsoleLogger::PacketConsoleLogger()
{
	logCommandParser=0;
}

void PacketConsoleLogger::SetLogCommandParser(LogCommandParser *lcp)
{
	logCommandParser=lcp;
	if (logCommandParser)
		logCommandParser->AddChannel("PacketConsoleLogger");
}
void PacketConsoleLogger::WriteLog(const char *str)
{
	if (logCommandParser)
		logCommandParser->WriteLog("PacketConsoleLogger", str);
}

#endif // _RAKNET_SUPPORT_*
