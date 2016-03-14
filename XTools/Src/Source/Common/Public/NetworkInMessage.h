//////////////////////////////////////////////////////////////////////////
// NetworkInMessage.h
//
// Interface for reading messages to send from remote machines
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once


XTOOLS_NAMESPACE_BEGIN

class NetworkInMessage
{
public:
	virtual ~NetworkInMessage() { }
	virtual byte ReadByte() = 0;
	virtual int16 ReadInt16() = 0;
	virtual int32 ReadInt32() = 0;
	virtual int64 ReadInt64() = 0;
	virtual float ReadFloat() = 0;
	virtual double ReadDouble() = 0;
	virtual XStringPtr ReadString() = 0;
	virtual void ReadArray(byte* data, uint32 arrayLength) = 0;

	// Returns the number of bytes in this message that how not yet been read
	virtual int32 GetUnreadBitsCount() const = 0;

	// Returns the total size of this message
	virtual int32 GetSize() const = 0;

	//////////////////////////////////////////////////////////////////////////
	// SWIG hidden
	// Java has not support for unsigned primitives, 
	// so these functions are set to be ignored by SWIG in Common.i
	virtual uint16 ReadUInt16() = 0;
	virtual uint32 ReadUInt32() = 0;
	virtual uint64 ReadUInt64() = 0;
	virtual std::string ReadStdString() = 0;

	virtual byte* GetData() const = 0;
};

XTOOLS_NAMESPACE_END

