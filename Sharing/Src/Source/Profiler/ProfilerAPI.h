//////////////////////////////////////////////////////////////////////////
// ProfilerAPI.h
//
// Includes all the headers required to use the profiler library
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "../Common/Public/Environment.h"
#include "../Common/Public/BasicTypes.h"
#include "../Common/Public/XToolsTypes.h"
#include "../Common/Public/Utils/SwigHelperMacros.h"
#include "../Common/Public/Utils/GetPointer.h"
#include "../Common/Public/Utils/RefCounted.h"
#include "../Common/Public/Utils/AtomicRefCounted.h"
#include "../Common/Public/Utils/RefPtr.h"
#include "../Common/Public/Utils/LogManager.h"
#include "../Common/Public/Utils/LogWriter.h"
#include "../Common/Public/Utils/Receipt.h"
#include "../Common/Public/Reflection.h"
#include "../Common/Public/XString.h"
#include "../Common/Public/SystemRole.h"
#include "../Common/Public/Listener.h"
#include "../Common/Public/DiscoveredSystem.h"
#include "../Common/Public/DiscoveryClientListener.h"
#include "../Common/Public/DiscoveryClient.h"

#include "LogMessage.h"
#include "ProfileSample.h"
#include "ProfileThread.h"
#include "ProfileFrame.h"
#include "ProfilerStreamListener.h"
#include "ProfilerStream.h"
#include "ProfilerStreamManager.h"