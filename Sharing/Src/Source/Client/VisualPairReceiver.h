// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// VisualPairReceiver.h
// Generates an image that encodes information about how to connect to the
// local machine, then prepares XTools to receive incoming pairing connections
// from remote clients using that information
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

DECLARE_PTR_PRE(VisualPairReceiver)

class VisualPairReceiver : public PairMaker
{
public:
	// Create a new VisualPairReceiver
	static ref_ptr<VisualPairReceiver> Create();

	// Generate a pairing tag image that encodes the data necessary to connect to this machine
	virtual TagImagePtr CreateTagImage() const = 0;
};

DECLARE_PTR_POST(VisualPairReceiver)

XTOOLS_NAMESPACE_END
