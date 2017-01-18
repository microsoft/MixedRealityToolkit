//////////////////////////////////////////////////////////////////////////
// SideCarContextImpl.cpp
//
// 
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SideCarContextImpl.h"

XTOOLS_NAMESPACE_BEGIN

XTOOLS_REFLECTION_DEFINE(SideCarContext)
.BaseClass<Reflection::XTObject>();

XTOOLS_REFLECTION_DEFINE(SideCarContextImpl)
.BaseClass<SideCarContext>();


SideCarContextImpl::SideCarContextImpl(const NetworkConnectionPtr& barabooConnection)
: m_barabooConnection(barabooConnection)
{

}


// Get the connection to the baraboo paired to this PC.  
// You can get the connection whether the pairing active or not, and use it to 
// listen for when the connection happens
NetworkConnectionPtr SideCarContextImpl::GetBarabooConnection()
{
	return m_barabooConnection;
}

XTOOLS_NAMESPACE_END