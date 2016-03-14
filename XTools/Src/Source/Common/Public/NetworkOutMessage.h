//////////////////////////////////////////////////////////////////////////
// NetworkOutMessage.h
//
// Interface for creating messages to send to remote machines
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class NetworkOutMessage : public AtomicRefCounted
{
public:
	virtual void Write(byte value) = 0;
	virtual void Write(int16 value) = 0;
	virtual void Write(int32 value) = 0;
	virtual void Write(int64 value) = 0;
	virtual void Write(float value) = 0;
	virtual void Write(double value) = 0;
	virtual void Write(const XStringPtr& value) = 0;
	virtual void WriteArray(const byte* data, uint32 length) = 0;

	// Clear out any data that has been set on this message and prepare it
	// to be used again
	virtual void Reset() = 0;

	//////////////////////////////////////////////////////////////////////////
	// SWIG hidden functions:

	// Java has not support for unsigned primitives, 
	// so these functions are set to be ignored by SWIG in Common.i
	virtual void Write(uint16 value) = 0;
	virtual void Write(uint32 value) = 0;
	virtual void Write(uint64 value) = 0;

	virtual byte* GetData() = 0;
	virtual uint32 GetSize() const = 0;
	virtual void Write(const std::string& value) = 0;
};

DECLARE_PTR(NetworkOutMessage)

XTOOLS_NAMESPACE_END