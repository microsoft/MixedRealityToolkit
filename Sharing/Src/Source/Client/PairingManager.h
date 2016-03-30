//////////////////////////////////////////////////////////////////////////
// PairingManager.h
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

/// Starting point for all things pairing related.  Loads a saved pairing
/// definition and uses it to reestablish a previous paired connection or
/// can be used to create a new connection.  
class PairingManager : public AtomicRefCounted, public Reflection::XTObject
{
	XTOOLS_REFLECTION_DECLARE(PairingManager)

public:
	/// Returns true if this device been paired to another.  This does not mean a connection has been established, 
	/// just that it has pairing info that can be used to re-established a connection
	virtual bool			HasPairingInfo() const = 0;

	/// Remove all pairing info, and disconnect from any connected device
	virtual void			ClearPairingInfo() = 0;

	/// Use the cached pairing information to re-establish a network connection to the paired device.
	/// The PairingManager will continue to try to connect until it either succeeds or is canceled.
	/// \param listener Pointer to the object that will be sent notifications if the connection succeeds or fails
	virtual bool			BeginConnecting(PairingListener* listener) = 0;

	/// Stop the current attempt to reestablish a connection with a previously paired client. 
	/// Does nothing if not attempting a connection.  
	virtual void			CancelConnecting() = 0;

	/// Initiate the pairing process using the connection information provided by the given \ref PairMaker.  
	/// \param pairMaker Provides the logic and settings to use for this pairing attempt
	/// \param listener Pointer to the object that will be sent notifications if the connection succeeds or fails
	/// \returns PairingResult::Ok if the attempt was successfully kicked off, other values if unsuccessful
	virtual PairingResult	BeginPairing(const PairMakerPtr& pairMaker, PairingListener* listener) = 0;

	/// Stop the current pairing attempt.  Does nothing if not attempting to pair
	virtual void			CancelPairing() = 0;

	/// Returns true if a pairing attempt is currently in progress
	virtual bool			IsPairing() const = 0;

	/// Returns true if this device has an established network connection to its paired device
	virtual bool			IsConnected() const = 0;
};

DECLARE_PTR(PairingManager)

XTOOLS_NAMESPACE_END
