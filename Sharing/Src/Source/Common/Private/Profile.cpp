// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// Profile.cpp
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Public/Profile.h>
#include "ProfileManagerImpl.h"

XTOOLS_NAMESPACE_BEGIN

// static 
void Profile::BeginRange(const std::string& name)
{
	ProfileManagerImplPtr mgr = ProfileManagerImpl::GetInstance();
	if (mgr != nullptr)
	{
		mgr->BeginRange(name);
	}
}

// static 
void Profile::EndRange()
{
	ProfileManagerImplPtr mgr = ProfileManagerImpl::GetInstance();
	if (mgr != nullptr)
	{
		mgr->EndRange();
	}
}

XTOOLS_NAMESPACE_END
