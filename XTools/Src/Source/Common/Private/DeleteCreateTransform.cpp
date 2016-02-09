//////////////////////////////////////////////////////////////////////////
// DeleteCreateTransform.cpp
//
// Transforms an incoming Create operation against an applied Delete operation. 
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DeleteCreateTransform.h"
#include "NoopOperation.h"
#include "DeleteOperation.h"
#include "CreateOperation.h"

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

TransformedPair DeleteCreateTransform::Apply(const OperationConstPtr& localOp, const OperationConstPtr& remoteOp) const
{
	// This operation should only be applied against a certain pair of operations
	XTASSERT(localOp->GetType() == Operation::Delete && remoteOp->GetType() == Operation::Create);

	// By default assume no conflict
	TransformedPair result(localOp, remoteOp);

	// If the operations are on the same element...
	if (localOp->GetTargetGuid() == remoteOp->GetTargetGuid())
	{
		// Both operations should be starting from the same state, so it should be impossible
		// to delete an element that is just being created
		XTASSERT(false);
	}
	// If the remote target is a descendant of the local target 
	else if (remoteOp->IsDescendantOf(localOp->GetTargetGuid()))
	{
		result.m_localOpTransformed = localOp;
		result.m_remoteOpTransformed = NoopOperation::Instance();
	}
	// If the local delete op is a child of the remote create...
	else if (localOp->IsDescendantOf(remoteOp->GetTargetGuid()))
	{
		// Both operations should be starting from the same state, so it should be impossible
		// to delete an element that is a child of the one that is just being created
		XTASSERT(false);
	}

	return result;
}

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
