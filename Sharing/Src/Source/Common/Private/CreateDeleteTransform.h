//////////////////////////////////////////////////////////////////////////
// CreateDeleteTransform.h
//
// Transforms an incoming Delete operation against an applied Create operation.  
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

class CreateDeleteTransform : public OpTransform
{
public:
	CreateDeleteTransform() {}

	// OpTransform Functions:
	virtual TransformedPair Apply(const OperationConstPtr& localOp, const OperationConstPtr& remoteOp) const XTOVERRIDE;
};

DECLARE_PTR(CreateDeleteTransform)

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END