// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// AckOperation.cpp
// Op that acknowledges the previous ops but does not increment the received op count
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AckOperation.h"

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

AckOperation::AckOperation(Sync::AuthorityLevel authLevel)
	: Operation(Operation::Ack, authLevel)
{

}


XGuid AckOperation::GetTargetGuid() const
{
	return kInvalidXGuid;
}


void AckOperation::Apply(const SyncContextPtr&)
{
	// Intentionally Blank
}


void AckOperation::Serialize(const NetworkOutMessagePtr&) const
{
	// Intentionally Blank
}


void AckOperation::Deserialize(NetworkInMessage&)
{
	// Intentionally Blank
}


void AckOperation::Notify(const SyncContextPtr&) const
{
	// Intentionally Blank
}

std::string AckOperation::GetOpDescription() const
{
	return GetTypeName();
}


NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
