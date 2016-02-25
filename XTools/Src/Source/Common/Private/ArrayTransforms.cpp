//////////////////////////////////////////////////////////////////////////
// ArrayTransforms.cpp
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ArrayTransforms.h"

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

TransformedPair UpdateUpdateTransform::Apply(const OperationConstPtr& localOp, const OperationConstPtr& remoteOp) const
{
	XTASSERT(localOp->GetType() == Operation::Update && remoteOp->GetType() == Operation::Update);

	// By default assume no conflict
	TransformedPair result(localOp, remoteOp);

	// If the operations are on the same array
	if (localOp->GetTargetGuid() == remoteOp->GetTargetGuid())
	{
		const UpdateOperation* localUpdateOp = static_cast<const UpdateOperation*>(localOp.get());
		const UpdateOperation* remoteUpdateOp = static_cast<const UpdateOperation*>(remoteOp.get());

		// Make sure that the element type is the same for both operations.  Converting types is not supported
		XTASSERT(localUpdateOp->GetValue().GetType() == remoteUpdateOp->GetValue().GetType());

		// If the operation is on the same element
		if (localUpdateOp->GetIndex() == remoteUpdateOp->GetIndex())
		{
			// If the values of the ops are the same...
			if (localUpdateOp->GetValue() == remoteUpdateOp->GetValue())
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
	}

	return result;
}


TransformedPair UpdateInsertTransform::Apply(const OperationConstPtr& localOp, const OperationConstPtr& remoteOp) const
{
	XTASSERT(localOp->GetType() == Operation::Update && remoteOp->GetType() == Operation::Insert);

	// By default assume no conflict
	TransformedPair result(localOp, remoteOp);

	// If the operations are on the same array
	if (localOp->GetTargetGuid() == remoteOp->GetTargetGuid())
	{
		const UpdateOperation* updateOp = static_cast<const UpdateOperation*>(localOp.get());
		const InsertOperation* insertOp = static_cast<const InsertOperation*>(remoteOp.get());

		// If the operation is on the same element
		if (updateOp->GetIndex() >= insertOp->GetIndex())
		{
			result.m_localOpTransformed = new UpdateOperation(updateOp->GetTargetGuid(), updateOp->GetIndex() + 1, updateOp->GetValue(), updateOp->GetAuthorityLevel(), updateOp->GetHierarchy());
		}
	}

	return result;
}


TransformedPair UpdateRemoveTransform::Apply(const OperationConstPtr& localOp, const OperationConstPtr& remoteOp) const
{
	XTASSERT(localOp->GetType() == Operation::Update && remoteOp->GetType() == Operation::Remove);

	// By default assume no conflict
	TransformedPair result(localOp, remoteOp);

	// If the operations are on the same array
	if (localOp->GetTargetGuid() == remoteOp->GetTargetGuid())
	{
		const UpdateOperation* updateOp = static_cast<const UpdateOperation*>(localOp.get());
		const RemoveOperation* removeOp = static_cast<const RemoveOperation*>(remoteOp.get());

		if (updateOp->GetIndex() > removeOp->GetIndex())
		{
			result.m_localOpTransformed = new UpdateOperation(updateOp->GetTargetGuid(), updateOp->GetIndex() - 1, updateOp->GetValue(), updateOp->GetAuthorityLevel(), updateOp->GetHierarchy());
		}
		else if (updateOp->GetIndex() == removeOp->GetIndex())
		{
			result.m_localOpTransformed = NoopOperation::Instance();
		}
	}

	return result;
}


TransformedPair InsertInsertTransform::Apply(const OperationConstPtr& localOp, const OperationConstPtr& remoteOp) const
{
	XTASSERT(localOp->GetType() == Operation::Insert && remoteOp->GetType() == Operation::Insert);

	// By default assume no conflict
	TransformedPair result(localOp, remoteOp);

	// If the operations are on the same array
	if (localOp->GetTargetGuid() == remoteOp->GetTargetGuid())
	{
		const InsertOperation* localInsertOp = static_cast<const InsertOperation*>(localOp.get());
		const InsertOperation* remoteInsertOp = static_cast<const InsertOperation*>(remoteOp.get());

		// If the operation is on the same element
		if (localInsertOp->GetIndex() > remoteInsertOp->GetIndex())
		{
			result.m_localOpTransformed = new InsertOperation(
				localInsertOp->GetTargetGuid(), 
				localInsertOp->GetIndex() + 1, 
				localInsertOp->GetValue(), 
				localInsertOp->GetAuthorityLevel(), 
				localInsertOp->GetHierarchy());
		}
		else if (localInsertOp->GetIndex() < remoteInsertOp->GetIndex())
		{
			result.m_remoteOpTransformed = new InsertOperation(
				remoteInsertOp->GetTargetGuid(), 
				remoteInsertOp->GetIndex() + 1, 
				remoteInsertOp->GetValue(), 
				remoteInsertOp->GetAuthorityLevel(), 
				remoteInsertOp->GetHierarchy());
		}
		else
		{
			if (remoteOp->GetAuthorityLevel() >= localOp->GetAuthorityLevel())
			{
				result.m_remoteOpTransformed = new InsertOperation(
					remoteInsertOp->GetTargetGuid(),
					remoteInsertOp->GetIndex() + 1,
					remoteInsertOp->GetValue(),
					remoteInsertOp->GetAuthorityLevel(),
					remoteInsertOp->GetHierarchy());
			}
			else
			{
				result.m_localOpTransformed = new InsertOperation(
					localInsertOp->GetTargetGuid(),
					localInsertOp->GetIndex() + 1,
					localInsertOp->GetValue(),
					localInsertOp->GetAuthorityLevel(),
					localInsertOp->GetHierarchy());
			}
		}
	}

	return result;
}


TransformedPair InsertRemoveTransform::Apply(const OperationConstPtr& localOp, const OperationConstPtr& remoteOp) const
{
	XTASSERT(localOp->GetType() == Operation::Insert && remoteOp->GetType() == Operation::Remove);

	// By default assume no conflict
	TransformedPair result(localOp, remoteOp);

	// If the operations are on the same array
	if (localOp->GetTargetGuid() == remoteOp->GetTargetGuid())
	{
		const InsertOperation* insertOp = static_cast<const InsertOperation*>(localOp.get());
		const RemoveOperation* removeOp = static_cast<const RemoveOperation*>(remoteOp.get());

		if (insertOp->GetIndex() > removeOp->GetIndex())
		{
			result.m_localOpTransformed = new InsertOperation(insertOp->GetTargetGuid(), insertOp->GetIndex() - 1, insertOp->GetValue(), insertOp->GetAuthorityLevel(), insertOp->GetHierarchy());
		}
		else if (insertOp->GetIndex() <= removeOp->GetIndex())
		{
			result.m_remoteOpTransformed = new RemoveOperation(removeOp->GetTargetGuid(), removeOp->GetIndex() + 1, removeOp->GetAuthorityLevel(), removeOp->GetHierarchy());
		}
	}

	return result;
}


TransformedPair RemoveRemoveTransform::Apply(const OperationConstPtr& localOp, const OperationConstPtr& remoteOp) const
{
	XTASSERT(localOp->GetType() == Operation::Remove && remoteOp->GetType() == Operation::Remove);

	// By default assume no conflict
	TransformedPair result(localOp, remoteOp);

	// If the operations are on the same array
	if (localOp->GetTargetGuid() == remoteOp->GetTargetGuid())
	{
		const RemoveOperation* localRemoveOp = static_cast<const RemoveOperation*>(localOp.get());
		const RemoveOperation* remoteRemoveOp = static_cast<const RemoveOperation*>(remoteOp.get());

		if (localRemoveOp->GetIndex() > remoteRemoveOp->GetIndex())
		{
			result.m_localOpTransformed = new RemoveOperation(localRemoveOp->GetTargetGuid(), localRemoveOp->GetIndex() - 1, localRemoveOp->GetAuthorityLevel(), localRemoveOp->GetHierarchy());
		}
		else if (localRemoveOp->GetIndex() < remoteRemoveOp->GetIndex())
		{
			result.m_remoteOpTransformed = new RemoveOperation(remoteRemoveOp->GetTargetGuid(), remoteRemoveOp->GetIndex() - 1, remoteRemoveOp->GetAuthorityLevel(), remoteRemoveOp->GetHierarchy());
		}
		else
		{
			result.m_localOpTransformed = NoopOperation::Instance();
			result.m_remoteOpTransformed = NoopOperation::Instance();
		}
	}

	return result;
}


NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
