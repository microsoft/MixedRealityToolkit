//////////////////////////////////////////////////////////////////////////
// SideCarContext.h
//
// Provides the sidecar with access to XTools resources
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class SideCarContext : public AtomicRefCounted, public Reflection::XTObject
{
	XTOOLS_REFLECTION_DECLARE(SideCarContext)

public:
	virtual ~SideCarContext() {}

	// Get the connection to the baraboo paired to this PC.  
	// You can get the connection whether the pairing active or not, and use it to 
	// listen for when the connection happens
	virtual NetworkConnectionPtr GetBarabooConnection() = 0;
};

DECLARE_PTR(SideCarContext)

XTOOLS_NAMESPACE_END
