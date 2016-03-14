//////////////////////////////////////////////////////////////////////////
// PairingResult.h
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

/// The list of results that the pairing management classes can return to the user
enum PairingResult : int32
{
	Ok = 0,
	CanceledByUser,
	FailedToOpenIncomingConnection,
	FailedToOpenOutgoingConnection,
	PairingAlreadyInProgress,
	ConnectionFailed,
	NoAddressToConnectTo
};

XTOOLS_NAMESPACE_END
