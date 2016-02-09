//////////////////////////////////////////////////////////////////////////
// CreateDeleteTransform.cpp
//
// Transforms an incoming Delete operation against an applied Create operation.  
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CreateDeleteTransform.h"
#include "NoopOperation.h"
#include "CreateOperation.h"
#include "DeleteOperation.h"

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

TransformedPair CreateDeleteTransform::Apply(const OperationConstPtr& localOp, const OperationConstPtr& remoteOp) const
{
	// This operation should only be applied against a certain pair of operations
	XTASSERT(localOp->GetType() == Operation::Create && remoteOp->GetType() == Operation::Delete);

	// By default assume no conflict
	TransformedPair result(localOp, remoteOp);

	const CreateOperation* localCreateOp = static_cast<const CreateOperation*>(localOp.get());
	const DeleteOperation* remoteDeleteOp = static_cast<const DeleteOperation*>(remoteOp.get());

	// If the operations are on the same element...
	if (localOp->GetTargetGuid() == remoteOp->GetTargetGuid())
	{
		// Both operations should be starting from the same state, so it should be impossible
		// to delete an element that is just being created
		XTASSERT(false);
	}
	// If the remote target is a descendant of the local target 
	else if (remoteDeleteOp->IsDescendantOf(localOp->GetTargetGuid()))
	{
		// Both operations should be starting from the same state, so it should be impossible
		// to delete an element that is a child of the one that is just being created
		XTASSERT(false);
	}
	// If the create op is a child of the delete, then the create is invalid
	else if (localCreateOp->IsDescendantOf(remoteDeleteOp->GetTargetGuid()))
	{
		result.m_localOpTransformed = NoopOperation::Instance();
		result.m_remoteOpTransformed = remoteOp;
	}

	return result;
}

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
