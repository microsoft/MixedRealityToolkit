//////////////////////////////////////////////////////////////////////////
// StringUtils.cpp
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Public/Utils/StringUtils.h>

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(StringUtils)

std::vector<std::string> Tokenize(const char *str, char c)
{
	std::vector<std::string> result;

	do
	{
		const char *begin = str;

		while (*str != c && *str)
			str++;

		std::string tokenString(begin, str);
		if (!tokenString.empty())
		{
			result.push_back(tokenString);
		}
	} while (0 != *str++);

	return result;
}

NAMESPACE_END(StringUtils)
XTOOLS_NAMESPACE_END
