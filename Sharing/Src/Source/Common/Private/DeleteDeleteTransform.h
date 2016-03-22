//////////////////////////////////////////////////////////////////////////
// DeleteDeleteTransform.h
//
// Transforms an incoming Delete operation against an applied Delete operation. 
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
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
