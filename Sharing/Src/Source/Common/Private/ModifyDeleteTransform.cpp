// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// ModifyDeleteTransform.cpp
// Transforms an incoming Delete operation against an applied Modify operation. 
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ModifyDeleteTransform.h"
#include "NoopOperation.h"
#include "DeleteOperation.h"
#include "ModifyOperation.h"

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

TransformedPair ModifyDeleteTransform::Apply(const OperationConstPtr& localOp, const OperationConstPtr& remoteOp) const
{
	// This operation should only be applied against a certain pair of operations
	XTASSERT(localOp->GetType() == Operation::Modify && remoteOp->GetType() == Operation::Delete);

	// By default assume no conflict
	TransformedPair result(localOp, remoteOp);

	// If the operations are on the same element or the remote op is deleting an ancestor of the modified op,
	// then the modify gets ignored
	if (localOp->GetTargetGuid() == remoteOp->GetTargetGuid() ||
		localOp->IsDescendantOf(remoteOp->GetTargetGuid()))
	{
		result.m_localOpTransformed = NoopOperation::Instance();
		result.m_remoteOpTransformed = remoteOp;
	}

	return result;
}

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
