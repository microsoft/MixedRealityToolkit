//////////////////////////////////////////////////////////////////////////
// RegistrationReceipt.cpp
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "RegistrationReceipt.h"

XTOOLS_NAMESPACE_BEGIN

RegistrationKey GetNewRegistrationKey()
{
	static std::atomic<int32> sCounter(0);

	return sCounter++;
}

XTOOLS_NAMESPACE_END
