// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// AudioManager.h
// Wraps audio functionality available to SWIG.
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
