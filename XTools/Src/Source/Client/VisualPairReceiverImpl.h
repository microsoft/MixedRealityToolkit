//////////////////////////////////////////////////////////////////////////
// VisualPairReceiverImpl.h
//
// 
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "PairingInfo.h"

XTOOLS_NAMESPACE_BEGIN

class VisualPairReceiverImpl : public VisualPairReceiver
{
public:
	VisualPairReceiverImpl();

	// PairMaker Functions:
	// Return true if the local machine will be receiving the pairing connection.
	// Return false if it will be initiating the pairing connection
	virtual bool		IsReceiver() XTOVERRIDE;

	// Return the number of addresses test when attempting to connect
	// Only used if IsReceiver() returns false
	virtual int32		GetAddressCount() XTOVERRIDE;

	// Return one of the address of the remote client to try to connect to.
	// Only used if IsReceiver() returns false
	virtual XStringPtr	GetAddress(int32 index) XTOVERRIDE;

	// If receiving the pairing connection, returns the port to listen on.
	// If initiating the pairing connection, returns the port on the remote client to connect to
	virtual uint16		GetPort() XTOVERRIDE;

	// Return true when this object is ready to provide XTools with the information it needs to start 
	// the pairing process.  ie: GetAddress and GetPort will not be called until this returns true.
	virtual bool		IsReadyToConnect() XTOVERRIDE;

	// Returns the key to send the remote client when validating the pairing connection.  
	// If no such check is necessary, return 0
	virtual int32		GetLocalKey() XTOVERRIDE;

	// Returns the key that should be received from the remote machine when validating the pairing connection.  
	// If the key received from the remote client does not match this key then the pairing will fail.  
	// If no such check is necessary, return 0
	virtual int32		GetRemoteKey() XTOVERRIDE;

	// Look up all the information necessary for remote machines to connect to this one,
	// encode that into a TagImage, and set this PairMaker as ready to connect
	virtual TagImagePtr CreateTagImage() const XTOVERRIDE;

private:
	PairingInfo m_pairingInfo;
};

XTOOLS_NAMESPACE_END
