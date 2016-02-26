//////////////////////////////////////////////////////////////////////////
// ArrayElement.h
//
// Copyright (C) 2016 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN
NAMESPACE_BEGIN(Sync)

// Abstract interface for internal functionality provided by array sync elements.
// The functions manipulate the array but do not trigger the creation of new outgoing operations
// to remote clients
class ArrayElement
{
	XTOOLS_REFLECTION_DECLARE(ArrayElement)

public:
	virtual int32 GetCount() const = 0;

	virtual XValue GetXValue(int32 index) const = 0;

	virtual void SetXValue(int32 index, const XValue& newValue) = 0;

	virtual void InsertXValue(int32 index, const XValue& newValue) = 0;

	virtual void RemoveXValue(int32 index) = 0;

	virtual void NotifyUpdated(int32 index, const XValue& newValue) = 0;

	virtual void NotifyInserted(int32 index, const XValue& newValue) = 0;

	virtual void NotifyRemoved(int32 index, const XValue& value) = 0;
};

NAMESPACE_END(Sync)
XTOOLS_NAMESPACE_END
