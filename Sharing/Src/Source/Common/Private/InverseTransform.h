//////////////////////////////////////////////////////////////////////////
// InverseTransform.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

// Since a transform(A,B) should be the same logic as transform(B,A), this class just wraps the logic for
// a transform and switches the order of the inputs.  
template<typename T>
class InverseTransform : public OpTransform
{
public:
	InverseTransform() : m_wrappedOp(new T()) {}

	virtual TransformedPair Apply(const OperationConstPtr& localOp, const OperationConstPtr& remoteOp) const XTOVERRIDE
	{
		// Call the inner transform with the parameters flipped
		TransformedPair result = m_wrappedOp->Apply(remoteOp, localOp);

		// Flip the results and return
		return TransformedPair(result.m_remoteOpTransformed, result.m_localOpTransformed);
	}

private:
	OpTransformPtr m_wrappedOp;
};

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
