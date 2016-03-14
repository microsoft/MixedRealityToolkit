//////////////////////////////////////////////////////////////////////////
// NetworkInMessageImpl.cpp
//
// Implementation of the NetworkInMessage interface
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NetworkInMessageImpl.h"

XTOOLS_NAMESPACE_BEGIN

NetworkInMessageImpl::NetworkInMessageImpl(const byte* message, unsigned int length)
: m_stream(const_cast<byte*>(message), length, false)	// Hate that we have to const-cast here, but RakNet isn't completely const correct
{
	 
}


byte NetworkInMessageImpl::ReadByte()
{
	byte value;
	XTVERIFY(m_stream.Read(value));
	return value;
}


int16 NetworkInMessageImpl::ReadInt16()
{
	int16 value;
	XTVERIFY(m_stream.Read(value));
	return value;
}


int32 NetworkInMessageImpl::ReadInt32()
{
	int32 value;
	XTVERIFY(m_stream.Read(value));
	return value;
}


int64 NetworkInMessageImpl::ReadInt64()
{
	int64 value;
	XTVERIFY(m_stream.Read(value));
	return value;
}


float NetworkInMessageImpl::ReadFloat()
{
	float value;
	XTVERIFY(m_stream.Read(value));
	return value;
}


double NetworkInMessageImpl::ReadDouble()
{
	double value;
	XTVERIFY(m_stream.Read(value));
	return value;
}


XStringPtr NetworkInMessageImpl::ReadString()
{
	RakNet::RakString value;
	XTVERIFY(m_stream.Read(value));
	return new XString(value.C_String());
}


void NetworkInMessageImpl::ReadArray(byte* arrayOut, uint32 arrayLength)
{
	XTVERIFY(m_stream.ReadAlignedBytes(arrayOut, arrayLength));
}


uint16 NetworkInMessageImpl::ReadUInt16()
{
	uint16 value;
	XTVERIFY(m_stream.Read(value));
	return value;
}


uint32 NetworkInMessageImpl::ReadUInt32()
{
	uint32 value;
	XTVERIFY(m_stream.Read(value));
	return value;
}


uint64 NetworkInMessageImpl::ReadUInt64()
{
	uint64 value;
	XTVERIFY(m_stream.Read(value));
	return value;
}


std::string NetworkInMessageImpl::ReadStdString()
{
	RakNet::RakString value;
	XTVERIFY(m_stream.Read(value));
	return value.C_String();
}


int32 NetworkInMessageImpl::GetUnreadBitsCount() const
{
	return static_cast<int32>(m_stream.GetNumberOfUnreadBits());
}


int32 NetworkInMessageImpl::GetSize() const
{
	return static_cast<int32>(m_stream.GetNumberOfBytesUsed());
}


byte* NetworkInMessageImpl::GetData() const
{
	return m_stream.GetData();
}

XTOOLS_NAMESPACE_END