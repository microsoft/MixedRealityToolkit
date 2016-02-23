//////////////////////////////////////////////////////////////////////////
// Syncable.cpp
//
// Base class for types that can by synchronized with the XTools sync system
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Syncable.h"

XTOOLS_NAMESPACE_BEGIN

XTOOLS_REFLECTION_DEFINE(Syncable)
.BaseClass<XTools::Reflection::XTObject>();

XTOOLS_NAMESPACE_END
