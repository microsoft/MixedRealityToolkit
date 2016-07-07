// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// DeleteModifyTransform.cpp
// Transforms an incoming Modify operation against an applied Delete operation. 
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DeleteModifyTransform.h"
#include "NoopOperation.h"
#include "DeleteOperation.h"

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

TransformedPair DeleteModifyTransform::Apply(const OperationConstPtr& localOp, const OperationConstPtr& remoteOp) const
{
	// This operation should only be applied against a certain pair of operations
	XTASSERT(localOp->GetType() == Operation::Delete);

	// By default assume no conflict
	TransformedPair result(localOp, remoteOp);

	// If the operations are on the same element or the local op is deleting an ancestor of the modified op,
	// then the modify gets ignored
	if (localOp->GetTargetGuid() == remoteOp->GetTargetGuid() ||
		remoteOp->IsDescendantOf(localOp->GetTargetGuid()))
	{
		result.m_localOpTransformed = localOp;
		result.m_remoteOpTransformed = NoopOperation::Instance();
	}

	return result;
}

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
