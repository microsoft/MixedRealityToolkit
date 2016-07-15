// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// CreateCreateTransform.h
// Transforms an incoming Create operation against an applied Create operation.  
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

class CreateCreateTransform : public OpTransform
{
public:
	CreateCreateTransform() {}

	// OpTransform Functions:
	virtual TransformedPair Apply(const OperationConstPtr& localOp, const OperationConstPtr& remoteOp) const XTOVERRIDE;
};

DECLARE_PTR(CreateCreateTransform)

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
