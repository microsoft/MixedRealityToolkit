//////////////////////////////////////////////////////////////////////////
// DeleteDeleteTransform.cpp
//
// Transforms an incoming Delete operation against an applied Delete operation. 
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DeleteDeleteTransform.h"
#include "NoopOperation.h"
#include "DeleteOperation.h"

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

TransformedPair DeleteDeleteTransform::Apply(const OperationConstPtr& localOp, const OperationConstPtr& remoteOp) const
{
	// This operation should only be applied against a certain pair of operations
	XTASSERT(localOp->GetType() == Operation::Delete && remoteOp->GetType() == Operation::Delete);

	TransformedPair result(localOp, remoteOp);

	// If the operations are targeting the same elements
	if (localOp->GetTargetGuid() == remoteOp->GetTargetGuid())
	{
		result.m_localOpTransformed = NoopOperation::Instance();
		result.m_remoteOpTransformed = NoopOperation::Instance();
	}
	// If the remote target is a descendant of the local target 
	else if (remoteOp->IsDescendantOf(localOp->GetTargetGuid()))
	{
		result.m_localOpTransformed = localOp;
		result.m_remoteOpTransformed = NoopOperation::Instance();
	}
	else if (localOp->IsDescendantOf(remoteOp->GetTargetGuid()))
	{
		result.m_localOpTransformed = NoopOperation::Instance();
		result.m_remoteOpTransformed = remoteOp;
	}

	return result;
}

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END