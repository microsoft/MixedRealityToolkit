//////////////////////////////////////////////////////////////////////////
// CreateCreateTransform.h
//
// Transforms an incoming Create operation against an applied Create operation.  
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
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