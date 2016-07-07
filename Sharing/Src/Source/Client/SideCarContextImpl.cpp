// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// SideCarContextImpl.cpp
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SideCarContextImpl.h"
#include <Private/DownloadManagerImpl.h>

XTOOLS_NAMESPACE_BEGIN

XTOOLS_REFLECTION_DEFINE(SideCarContext)
.BaseClass<Reflection::XTObject>();

XTOOLS_REFLECTION_DEFINE(SideCarContextImpl)
.BaseClass<SideCarContext>()
.BaseClass<IUpdateable>();


SideCarContextImpl::SideCarContextImpl(const NetworkConnectionPtr& barabooConnection)
: m_barabooConnection(barabooConnection)
{
#if !defined(XTOOLS_PLATFORM_WINRT)
	m_downloadManager = new DownloadManagerImpl();
#endif
}


// Get the connection to the baraboo paired to this PC.  
// You can get the connection whether the pairing active or not, and use it to 
// listen for when the connection happens
NetworkConnectionPtr SideCarContextImpl::GetBarabooConnection()
{
	return m_barabooConnection;
}


// Get the download manager for downloading data over http  
DownloadManagerPtr SideCarContextImpl::GetDownloadManager()
{
	return m_downloadManager;
}


void SideCarContextImpl::Update()
{
	if (m_downloadManager)
	{
		m_downloadManager->Update();
	}
}

XTOOLS_NAMESPACE_END
