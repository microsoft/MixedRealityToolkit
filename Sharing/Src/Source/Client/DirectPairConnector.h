//////////////////////////////////////////////////////////////////////////
// DirectPairConnector.h
//
// A PairMaker that initiates a pairing to a remote client via a direct connection
// given the remote client machine's name or IP address
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class DirectPairConnector : public PairMaker
{
public:
	DirectPairConnector();
	explicit DirectPairConnector(const XStringPtr& remoteNameOrIP);
	DirectPairConnector(const XStringPtr& remoteNameOrIP, uint16 port);

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

	// Set the IP address or domain name of the remote client to connect to
	void				SetRemoteAddress(const XStringPtr& remoteNameOrIP);

	// Set the port of the remote client to connect to.  
	// Optional; if not set, will use the default XTools client pairing port
	void				SetRemotePort(uint16 port);

private:
	XStringPtr	m_remoteName;
	uint16		m_remotePort;
};

XTOOLS_NAMESPACE_END