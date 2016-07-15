// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// PairingInfo.h
// Information about a pairing, along with support for simple serialization
// to and from a string
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class PairingInfo
{
public:
	PairingInfo();

	const IPAddressList&	GetAddresses() const;
	void					SetAddresses(const IPAddressList& addresses);

	uint16					GetPort() const;
	void					SetPort(uint16 port);

	int32					GetLocalKey() const;
	void					SetLocalKey(int32 localKey);

	int32					GetRemoteKey() const;
	void					SetRemoteKey(int32 remoteKey);

	std::string				Encode() const;
	bool					Decode(const std::string& encodedString);

private:
	IPAddressList	m_addresses;
	uint16			m_port;
	int32			m_localKey;
	int32			m_remoteKey;
};

XTOOLS_NAMESPACE_END
