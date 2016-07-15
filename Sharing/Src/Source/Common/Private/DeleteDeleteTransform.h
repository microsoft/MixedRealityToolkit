// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// DeleteDeleteTransform.h
// Transforms an incoming Delete operation against an applied Delete operation. 
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

class DeleteDeleteTransform : public OpTransform
{
public:
	DeleteDeleteTransform() {}

	// OpTransform Functions:
	virtual TransformedPair Apply(const OperationConstPtr& localOp, const OperationConstPtr& remoteOp) const XTOVERRIDE;
};

DECLARE_PTR(DeleteDeleteTransform)

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
