// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// PairingResult.h
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
