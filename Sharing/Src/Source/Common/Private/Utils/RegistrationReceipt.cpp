// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// RegistrationReceipt.cpp
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
