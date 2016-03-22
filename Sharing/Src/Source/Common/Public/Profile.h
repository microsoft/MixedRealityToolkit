//////////////////////////////////////////////////////////////////////////
// Profile.h
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class Profile
{
public:
	static void BeginRange(const std::string& name);
	static void EndRange();
};

XTOOLS_NAMESPACE_END
