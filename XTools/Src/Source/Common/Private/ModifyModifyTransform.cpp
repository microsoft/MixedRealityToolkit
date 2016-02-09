//////////////////////////////////////////////////////////////////////////
// ModifyModifyTransform.cpp
//
// Transforms an incoming Modify operation against an applied Modify operation.  
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ModifyModifyTransform.h"
#include "NoopOperation.h"
#include "ModifyOperation.h"

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

TransformedPair ModifyModifyTransform::Apply(const OperationConstPtr& localOp, const OperationConstPtr& remoteOp) const
{
	// This operation should only be applied to pairs of Modify operations
	XTASSERT(localOp->GetType() == Operation::Modify && remoteOp->GetType() == Operation::Modify);

	// By default assume no conflict
	TransformedPair result(localOp, remoteOp);

	// If the operations are on different elements then no fix-up is required
	if (localOp->GetTargetGuid() == remoteOp->GetTargetGuid())
	{
		const ModifyOperation* localModifyOp = static_cast<const ModifyOperation*>(localOp.get());
		const ModifyOperation* remoteModifyOp = static_cast<const ModifyOperation*>(remoteOp.get());

		// Make sure that the element type is the same for both operations.  Converting types is not supported
		XTASSERT(localModifyOp->GetValue().GetType() == remoteModifyOp->GetValue().GetType());

		// If the values of the ops are the same...
		if (localModifyOp->GetValue() == remoteModifyOp->GetValue())
		{
			// No change is necessary in either order
			result.m_localOpTransformed = NoopOperation::Instance();
			result.m_remoteOpTransformed = NoopOperation::Instance();
		}
		else
		{
			// Use the authority level to determine which ops value should be used in the end
			if (remoteOp->GetAuthorityLevel() >= localOp->GetAuthorityLevel())
			{
				result.m_localOpTransformed = localOp;
				result.m_remoteOpTransformed = NoopOperation::Instance();
			}
			else
			{
				result.m_localOpTransformed = NoopOperation::Instance();
				result.m_remoteOpTransformed = remoteOp;
			}
		}
	}

	return result;
}

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
