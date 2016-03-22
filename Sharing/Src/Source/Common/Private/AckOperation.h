//////////////////////////////////////////////////////////////////////////
// AckOperation.h
//
// Op that acknowledges the previous ops but does not increment the received op count
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

class AckOperation : public Operation
{
public:
	explicit AckOperation(Sync::AuthorityLevel authLevel);

	virtual XGuid GetTargetGuid() const XTOVERRIDE;

	virtual void Apply(const SyncContextPtr& context) XTOVERRIDE;

	virtual void Serialize(const NetworkOutMessagePtr& msg) const XTOVERRIDE;

	virtual void Deserialize(NetworkInMessage& msg) XTOVERRIDE;

	virtual void Notify(const SyncContextPtr& context) const XTOVERRIDE;

	virtual std::string GetOpDescription() const XTOVERRIDE;
};

DECLARE_PTR(AckOperation)

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END