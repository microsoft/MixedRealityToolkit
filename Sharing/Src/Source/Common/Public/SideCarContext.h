// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// SideCarContext.h
// Provides the sidecar with access to XTools resources
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
	
	// Get the download manager for downloading data over http  
	virtual DownloadManagerPtr	GetDownloadManager() = 0;
};

DECLARE_PTR(SideCarContext)

XTOOLS_NAMESPACE_END
