//////////////////////////////////////////////////////////////////////////
// CreateModifyTransform.cpp
//
// Transforms an incoming Modify operation against an applied Create operation.  
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CreateModifyTransform.h"
#include "NoopOperation.h"
#include "CreateOperation.h"
#include "ModifyOperation.h"

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

TransformedPair CreateModifyTransform::Apply(const OperationConstPtr& localOp, const OperationConstPtr& remoteOp) const
{
	// This operation should only be applied to pairs of Create-modify
	XTASSERT(localOp->GetType() == Operation::Create && remoteOp->GetType() == Operation::Modify);

	// By default assume no conflict
	TransformedPair result(localOp, remoteOp);

	// If the operations are on the same element...
	if (localOp->GetTargetGuid() == remoteOp->GetTargetGuid())
	{
		// Both operations should be starting from the same state, so it should be impossible
		// to be modifying an element that is just being created
		XTASSERT(false);
	}

	return result;
}

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END