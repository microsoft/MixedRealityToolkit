// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// IUpdateable.h
// Interface class for objects that can be updated.  Makes it easy to 
// have a collection of subsystems that can be kept as a list and updated each in turn.  
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class IUpdateable
{
	XTOOLS_REFLECTION_DECLARE(IUpdateable)

public:
	virtual ~IUpdateable() {}

	virtual void Update() = 0;
};

XTOOLS_NAMESPACE_END
