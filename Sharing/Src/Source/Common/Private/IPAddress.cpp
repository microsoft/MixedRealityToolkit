//////////////////////////////////////////////////////////////////////////
// IPAddress.cpp
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Public/IPAddress.h>

XTOOLS_NAMESPACE_BEGIN

IPAddressV4::IPAddressV4()
{
	m_values[0] = 0;
	m_values[1] = 0;
	m_values[2] = 0;
	m_values[3] = 0;
}


IPAddressV4::IPAddressV4(const IPAddressV4& rhs)
{
	m_values[0] = rhs.m_values[0];
	m_values[1] = rhs.m_values[1];
	m_values[2] = rhs.m_values[2];
	m_values[3] = rhs.m_values[3];
}


IPAddressV4::IPAddressV4(const std::string& address)
{
	m_values[0] = 0;
	m_values[1] = 0;
	m_values[2] = 0;
	m_values[3] = 0;

	std::vector<std::string> tokens = StringUtils::Tokenize(address.c_str(), '.');
	if (tokens.size() == 4)
	{
		try
		{
			for (int i = 0; i < 4; ++i)
			{
				m_values[i] = (byte)std::stoi(tokens[i], nullptr, 10);
			}
		}
		catch (...)
		{
			LogError("Failed to load IP Address string: %s", address.c_str());
		}
	}
}


IPAddressV4& IPAddressV4::operator=(const IPAddressV4& rhs)
{
	m_values[0] = rhs.m_values[0];
	m_values[1] = rhs.m_values[1];
	m_values[2] = rhs.m_values[2];
	m_values[3] = rhs.m_values[3];

	return *this;
}


byte& IPAddressV4::operator[](int32 index)
{
	XTASSERT(index >= 0 && index < 4);
	return m_values[index];
}


byte IPAddressV4::operator[](int32 index) const
{
	XTASSERT(index >= 0 && index < 4);
	return m_values[index];
}


std::string IPAddressV4::ToString() const
{
	char buffer[22];

#if defined(XTOOLS_PLATFORM_WINDOWS_ANY)
	sprintf_s(buffer, sizeof(buffer), "%i.%i.%i.%i", m_values[0], m_values[1], m_values[2], m_values[3]);
#else
	sprintf_s(buffer, sizeof(buffer), "%i.%i.%i.%i", m_values[0], m_values[1], m_values[2], m_values[3]);
#endif

	return std::string(buffer);
}

XTOOLS_NAMESPACE_END
