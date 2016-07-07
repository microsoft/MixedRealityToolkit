// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// NoopOperation.cpp
// Represents a change to the shared state that has no effect
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "NoopOperation.h"

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

ref_ptr<NoopOperation> NoopOperation::m_sInstance = new NoopOperation(AuthorityLevel::Unknown);

NoopOperation::NoopOperation(Sync::AuthorityLevel authLevel)
	: Operation(Operation::Noop, authLevel)
{

}


XGuid NoopOperation::GetTargetGuid() const
{
	return kInvalidXGuid;
}


void NoopOperation::Apply(const SyncContextPtr& )
{
	// Intentionally Blank
}


void NoopOperation::Serialize(const NetworkOutMessagePtr& ) const
{
	// Intentionally Blank
}


void NoopOperation::Deserialize(NetworkInMessage& )
{
	// Intentionally Blank
}


void NoopOperation::Notify(const SyncContextPtr& ) const
{
	// Intentionally Blank
}

std::string NoopOperation::GetOpDescription() const
{
	return GetTypeName();
}

// static
NoopOperationPtr NoopOperation::Instance()
{
	return m_sInstance;
}

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
