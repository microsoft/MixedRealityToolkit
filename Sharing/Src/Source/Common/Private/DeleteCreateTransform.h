// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// DeleteCreateTransform.h
// Transforms an incoming Create operation against an applied Delete operation. 
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

class DeleteCreateTransform : public OpTransform
{
public:
	DeleteCreateTransform() {}

	// OpTransform Functions:
	virtual TransformedPair Apply(const OperationConstPtr& localOp, const OperationConstPtr& remoteOp) const XTOVERRIDE;
};

DECLARE_PTR(DeleteCreateTransform)

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
