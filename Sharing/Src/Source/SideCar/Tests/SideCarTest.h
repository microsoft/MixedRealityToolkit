// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// SideCarTest.h
// Implementation of the SideCar interface.  Used for testing that the API works
//////////////////////////////////////////////////////////////////////////

#pragma once

class SideCarTest : public XTools::SideCar, public XTools::NetworkConnectionListener
{
public:
	SideCarTest();

private:
	// XTools::SideCar Functions:
	virtual void Initialize(const XTools::SideCarContextPtr& context) override;
	virtual void Update() override;

	// XTools::NetworkConnectionListener Functions:
	virtual void OnConnected(const XTools::NetworkConnectionPtr& connection) override;
	virtual void OnDisconnected(const XTools::NetworkConnectionPtr& connection) override;
	virtual void OnMessageReceived(const XTools::NetworkConnectionPtr& connection, XTools::NetworkInMessage& message) override;

	XTools::SideCarContextPtr m_context;
	static const XTools::byte kTestMessageID = 254;
};
