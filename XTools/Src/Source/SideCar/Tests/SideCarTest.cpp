//////////////////////////////////////////////////////////////////////////
// SideCarTest.cpp
//
// Implementation of the SideCar interface.  Used for testing that the API works
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SideCarTest.h"
#include <iostream>
#include <assert.h>
#include <string>

XT_DEFINE_SIDECAR(SideCarTest)


SideCarTest::SideCarTest()
{
    
}


void SideCarTest::Initialize(const XTools::SideCarContextPtr& context)
{
	std::cout << "SideCarTest::Initialize\n";

	// Save the context in case we want to use it later
	m_context = context;

	// Register to receive notifications from the baraboo connection.  Use the Receipt version so that this 
	// class will automatically unregister when it is destroyed
	m_context->GetBarabooConnection()->AddListener(kTestMessageID, this);
}


void SideCarTest::Update()
{
	
}


void SideCarTest::OnConnected(const XTools::NetworkConnectionPtr&)
{
	std::cout << "SideCarTest::OnConnected\n";
}


void SideCarTest::OnDisconnected(const XTools::NetworkConnectionPtr&)
{
	std::cout << "SideCarTest::OnDisconnected\n";
}


void SideCarTest::OnMessageReceived(const XTools::NetworkConnectionPtr& connection, XTools::NetworkInMessage&)
{
	std::cout << "SideCarTest::OnMessageReceived\n";

	using namespace XTools;

	// Message received from Baraboo: reply with a message of the same ID
	NetworkOutMessagePtr outMessage = connection->CreateMessage(kTestMessageID);

	outMessage->Write(12345);
	outMessage->Write(12345.f);

	connection->Send(outMessage);
}
#include <string>