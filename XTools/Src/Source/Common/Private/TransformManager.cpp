//////////////////////////////////////////////////////////////////////////
// TransformManager.cpp
//
// Starting point for transforming incoming operations against local operations
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TransformManager.h"

// OpTransforms:
#include "CreateCreateTransform.h"
#include "CreateModifyTransform.h"
#include "CreateDeleteTransform.h"
// No ModifyCreateTransform
#include "ModifyModifyTransform.h"
#include "ModifyDeleteTransform.h"
#include "DeleteCreateTransform.h"
#include "DeleteModifyTransform.h"
#include "DeleteDeleteTransform.h"

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

TransformManager::TransformManager()
{
	// Add all the OpTransforms to the transform matrix.
	// Any empty pairings will be assumed to be no-op transformations
	m_transformMatrix[OpPair(Operation::Create, Operation::Create)] = new CreateCreateTransform();
	m_transformMatrix[OpPair(Operation::Create, Operation::Modify)] = new CreateModifyTransform();
	m_transformMatrix[OpPair(Operation::Create, Operation::Delete)] = new CreateDeleteTransform();

	m_transformMatrix[OpPair(Operation::Modify, Operation::Modify)] = new ModifyModifyTransform();
	m_transformMatrix[OpPair(Operation::Modify, Operation::Delete)] = new ModifyDeleteTransform();

	m_transformMatrix[OpPair(Operation::Delete, Operation::Create)] = new DeleteCreateTransform();
	m_transformMatrix[OpPair(Operation::Delete, Operation::Modify)] = new DeleteModifyTransform();
	m_transformMatrix[OpPair(Operation::Delete, Operation::Delete)] = new DeleteDeleteTransform();
}


TransformedPair TransformManager::Transform(const OperationConstPtr& applyOp, const OperationConstPtr& incomingOp) const
{
	// Check to see if there is a transformation registered for this pairing of operations
	auto mapIter = m_transformMatrix.find( OpPair(applyOp->GetType(), incomingOp->GetType()) );
	if (mapIter != m_transformMatrix.end())
	{
		return mapIter->second->Apply(applyOp, incomingOp);
	}

	// No transformation necessary; just return the original incoming operation
	return TransformedPair(applyOp, incomingOp);
}

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END