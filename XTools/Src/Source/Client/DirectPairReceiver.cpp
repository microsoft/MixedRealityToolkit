//////////////////////////////////////////////////////////////////////////
// DirectPairReceiver.cpp
//
// 
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DirectPairReceiver.h"

XTOOLS_NAMESPACE_BEGIN

DirectPairReceiver::DirectPairReceiver()
	: m_localPort(kAppPluginPort)
{

}


DirectPairReceiver::DirectPairReceiver(uint16 port)
	: m_localPort(port)
{

}


bool DirectPairReceiver::IsReceiver()
{
	return true;
}


int32 DirectPairReceiver::GetAddressCount()
{
	return 0;
}


XStringPtr DirectPairReceiver::GetAddress(int32 )
{
	return nullptr;
}


uint16 DirectPairReceiver::GetPort()
{
	return m_localPort;
}


void DirectPairReceiver::Update()
{
	// Intentionally Blank
}


bool DirectPairReceiver::IsReadyToConnect()
{
	return true;
}


void DirectPairReceiver::SetIncomingPort(uint16 port)
{
	m_localPort = port;
}

XTOOLS_NAMESPACE_END