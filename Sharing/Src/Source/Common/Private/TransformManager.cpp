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

#include "ArrayTransforms.h"
#include "ReplaceTransforms.h"

#include "InverseTransform.h"

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

TransformManager::TransformManager()
{
	// Add all the OpTransforms to the transform matrix.
	// Any empty pairings will be assumed to require no transformation
	m_transformMatrix[OpPair(Operation::Create, Operation::Create)] = new CreateCreateTransform();
	m_transformMatrix[OpPair(Operation::Create, Operation::Modify)] = new CreateModifyTransform();
	m_transformMatrix[OpPair(Operation::Create, Operation::Delete)] = new CreateDeleteTransform();

	m_transformMatrix[OpPair(Operation::Modify, Operation::Modify)] = new ModifyModifyTransform();
	m_transformMatrix[OpPair(Operation::Modify, Operation::Delete)] = new ModifyDeleteTransform();

	m_transformMatrix[OpPair(Operation::Delete, Operation::Create)] = new DeleteCreateTransform();
	m_transformMatrix[OpPair(Operation::Delete, Operation::Modify)] = new DeleteModifyTransform();
	m_transformMatrix[OpPair(Operation::Delete, Operation::Delete)] = new DeleteDeleteTransform();

	m_transformMatrix[OpPair(Operation::Delete, Operation::Update)] = new DeleteModifyTransform();
	m_transformMatrix[OpPair(Operation::Delete, Operation::Insert)] = new DeleteModifyTransform();
	m_transformMatrix[OpPair(Operation::Delete, Operation::Remove)] = new DeleteModifyTransform();
	m_transformMatrix[OpPair(Operation::Update, Operation::Delete)] = new InverseTransform<DeleteModifyTransform>();
	m_transformMatrix[OpPair(Operation::Insert, Operation::Delete)] = new InverseTransform<DeleteModifyTransform>();
	m_transformMatrix[OpPair(Operation::Remove, Operation::Delete)] = new InverseTransform<DeleteModifyTransform>();

	m_transformMatrix[OpPair(Operation::Replace, Operation::Create)] = new ReplaceCreateTransform();
	m_transformMatrix[OpPair(Operation::Replace, Operation::Modify)] = new ReplaceModifyTransform();
	m_transformMatrix[OpPair(Operation::Replace, Operation::Delete)] = new ReplaceDeleteTransform();
	m_transformMatrix[OpPair(Operation::Create, Operation::Replace)] = new InverseTransform<ReplaceCreateTransform>();
	m_transformMatrix[OpPair(Operation::Modify, Operation::Replace)] = new InverseTransform<ReplaceModifyTransform>();
	m_transformMatrix[OpPair(Operation::Delete, Operation::Replace)] = new InverseTransform<ReplaceDeleteTransform>();
	m_transformMatrix[OpPair(Operation::Replace, Operation::Replace)] = new ReplaceReplaceTransform();
	
	m_transformMatrix[OpPair(Operation::Update, Operation::Update)] = new UpdateUpdateTransform();
	m_transformMatrix[OpPair(Operation::Update, Operation::Insert)] = new UpdateInsertTransform();
	m_transformMatrix[OpPair(Operation::Update, Operation::Remove)] = new UpdateRemoveTransform();

	m_transformMatrix[OpPair(Operation::Insert, Operation::Insert)] = new InsertInsertTransform();
	m_transformMatrix[OpPair(Operation::Insert, Operation::Update)] = new InverseTransform<UpdateInsertTransform>();
	m_transformMatrix[OpPair(Operation::Insert, Operation::Remove)] = new InsertRemoveTransform();

	m_transformMatrix[OpPair(Operation::Remove, Operation::Remove)] = new RemoveRemoveTransform();
	m_transformMatrix[OpPair(Operation::Remove, Operation::Update)] = new InverseTransform<UpdateRemoveTransform>();
	m_transformMatrix[OpPair(Operation::Remove, Operation::Insert)] = new InverseTransform<InsertRemoveTransform>();
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