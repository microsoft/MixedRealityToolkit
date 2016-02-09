//////////////////////////////////////////////////////////////////////////
// CreateModifyTransform.h
//
// Transforms an incoming Modify operation against an applied Create operation.  
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

class CreateModifyTransform : public OpTransform
{
public:
	CreateModifyTransform() {}

	// OpTransform Functions:
	virtual TransformedPair Apply(const OperationConstPtr& localOp, const OperationConstPtr& remoteOp) const XTOVERRIDE;
};

DECLARE_PTR(CreateModifyTransform)

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END