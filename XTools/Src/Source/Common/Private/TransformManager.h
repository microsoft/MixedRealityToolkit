//////////////////////////////////////////////////////////////////////////
// TransformManager.h
//
// Starting point for transforming incoming operations against local operations
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

class TransformManager
{
public:
	TransformManager();

	// Given two operations c and s, return another pair of operations such that if the client
	// applies c followed by s’, and the server applies s followed
	// by c’, then the client and server will wind up in the same final state.
	TransformedPair Transform(const OperationConstPtr& localOp, const OperationConstPtr& remoteOp) const;

private:
	typedef std::pair<Operation::Type, Operation::Type> OpPair;

	std::map<OpPair, OpTransformPtr>	m_transformMatrix;
};

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END