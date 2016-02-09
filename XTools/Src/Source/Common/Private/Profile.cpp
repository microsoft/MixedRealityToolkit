//////////////////////////////////////////////////////////////////////////
// Profile.cpp
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
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
