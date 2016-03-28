//////////////////////////////////////////////////////////////////////////
// Common.h
//
// Header including all interfaces that get exposed to consumers of this library
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <WrapperAPIs.h>
#include <Public/Platform.h>

#include <../VersionInfo.h>

#include <Public/XToolsVersion.h>
#include <Public/CommonSettings.h>
#include <Public/Utils/PlatformUtils.h>
#include <Public/Utils/ScopedPtr.h>
#include <Public/Utils/ScopedArray.h>
#include <Public/Utils/RefPtrTS.h>
#include <Public/Utils/RefPtrProxy.h>
#include <Public/Utils/RefPtrProxyTS.h>
#include <Public/Utils/Logger.h>
#include <Public/Utils/Receipt.h>
#include <Public/Utils/Hash.h>
#include <Public/Utils/Mutex.h>
#include <Public/Utils/MemoryManager.h>
#include <Public/Utils/XThread.h>
#include <Public/Utils/MemberFuncThread.h>
#include <Public/Utils/ClassThreadRunner.h>
#include <Public/Utils/NotifyList.h>
#include <Public/Utils/IUpdateable.h>
#include <Public/Utils/Event.h>
#include <Public/Utils/StringUtils.h>
#include <Private/Utils/RegistrationReceipt.h>
#include <Public/Utils/ListenerList.h>
#include <Public/ScopedProfile.h>
#include <Public/IPAddress.h>
#include <Public/XSocket.h>
#include <Public/XSocketManager.h>
#include <Public/XSocketListener.h>
#include <Public/IncomingXSocketListener.h>
#include <Public/HandshakeLogic.h>
#include <Public/NetworkHandshake.h>
#include <Public/AuthorityLevel.h>
#include <Public/SyncManager.h>
#include <Public/SessionDescriptor.h>
#include <Public/UserPresenceManagerListener.h>
#include <Public/UserPresenceManager.h>
#include <Public/AudioSessionProcessor.h>
#include <Public/AudioManager.h>
#include <Public/ClientContext.h>

#include <Public/PairingHandshakeLogic.h>
#include <Public/SessionListHandshakeLogic.h>
#include <Public/SessionHandshakeLogic.h>

#include <Public/SystemRole.h>
#include <Public/DiscoveredSystem.h>
#include <Public/DiscoveryClientListener.h>
#include <Public/DiscoveryClient.h>

#include <Public/ProfileManager.h>

#include <Private/json/cpprest/json.h>
#include <Private/json/cpprest/asyncrt_utils.h>
#include <Private/JSONMessage.h>
#include <Private/SessionMessageRouter.h>
#include <Private/NewSessionRequest.h>
#include <Private/NewSessionReply.h>
#include <Private/JoinSessionRequest.h>
#include <Private/JoinSessionReply.h>
#include <Private/UserJoinedSessionMsg.h>
#include <Private/UserChangedSessionMsg.h>
#include <Private/UserLeftSessionMsg.h>
#include <Private/ListSessionsRequest.h>
#include <Private/ListSessionsReply.h>
#include <Private/SessionAddedMsg.h>
#include <Private/SessionClosedMsg.h>

#include <Private/Syncable.h>
#include <Private/SyncObject.h>
#include <Private/SyncPrimitives.h>
#include <Private/SyncEnum.h>
#include <Private/SyncArray.h>
