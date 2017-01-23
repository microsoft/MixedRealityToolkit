//////////////////////////////////////////////////////////////////////////
// SideCarAPI.h
//
// Single include file that includes all the other files that make up the 
// XTools SideCar API
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#define XTOOLS_SIDECAR

#include <Public/Environment.h>
#include <Public/BasicTypes.h>
#include <Public/XToolsTypes.h>
#include <Public/Utils/RefCounted.h>
#include <Public/Utils/AtomicRefCounted.h>
#include <Public/Utils/GetPointer.h>
#include <Public/Utils/RefPtr.h>
#include <Public/Utils/Receipt.h>
#include <Public/Utils/SwigHelperMacros.h>
#include <Public/Utils/LogManager.h>
#include <Public/Reflection.h>
#include <Public/XToolsMessageTypes.h>
#include <Public/XString.h>
#include <Public/Listener.h>
#include <Public/NetworkOutMessage.h>
#include <Public/NetworkInMessage.h>
#include <Public/NetworkConnectionListener.h>
#include <Public/User.h>
#include <Public/NetworkConnection.h>
#include <Public/ProfileManager.h>
#include <Public/SideCarContext.h>
#include <Public/SideCar.h>
#include <Public/SideCarFunctions.h>
#include <Public/SideCarMacros.h>
#include <Public/ScopedProfile.h>