// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// OperationFactory.h
// Quick scalable way to create Operations from an enum
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

//////////////////////////////////////////////////////////////////////////
class OpMaker : public RefCounted
{
public:
	virtual OperationPtr Create(AuthorityLevel authLevel) const = 0;
};
DECLARE_PTR(OpMaker)


//////////////////////////////////////////////////////////////////////////
class OperationFactory
{
public:
	void RegisterMaker(Operation::Type type, const OpMakerPtr& maker);

	OperationPtr Make(Operation::Type type, AuthorityLevel authLevel) const;

private:
	std::map<Operation::Type, OpMakerPtr> m_makers;
};


//////////////////////////////////////////////////////////////////////////
template<typename T>
class OpMakerT : public OpMaker
{
public:
	virtual OperationPtr Create(AuthorityLevel authLevel) const XTOVERRIDE
	{
		return new T(authLevel);
	}
};

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
