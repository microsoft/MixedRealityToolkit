// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// Syncable.cpp
// Base class for types that can by synchronized with the XTools sync system
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Syncable.h"

XTOOLS_NAMESPACE_BEGIN

XTOOLS_REFLECTION_DEFINE(Syncable)
.BaseClass<XTools::Reflection::XTObject>();

XTOOLS_NAMESPACE_END
