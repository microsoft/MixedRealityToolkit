//////////////////////////////////////////////////////////////////////////
// DirectPairConnector.cpp
//
// 
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DirectPairConnector.h"

XTOOLS_NAMESPACE_BEGIN

DirectPairConnector::DirectPairConnector()
: m_remoteName(new XString("localhost"))
, m_remotePort(kAppPluginPort)
{

}


DirectPairConnector::DirectPairConnector(const XStringPtr& remoteNameOrIP)
	: m_remoteName(remoteNameOrIP)
	, m_remotePort(kAppPluginPort)
{

}


DirectPairConnector::DirectPairConnector(const XStringPtr& remoteNameOrIP, uint16 port)
	: m_remoteName(remoteNameOrIP)
	, m_remotePort(port)
{

}


bool DirectPairConnector::IsReceiver()
{
	return false;
}



int32 DirectPairConnector::GetAddressCount()
{
	return 1;
}


XStringPtr DirectPairConnector::GetAddress(int32 )
{
	return m_remoteName;
}


uint16 DirectPairConnector::GetPort()
{
	return m_remotePort;
}


void DirectPairConnector::Update()
{
	// Intentionally blank
}


bool DirectPairConnector::IsReadyToConnect()
{
	return true;
}


void DirectPairConnector::SetRemoteAddress(const XStringPtr& remoteNameOrIP)
{
	m_remoteName = remoteNameOrIP;
}


void DirectPairConnector::SetRemotePort(uint16 port)
{
	m_remotePort = port;
}

XTOOLS_NAMESPACE_END