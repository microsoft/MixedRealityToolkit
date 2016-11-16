//////////////////////////////////////////////////////////////////////////
// CommonPrivate.h
//
// Main include file for all the private headers in the Common library.  
// Using this ensures that classes get defined in the correct order.  
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

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
#include <Private/ReplaceOperation.h>
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
