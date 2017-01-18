//////////////////////////////////////////////////////////////////////////
// ReplaceTransforms.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

struct ReplaceCreateTransform : public OpTransform
{
	virtual TransformedPair Apply(const OperationConstPtr& localOp, const OperationConstPtr& remoteOp) const XTOVERRIDE;
};

struct ReplaceModifyTransform : public OpTransform
{
	virtual TransformedPair Apply(const OperationConstPtr& localOp, const OperationConstPtr& remoteOp) const XTOVERRIDE;
};

struct ReplaceDeleteTransform : public OpTransform
{
	virtual TransformedPair Apply(const OperationConstPtr& localOp, const OperationConstPtr& remoteOp) const XTOVERRIDE;
};

struct ReplaceReplaceTransform : public OpTransform
{
	virtual TransformedPair Apply(const OperationConstPtr& localOp, const OperationConstPtr& remoteOp) const XTOVERRIDE;
};

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
