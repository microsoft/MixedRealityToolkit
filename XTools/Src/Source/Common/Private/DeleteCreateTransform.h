//////////////////////////////////////////////////////////////////////////
// DeleteCreateTransform.h
//
// Transforms an incoming Create operation against an applied Delete operation. 
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
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
