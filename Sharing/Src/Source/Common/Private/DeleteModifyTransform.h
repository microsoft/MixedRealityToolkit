// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// DeleteModifyTransform.h
// Transforms an incoming Modify operation against an applied Delete operation. 
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

class DeleteModifyTransform : public OpTransform
{
public:
	DeleteModifyTransform() {}

	// OpTransform Functions:
	virtual TransformedPair Apply(const OperationConstPtr& localOp, const OperationConstPtr& remoteOp) const XTOVERRIDE;
};

DECLARE_PTR(DeleteModifyTransform)

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
