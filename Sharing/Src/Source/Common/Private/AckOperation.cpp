//////////////////////////////////////////////////////////////////////////
// AckOperation.cpp
//
// Op that acknowledges the previous ops but does not increment the received op count
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
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
