// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// CommonPrivate.h
// Main include file for all the private headers in the Common library.  
// Using this ensures that classes get defined in the correct order.  
//////////////////////////////////////////////////////////////////////////

#pragma once

#if defined(XTOOLS_PLATFORM_WINDOWS_DESKTOP)
# define CURL_STATICLIB
# include <curl/curl.h>
#elif defined(XTOOLS_PLATFORM_OSX)
# include <curl/curl.h>
#endif

#include <Private/RakNetIncludes.h>
#include <Private/Utils/ScopedLock.h>
#include <Private/Utils/TypeUtils.h>
#include <Private/Peer.h>
#include <Private/XSocketImpl.h>
#include <Private/XSocketManagerImpl.h>
#include <Private/NetworkCommonPrivate.h>
#include <Private/NetworkConnectionImpl.h>
#include <Private/Operation.h>
#include <Private/AckOperation.h>
#include <Private/NoopOperation.h>
#include <Private/CreateOperation.h>
#include <Private/ModifyOperation.h>
#include <Private/DeleteOperation.h>
#include <Private/UpdateOperation.h>
#include <Private/InsertOperation.h>
#include <Private/RemoveOperation.h>
#include <Private/ArrayElement.h>
#include <Private/IntElementImpl.h>
#include <Private/FloatElementImpl.h>
#include <Private/StringElementImpl.h>
#include <Private/ObjectElementImpl.h>
#include <Private/IntArrayElementImpl.h>
#include <Private/OperationFactory.h>
#include <Private/ElementFactory.h>
#include <Private/OpTransform.h>
#include <Private/TransformManager.h>
#include <Private/SyncContext.h>
