//////////////////////////////////////////////////////////////////////////
// NetworkOutMessageImpl.h
//
// Implementation of the NetworkOutMessage interface
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class NetworkOutMessageImpl : public NetworkOutMessage
{
public:
	NetworkOutMessageImpl();

	// NetworkOutMessage Functions:
	virtual void Write(byte value) XTOVERRIDE;
	virtual void Write(int16 value) XTOVERRIDE;
	virtual void Write(int32 value) XTOVERRIDE;
	virtual void Write(int64 value) XTOVERRIDE;
	virtual void Write(float value) XTOVERRIDE;
	virtual void Write(double value) XTOVERRIDE;
	virtual void Write(const XStringPtr& value) XTOVERRIDE;
	virtual void WriteArray(const byte* value, uint32 length) XTOVERRIDE;

	virtual void Write(uint16 value) XTOVERRIDE;
	virtual void Write(uint32 value) XTOVERRIDE;
	virtual void Write(uint64 value) XTOVERRIDE;

	// Clear out any data that has been set on this message and prepare it
	// to be used again
	virtual void Reset() XTOVERRIDE;

	virtual byte* GetData() XTOVERRIDE;
	virtual uint32 GetSize() const XTOVERRIDE;

	virtual void Write(const std::string& value) XTOVERRIDE;

private:
	RakNet::BitStream m_stream;
};

DECLARE_PTR(NetworkOutMessageImpl)

XTOOLS_NAMESPACE_END