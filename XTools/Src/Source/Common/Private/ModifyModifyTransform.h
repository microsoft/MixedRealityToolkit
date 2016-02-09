//////////////////////////////////////////////////////////////////////////
// ModifyModifyTransform.h
//
// Transforms an incoming Modify operation against an applied Modify operation.  
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

class ModifyModifyTransform : public OpTransform
{
public:
	ModifyModifyTransform() {}

	// OpTransform Functions:
	virtual TransformedPair Apply(const OperationConstPtr& localOp, const OperationConstPtr& remoteOp) const XTOVERRIDE;
};

DECLARE_PTR(ModifyModifyTransform)

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END