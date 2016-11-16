//////////////////////////////////////////////////////////////////////////
// ReplaceTransforms.cpp
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ReplaceTransforms.h"
#include "ReplaceOperation.h"

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

TransformedPair ReplaceCreateTransform::Apply(const OperationConstPtr& localOp, const OperationConstPtr& remoteOp) const
{
	XTASSERT(localOp->GetType() == Operation::Replace && remoteOp->GetType() == Operation::Create);

	// By default assume no conflict
	TransformedPair result(localOp, remoteOp);

	// If the operations are on the same element...
	// This is possible if three or more clients try to create the same element at the same time
	if (localOp->GetTargetGuid() == remoteOp->GetTargetGuid())
	{
		const ReplaceOperation* localReplaceOp = static_cast<const ReplaceOperation*>(localOp.get());
		const CreateOperation* remoteCreateOp = static_cast<const CreateOperation*>(remoteOp.get());

		// Use the authority level to determine which op needs to get changed to a ReplaceOperation
		if (remoteOp->GetAuthorityLevel() >= localOp->GetAuthorityLevel())
		{
			result.m_localOpTransformed = localReplaceOp;
			result.m_remoteOpTransformed = NoopOperation::Instance();
		}
		else
		{
			result.m_localOpTransformed = NoopOperation::Instance();
			result.m_remoteOpTransformed = new ReplaceOperation(remoteCreateOp);
		}
	}
	// If the remote target is a descendant of the local target 
	else if (remoteOp->IsDescendantOf(localOp->GetTargetGuid()))
	{
		// The remote Create operation is invalidated by the replacement of its ancestor
		result.m_localOpTransformed = localOp;
		result.m_remoteOpTransformed = NoopOperation::Instance();

	}
	// If the local replace op is a child of the remote create...
	else if (localOp->IsDescendantOf(remoteOp->GetTargetGuid()))
	{
		// Both operations should be starting from the same state, so it should be impossible
		// to replace an element that is a child of the one that is just being created
		XTASSERT(false);
	}

	return result;
}


TransformedPair ReplaceModifyTransform::Apply(const OperationConstPtr& localOp, const OperationConstPtr& remoteOp) const
{
	XTASSERT(localOp->GetType() == Operation::Replace && remoteOp->GetType() == Operation::Modify);

	// By default assume no conflict
	TransformedPair result(localOp, remoteOp);

	// If the operations are on the same element or the modify is on a child of the Replaced element...
	if (localOp->GetTargetGuid() == remoteOp->GetTargetGuid() || remoteOp->IsDescendantOf(localOp->GetTargetGuid()))
	{
		// ... then the Replace trumps the Modify
		result.m_localOpTransformed = localOp;
		result.m_remoteOpTransformed = NoopOperation::Instance();
	}

	return result;
}


TransformedPair ReplaceDeleteTransform::Apply(const OperationConstPtr& localOp, const OperationConstPtr& remoteOp) const
{
	XTASSERT(localOp->GetType() == Operation::Replace && remoteOp->GetType() == Operation::Delete);

	// By default assume no conflict
	TransformedPair result(localOp, remoteOp);

	// If the operations are on the same element...
	if (localOp->GetTargetGuid() == remoteOp->GetTargetGuid())
	{
		const ReplaceOperation* localReplaceOp = static_cast<const ReplaceOperation*>(localOp.get());

		// ... then the Replace gets transformed to a Create, and the Delete is transformed to a no-op
		result.m_localOpTransformed = new CreateOperation(
			localReplaceOp->GetElementType(), 
			localReplaceOp->GetName(), 
			localReplaceOp->GetTargetGuid(), 
			localReplaceOp->GetOwnerID(), 
			localReplaceOp->GetValue(), 
			localReplaceOp->GetAuthorityLevel(), 
			localReplaceOp->GetHierarchy());
		result.m_remoteOpTransformed = NoopOperation::Instance();
	}
	// If the remote target is a descendant of the local target 
	else if (remoteOp->IsDescendantOf(localOp->GetTargetGuid()))
	{
		// The Replace will wipe out the delete
		result.m_localOpTransformed = localOp;
		result.m_remoteOpTransformed = NoopOperation::Instance();
	}
	// If the local replace op is a child of the remote create...
	else if (localOp->IsDescendantOf(remoteOp->GetTargetGuid()))
	{
		// The delete will wipe out the replace
		result.m_localOpTransformed = NoopOperation::Instance();
		result.m_remoteOpTransformed = remoteOp;
	}

	return result;
}


TransformedPair ReplaceReplaceTransform::Apply(const OperationConstPtr& localOp, const OperationConstPtr& remoteOp) const
{
	XTASSERT(localOp->GetType() == Operation::Replace && remoteOp->GetType() == Operation::Replace);

	// By default assume no conflict
	TransformedPair result(localOp, remoteOp);

	// If the operations are on the same element...
	if (localOp->GetTargetGuid() == remoteOp->GetTargetGuid())
	{
		const ReplaceOperation* localReplaceOp = static_cast<const ReplaceOperation*>(localOp.get());
		const ReplaceOperation* remoteReplaceOp = static_cast<const ReplaceOperation*>(remoteOp.get());

		// Check to see if the ops are Replacing the old element with the same type
		if (localReplaceOp->GetElementType() == remoteReplaceOp->GetElementType())
		{
			// The new element type is the same, now check to see if the values are the same
			if (localReplaceOp->GetValue() == remoteReplaceOp->GetValue())
			{
				// No change is necessary in either order
				result.m_localOpTransformed = NoopOperation::Instance();
				result.m_remoteOpTransformed = NoopOperation::Instance();
			}
			else
			{
				// Use the authority level to determine which ops' value should be used in the end
				if (remoteOp->GetAuthorityLevel() >= localOp->GetAuthorityLevel())
				{
					result.m_localOpTransformed = new ModifyOperation(localReplaceOp->GetTargetGuid(), localReplaceOp->GetValue(), localReplaceOp->GetAuthorityLevel(), localReplaceOp->GetHierarchy());
					result.m_remoteOpTransformed = NoopOperation::Instance();
				}
				else
				{
					result.m_localOpTransformed = NoopOperation::Instance();
					result.m_remoteOpTransformed = new ModifyOperation(remoteReplaceOp->GetTargetGuid(), remoteReplaceOp->GetValue(), remoteReplaceOp->GetAuthorityLevel(), remoteReplaceOp->GetHierarchy());
				}
			}
		}
		// The ops are set to create elements of different types. 
		else
		{
			//  Use Authority level to determine which one wins
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
	// If the remote target is a descendant of the local target 
	else if (remoteOp->IsDescendantOf(localOp->GetTargetGuid()))
	{
		// The ancestor will wipe out the descendant
		result.m_localOpTransformed = localOp;
		result.m_remoteOpTransformed = NoopOperation::Instance();
	}
	// If the local replace op is a child of the remote create...
	else if (localOp->IsDescendantOf(remoteOp->GetTargetGuid()))
	{
		// The ancestor will wipe out the descendant
		result.m_localOpTransformed = NoopOperation::Instance();
		result.m_remoteOpTransformed = remoteOp;
	}

	return result;
}

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
