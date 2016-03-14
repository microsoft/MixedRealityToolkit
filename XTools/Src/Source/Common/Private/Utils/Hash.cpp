//////////////////////////////////////////////////////////////////////////
// Hash.cpp
//
// 
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Public/Utils/Hash.h>

XTOOLS_NAMESPACE_BEGIN

// Analysis of hash function performance:
// http://www.strchr.com/hash_functions


// Larson hash
uint64 HashString(const char* s)
{
	uint64 hash = 31;
	while (*s)
	{
		hash = hash * 101 + *s++;
	}
	return hash;
}


uint64 HashBuffer(const char* buff, uint64 count)
{
	uint64 hash = 31;
	while (count--)
	{
		hash = hash * 101 + *buff++;
	}
	return hash;
}


//static const uint64 kHashPrimeA = 54059;
//static const uint64 kHashPrimeB = 76963;
//uint64 HashString(const char* str)
//{
//	uint64 h = 31;
//	while (*str) 
//	{
//		h = (h * kHashPrimeA) ^ (str[0] * kHashPrimeB);
//		++str;
//	}
//
//	return h;
//}

XTOOLS_NAMESPACE_END
