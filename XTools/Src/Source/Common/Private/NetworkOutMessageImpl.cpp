//////////////////////////////////////////////////////////////////////////
// NetworkOutMessageImpl.cpp
//
// Implementation of the NetworkOutMessage interface
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NetworkOutMessageImpl.h"
#include "NetworkInMessageImpl.h"

XTOOLS_NAMESPACE_BEGIN

const int DefaultMessageSize = 100; // bytes pre-allocated for the message

NetworkOutMessageImpl::NetworkOutMessageImpl()
	: m_stream(DefaultMessageSize)
{

}


void NetworkOutMessageImpl::Write(byte value)
{
	m_stream.Write(value);
}

void NetworkOutMessageImpl::Write(int16 value)
{
	m_stream.Write(value);
}


void NetworkOutMessageImpl::Write(int32 value)
{
	m_stream.Write(value);
}


void NetworkOutMessageImpl::Write(int64 value)
{
	m_stream.Write(value);
}


void NetworkOutMessageImpl::Write(float value)
{
	m_stream.Write(value);
}


void NetworkOutMessageImpl::Write(double value)
{
	m_stream.Write(value);
}


void NetworkOutMessageImpl::Write(const XStringPtr& value)
{
	if (value)
	{
		m_stream.Write(RakNet::RakString(value->GetString().c_str()));
	}
	else
	{
		m_stream.Write(RakNet::RakString(""));
	}
}


void NetworkOutMessageImpl::WriteArray(const byte* value, uint32 length)
{
	m_stream.WriteAlignedBytes(value, length);
}


void NetworkOutMessageImpl::Write(uint16 value)
{
	m_stream.Write(value);
}


void NetworkOutMessageImpl::Write(uint32 value)
{
	m_stream.Write(value);
}


void NetworkOutMessageImpl::Write(uint64 value)
{
	m_stream.Write(value);
}


byte* NetworkOutMessageImpl::GetData()
{
	return m_stream.GetData();
}


uint32 NetworkOutMessageImpl::GetSize() const
{
	return m_stream.GetNumberOfBytesUsed();
}


void NetworkOutMessageImpl::Write(const std::string& value)
{
	m_stream.Write(RakNet::RakString(value.c_str()));
}


void NetworkOutMessageImpl::Reset()
{
	m_stream.Reset();
}


XTOOLS_NAMESPACE_END
