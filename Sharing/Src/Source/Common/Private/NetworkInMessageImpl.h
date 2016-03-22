//////////////////////////////////////////////////////////////////////////
// NetworkInMessageImpl.h
//
// Implementation of the NetworkInMessage interface
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class NetworkInMessageImpl : public NetworkInMessage
{
public:
	NetworkInMessageImpl(const byte* message, unsigned int length);

	// NetworkInMessage Functions:
	virtual byte ReadByte() XTOVERRIDE;
	virtual int16 ReadInt16() XTOVERRIDE;
	virtual int32 ReadInt32() XTOVERRIDE;
	virtual int64 ReadInt64() XTOVERRIDE;
	virtual float ReadFloat() XTOVERRIDE;
	virtual double ReadDouble() XTOVERRIDE;
	virtual XStringPtr ReadString() XTOVERRIDE;
	virtual void ReadArray(byte* arrayOut, uint32 arrayLength) XTOVERRIDE;

	

	// Returns the number of bytes in this message that how not yet been read
	virtual int32 GetUnreadBitsCount() const XTOVERRIDE;

	// Returns the total size of this message
	virtual int32 GetSize() const XTOVERRIDE;

	//////////////////////////////////////////////////////////////////////////
	// SWIG hidden

	virtual uint16 ReadUInt16() XTOVERRIDE;
	virtual uint32 ReadUInt32() XTOVERRIDE;
	virtual uint64 ReadUInt64() XTOVERRIDE;

	virtual std::string ReadStdString() XTOVERRIDE;

	virtual byte* GetData() const XTOVERRIDE;

	const RakNet::BitStream& GetBitStream() const { return m_stream; }

private:
	// Prevent automatic copy construction
	NetworkInMessageImpl(const NetworkInMessageImpl& rhs);
	NetworkInMessageImpl& operator=(const NetworkInMessageImpl& rhs);

	RakNet::BitStream	m_stream;
};

XTOOLS_NAMESPACE_END