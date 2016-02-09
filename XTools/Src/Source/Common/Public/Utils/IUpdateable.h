//////////////////////////////////////////////////////////////////////////
// IUpdateable.h
//
// Interface class for objects that can be updated.  Makes it easy to 
// have a collection of subsystems that can be kept as a list and updated each in turn.  
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
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
