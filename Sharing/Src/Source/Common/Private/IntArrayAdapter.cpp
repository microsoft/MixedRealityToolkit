// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// IntArrayAdapter.cpp
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IntArrayAdapter.h"

XTOOLS_NAMESPACE_BEGIN

void IntArrayAdapter::SetValueChangedCallback(const Callback2<int32, int32>& callback)
{
	m_valueChangedCallback = callback;
}


void IntArrayAdapter::SetValueInsertedCallback(const Callback2<int32, int32>& callback)
{
	m_valueInsertedCallback = callback;
}


void IntArrayAdapter::SetValueRemovedCallback(const Callback2<int32, int32>& callback)
{
	m_valueRemovedCallback = callback;
}


void IntArrayAdapter::OnValueChanged(int32 index, int32 newValue)
{
	m_valueChangedCallback.Call(index, newValue);
}


void IntArrayAdapter::OnValueInserted(int32 index, int32 value)
{
	m_valueInsertedCallback.Call(index, value);
}


void IntArrayAdapter::OnValueRemoved(int32 index, int32 value)
{
	m_valueRemovedCallback.Call(index, value);
}

XTOOLS_NAMESPACE_END
