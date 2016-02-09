//////////////////////////////////////////////////////////////////////////
// SideCar.h
//
// Abstract base class for all XTools 'SideCars', ie: plug-ins to XTools.
// Users should implement a class that inherits from this class in their
// sidecar dll
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class SideCar : public RefCounted
{
public:
	virtual ~SideCar() {}

	virtual void Initialize(const SideCarContextPtr& context) = 0;

	virtual void Update() = 0;

};

DECLARE_PTR(SideCar)

XTOOLS_NAMESPACE_END