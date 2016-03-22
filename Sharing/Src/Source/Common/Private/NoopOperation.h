//////////////////////////////////////////////////////////////////////////
// NoopOperation.h
//
// Represents a change to the shared state that has no effect
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

class NoopOperation : public Operation
{
public:
	explicit NoopOperation(Sync::AuthorityLevel authLevel);

	virtual XGuid GetTargetGuid() const XTOVERRIDE;

	virtual void Apply(const SyncContextPtr& context) XTOVERRIDE;

	virtual void Serialize(const NetworkOutMessagePtr& msg) const XTOVERRIDE;

	virtual void Deserialize(NetworkInMessage& msg) XTOVERRIDE;

	virtual void Notify(const SyncContextPtr& context) const XTOVERRIDE;

	virtual std::string GetOpDescription() const XTOVERRIDE;

	static ref_ptr<NoopOperation> Instance();

private:
	static ref_ptr<NoopOperation> m_sInstance;
};

DECLARE_PTR(NoopOperation)

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END