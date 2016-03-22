//////////////////////////////////////////////////////////////////////////
// AudioManager.h
//
// Wraps audio functionality available to SWIG.
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN

class AudioManager : public AtomicRefCounted
{
public:
	virtual void SetMicrophoneEnabled(bool bEnabled) = 0;
};

DECLARE_PTR(AudioManager)

XTOOLS_NAMESPACE_END