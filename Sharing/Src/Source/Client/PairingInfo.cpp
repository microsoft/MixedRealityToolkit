//////////////////////////////////////////////////////////////////////////
// PairingInfo.cpp
//
// Information about a pairing, along with support for simple serialization
// to and from a string
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PairingInfo.h"

XTOOLS_NAMESPACE_BEGIN

static const std::string kPairingStringPrefix("XT>");

PairingInfo::PairingInfo()
	: m_port(0)
	, m_localKey(0)
	, m_remoteKey(0)
{ }


const IPAddressList& PairingInfo::GetAddresses() const
{
	return m_addresses;
}


void PairingInfo::SetAddresses(const IPAddressList& address)
{
	m_addresses = address;
}


uint16 PairingInfo::GetPort() const
{
	return m_port;
}


void PairingInfo::SetPort(uint16 port)
{
	m_port = port;
}


int32 PairingInfo::GetLocalKey() const
{
	return m_localKey;
}


void PairingInfo::SetLocalKey(int32 localKey)
{
	m_localKey = localKey;
}


int32 PairingInfo::GetRemoteKey() const
{
	return m_remoteKey;
}


void PairingInfo::SetRemoteKey(int32 remoteKey)
{
	m_remoteKey = remoteKey;
}


std::string PairingInfo::Encode() const
{
	std::stringstream encodedString;

	encodedString.flags(std::ios::hex);	// Write it all as hex, keep the

	encodedString << kPairingStringPrefix;	// prefix to ensure we're reading the right QR code

	encodedString << m_addresses.size() << ";";	// the number of IP addresses

	// Write each IP address
	for (size_t i = 0; i < m_addresses.size(); ++i)
	{
		encodedString << (uint32)m_addresses[i][0] << ".";
		encodedString << (uint32)m_addresses[i][1] << ".";
		encodedString << (uint32)m_addresses[i][2] << ".";
		encodedString << (uint32)m_addresses[i][3] << ";";
	}
	
	encodedString << m_port << ";";	// Write the port

	encodedString << m_localKey << ";";	// Write the local pairing key

	encodedString << m_remoteKey << ";"; // Write the remote pairing key

	return encodedString.str();
}


bool PairingInfo::Decode(const std::string& encodedString)
{
	m_addresses.clear();

	// Verify the preamble is correct
	std::string prefix = encodedString.substr(0, kPairingStringPrefix.length());
	if (prefix != kPairingStringPrefix)
	{
		return false;
	}

	std::vector<std::string> tokens = StringUtils::Tokenize(encodedString.c_str() + kPairingStringPrefix.length(), ';');

	try
	{
		size_t tokenIndex = 0;

		int numAddresses = std::stoi(tokens[tokenIndex++], nullptr, 16);
		if (numAddresses == 0)
		{
			return false;
		}

		for (int i = 0; i < numAddresses; ++i)
		{
			if (tokenIndex == tokens.size())
			{
				return false;
			}

			std::vector<std::string> addressTokens = StringUtils::Tokenize(tokens[tokenIndex++].c_str(), '.');
			if (addressTokens.size() != 4)
			{
				return false;
			}

			IPAddressV4 addr;
			addr[0] = (byte)std::stoi(addressTokens[0], nullptr, 16);
			addr[1] = (byte)std::stoi(addressTokens[1], nullptr, 16);
			addr[2] = (byte)std::stoi(addressTokens[2], nullptr, 16);
			addr[3] = (byte)std::stoi(addressTokens[3], nullptr, 16);

			m_addresses.push_back(addr);
		}

		m_port = (uint16)std::stoi(tokens[tokenIndex++], nullptr, 16);
		m_localKey = std::stoi(tokens[tokenIndex++], nullptr, 16);
		m_remoteKey = std::stoi(tokens[tokenIndex++], nullptr, 16);
	}
	catch (const std::invalid_argument&)
	{
		return false;
	}
	catch (const std::out_of_range&)
	{
		return false;
	}

	return true;
}

XTOOLS_NAMESPACE_END