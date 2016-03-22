//////////////////////////////////////////////////////////////////////////
// NotifyList.h
//
// Provides a convenient wrapper to call a callback function from a list of objects
// in a way that will not break if the called function happens to remove the element from the list
//
// Copyright (C) 2015 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

template<typename C>
void NotifyList(const std::vector<C*>& list, void (C::* func) (void))
{
	for (int32 i = (int32)list.size() - 1; i >= 0; --i)
	{
		(list[i]->*func)();
	}
}


template<typename C>
void NotifyList(const std::vector<ref_ptr<C> >& list, void (C::* func) (void))
{
	for (int32 i = (int32)list.size() - 1; i >= 0; --i)
	{
		(list[i]->*func)();
	}
}


template<typename C, typename P1, typename T1>
void NotifyList(const std::vector<C*>& list, void (C::* func) (P1), T1& param)
{
	for (int32 i = (int32)list.size() - 1; i >= 0; --i)
	{
		(list[i]->*func)(param);
	}
}


template<typename C, typename P1, typename T1>
void NotifyList(const std::vector<ref_ptr<C> >& list, void (C::* func) (P1), T1& param)
{
	for (int32 i = (int32)list.size() - 1; i >= 0; --i)
	{
		(list[i]->*func)(param);
	}
}


template<typename C, typename P1, typename P2, typename T1, typename T2>
void NotifyList(const std::vector<C*>& list, void (C::* func) (P1, P2), T1 param1, T2& param2)
{
	for (int32 i = (int32)list.size() - 1; i >= 0; --i)
	{
		(list[i]->*func)(param1, param2);
	}
}


template<typename C, typename P1, typename P2, typename T1, typename T2>
void NotifyList(const std::vector<ref_ptr<C> >& list, void (C::* func) (P1, P2), T1& param1, T2& param2)
{
	for (int32 i = (int32)list.size() - 1; i >= 0; --i)
	{
		(list[i]->*func)(param1, param2);
	}
}

XTOOLS_NAMESPACE_END