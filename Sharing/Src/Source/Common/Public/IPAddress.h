// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// IPAddress.h
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class IPAddressV4
{
public:
	// Defaults to 0.0.0.0
	IPAddressV4();
	IPAddressV4(const IPAddressV4& rhs);
	explicit IPAddressV4(const std::string& address);
	
	IPAddressV4& operator=(const IPAddressV4& rhs);

	byte& operator[](int32 index);
	byte operator[](int32 index) const;

	std::string ToString() const;

private:
	byte m_values[4];
};

typedef std::vector<IPAddressV4> IPAddressList;

XTOOLS_NAMESPACE_END
