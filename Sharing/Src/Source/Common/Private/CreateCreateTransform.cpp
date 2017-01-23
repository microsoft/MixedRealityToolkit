//////////////////////////////////////////////////////////////////////////
// CreateCreateTransform.cpp
//
// Transforms an incoming Create operation against an applied Create operation.  
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CreateCreateTransform.h"
#include "NoopOperation.h"
#include "CreateOperation.h"
#include "ReplaceOperation.h"

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

		// Check to see if the operations are trying to create fundamentally different elements
		if (localCreateOp->GetElementType() != remoteCreateOp->GetElementType() ||	// Elements are different types
			(localCreateOp->GetElementType() == ElementType::ObjectType && localCreateOp->GetValue() != remoteCreateOp->GetValue()) // ObjectElements represent different class types
			)
		{
			// Use the authority level to determine which op needs to get changed to a ReplaceOperation
			if (remoteOp->GetAuthorityLevel() >= localOp->GetAuthorityLevel())
			{
				result.m_localOpTransformed = new ReplaceOperation(localCreateOp);
				result.m_remoteOpTransformed = NoopOperation::Instance();
			}
			else
			{
				result.m_localOpTransformed = NoopOperation::Instance();
				result.m_remoteOpTransformed = new ReplaceOperation(remoteCreateOp);
			}
		}
		// The elements are the same type.  Check to see if they have different starting values
		else
		{
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
	}

	return result;
}

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END