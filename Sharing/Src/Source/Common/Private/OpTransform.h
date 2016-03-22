//////////////////////////////////////////////////////////////////////////
// OpTransform.h
//
// Base class for logic that transforms incoming operations against 
// applied operation
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

struct TransformedPair
{
	TransformedPair();
	TransformedPair(const OperationConstPtr& local, const OperationConstPtr& remote)
		: m_localOpTransformed(local)
		, m_remoteOpTransformed(remote) {}

	OperationConstPtr m_localOpTransformed;
	OperationConstPtr m_remoteOpTransformed;
};


class OpTransform : public RefCounted
{
public:
	
	// The properties of the return value must be that, from a common starting state:
	// applying localOp then m_remoteOpTransformed yields the same state as applying remoteOp then m_localOpTransformed
	virtual TransformedPair Apply(const OperationConstPtr& localOp, const OperationConstPtr& remoteOp) const = 0;
};

DECLARE_PTR(OpTransform)

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END