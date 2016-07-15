// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// IntArrayAdapter.h
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class IntArrayAdapter : public AtomicRefCounted, public IntArrayListener
{
public:
	IntArrayAdapter() {}

	void SetValueChangedCallback(const Callback2<int32, int32>& callback);
	void SetValueInsertedCallback(const Callback2<int32, int32>& callback);
	void SetValueRemovedCallback(const Callback2<int32, int32>& callback);

private:
	virtual void OnValueChanged(int32 index, int32 newValue) XTOVERRIDE;
	virtual void OnValueInserted(int32 index, int32 value) XTOVERRIDE;
	virtual void OnValueRemoved(int32 index, int32 value) XTOVERRIDE;

	Callback2<int32, int32> m_valueChangedCallback;
	Callback2<int32, int32> m_valueInsertedCallback;
	Callback2<int32, int32> m_valueRemovedCallback;
};

DECLARE_PTR(IntArrayAdapter)

XTOOLS_NAMESPACE_END
