//////////////////////////////////////////////////////////////////////////
// WrapperAPIs.h
//
// Single place to include all the APIs that should be exposed to 
// Java and C# through SWIG
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <Public/Environment.h>
#include <Public/BasicTypes.h>
#include <Public/XToolsTypes.h>
#include <Public/Utils/SwigHelperMacros.h>
#include <Public/Utils/GetPointer.h>
#include <Public/Utils/RefCounted.h>
#include <Public/Utils/AtomicRefCounted.h>
#include <Public/Utils/RefPtr.h>
#include <Public/Utils/Receipt.h>
#include <Public/Utils/LogManager.h>
#include <Public/Utils/LogWriter.h>
#include <Public/Utils/Log.h>
#include <Public/Reflection.h>
#include <Public/Listener.h>
#include <Public/XToolsMessageTypes.h>
#include <Public/XGuid.h>
#include <Public/XString.h>
#include <Public/NetworkOutMessage.h>
#include <Public/NetworkInMessage.h>
#include <Public/NetworkConnectionListener.h>
#include <Public/User.h>
#include <Public/NetworkConnection.h>
#include <Public/XValue.h>
#include <Public/Element.h>
#include <Public/BoolElement.h>
#include <Public/IntElement.h>
#include <Public/LongElement.h>
#include <Public/FloatElement.h>
#include <Public/DoubleElement.h>
#include <Public/StringElement.h>
#include <Public/IntArrayListener.h>
#include <Public/IntArrayElement.h>
#include <Public/FloatArrayListener.h>
#include <Public/FloatArrayElement.h>
#include <Public/StringArrayListener.h>
#include <Public/StringArrayElement.h>
#include <Public/ObjectElementListener.h>
#include <Public/ObjectElement.h>
#include <Public/SyncListener.h>
#include <Public/SessionListener.h>
#include <Public/Session.h>
#include <Public/SessionManagerListener.h>
#include <Public/SessionManager.h>
#include <Public/UserPresenceManagerListener.h>
#include <Public/UserPresenceManager.h>
#include <Public/ClientConfig.h>
#include <Public/AudioManager.h>
#include <Public/ProfileManager.h>
#include <Public/Profile.h>
#include <Public/Room.h>
#include <Public/AnchorDownloadRequest.h>
#include <Public/RoomManagerListener.h>
#include <Public/RoomManager.h>
