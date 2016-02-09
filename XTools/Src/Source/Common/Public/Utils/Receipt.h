//////////////////////////////////////////////////////////////////////////
// Receipt.h
//
// Base class for any kind of object that you want to take action when
// it loses all references
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#if !defined(SWIG)
# include <vector>
#endif

XTOOLS_NAMESPACE_BEGIN

class Receipt : public AtomicRefCounted
{
public:
	virtual ~Receipt() {}
	virtual void Clear() {}
};

DECLARE_PTR(Receipt)

#if !defined(SWIG)
typedef std::vector<ReceiptPtr> ReceiptVector;
#endif

XTOOLS_NAMESPACE_END