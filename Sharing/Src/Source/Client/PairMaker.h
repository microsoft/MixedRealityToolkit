//////////////////////////////////////////////////////////////////////////
// PairMaker.h
//
// Base class for logic that figures out the connection information for a
// remote client that we want to pair to.  
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

// This allows C++ calls to these functions to cause the Java or C# code overloading the function to execute
#if defined(SWIG)
%feature("director") PairMaker;
#endif

// Disable unused parameter warnings.  Pure virtual functions don't play nice with SWIG, but we still want 
// to see the names of the parameters to make reading the code clearer
#pragma warning( push )
#pragma warning( disable : 4100 ) 

class PairMaker : public AtomicRefCounted
{
public:
	// Return true if the local machine will be receiving the pairing connection.
	// Return false if it will be initiating the pairing connection
	virtual bool		IsReceiver() XTABSTRACT { return false; }

	// Return the number of addresses test when attempting to connect
	// Only used if IsReceiver() returns false
	virtual int32		GetAddressCount() XTABSTRACT = 0;

	// Return one of the address of the remote client to try to connect to.
	// Only used if IsReceiver() returns false
	virtual XStringPtr	GetAddress(int32 index) XTABSTRACT { return nullptr; }

	// If receiving the pairing connection, returns the port to listen on.
	// If initiating the pairing connection, returns the port on the remote client to connect to
	virtual uint16		GetPort() XTABSTRACT { return 0; }

	// Called each time the SharingManager update
	virtual void		Update() {}

	// Return true when this object is ready to provide XTools with the information it needs to start 
	// the pairing process.  ie: GetAddress and GetPort will not be called until this returns true.
	virtual bool		IsReadyToConnect() XTABSTRACT { return false; }

	// Returns the key to send the remote client when validating the pairing connection.  
	// If no such check is necessary, return 0
	virtual int32		GetLocalKey() { return 0; }

	// Returns the key that should be received from the remote machine when validating the pairing connection.  
	// If the key received from the remote client does not match this key then the pairing will fail.  
	// If no such check is necessary, return 0
	virtual int32		GetRemoteKey() { return 0; }
};

DECLARE_PTR(PairMaker)

#pragma warning( pop )

XTOOLS_NAMESPACE_END