//////////////////////////////////////////////////////////////////////////
// DirectPairReceiver.h
//
// A PairMaker the puts XTools into listening mode to receive connections 
// from remote clients that want to pair with DirectPairConnectors.  
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class DirectPairReceiver : public PairMaker
{
public:
	DirectPairReceiver();
	explicit DirectPairReceiver(uint16 port);

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

	// Called each time the SharingManager update
	virtual void		Update() XTOVERRIDE;

	// Return true when this object is ready to provide XTools with the information it needs to start 
	// the pairing process.  ie: GetAddress and GetPort will not be called until this returns true.
	virtual bool		IsReadyToConnect() XTOVERRIDE;

	// Local Functions:
	void				SetIncomingPort(uint16 port);

private:
	uint16 m_localPort;
};

XTOOLS_NAMESPACE_END