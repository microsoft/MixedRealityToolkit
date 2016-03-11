//////////////////////////////////////////////////////////////////////////
// NetworkThreadCommands.cpp
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NetworkThreadCommands.h"

XTOOLS_NAMESPACE_BEGIN

XTOOLS_REFLECTION_DEFINE(Command)
.BaseClass<Reflection::XTObject>();

XTOOLS_REFLECTION_DEFINE(OpenCommand)
.BaseClass<Command>();

XTOOLS_REFLECTION_DEFINE(AcceptCommand)
.BaseClass<Command>();

XTOOLS_NAMESPACE_END
