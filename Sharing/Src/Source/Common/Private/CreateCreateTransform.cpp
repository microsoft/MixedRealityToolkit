// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// CreateCreateTransform.cpp
// Transforms an incoming Create operation against an applied Create operation.  
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CreateCreateTransform.h"
#include "NoopOperation.h"
#include "CreateOperation.h"
#include "ModifyOperation.h"

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

TransformedPair CreateCreateTransform::Apply(const OperationConstPtr& localOp, const OperationConstPtr& remoteOp) const
{
	// This operation should only be applied to pairs of Create operations
	XTASSERT(localOp->GetType() == Operation::Create && remoteOp->GetType() == Operation::Create);

	// By default assume no conflict
	TransformedPair result(localOp, remoteOp);

	// If the operations are on the same element...
	if (localOp->GetTargetGuid() == remoteOp->GetTargetGuid())
	{
		const CreateOperation* localCreateOp = static_cast<const CreateOperation*>(localOp.get());
		const CreateOperation* remoteCreateOp = static_cast<const CreateOperation*>(remoteOp.get());

		// Make sure that the created element is the same type for both operations.  Converting types is not supported
		XTASSERT(localCreateOp->GetValue().GetType() == remoteCreateOp->GetValue().GetType());

		// If the values of the create elements are the same...
		if (localCreateOp->GetValue() == remoteCreateOp->GetValue())
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
				result.m_localOpTransformed = new ModifyOperation(localCreateOp);
				result.m_remoteOpTransformed = NoopOperation::Instance();
			}
			else
			{
				result.m_localOpTransformed = NoopOperation::Instance();
				result.m_remoteOpTransformed = new ModifyOperation(remoteCreateOp);
			}
		}
	}

	return result;
}

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
