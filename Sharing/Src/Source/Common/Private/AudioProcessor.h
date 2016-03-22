//////////////////////////////////////////////////////////////////////////
// AudioProcessor.h
//
// Interface for processing an audio packet.
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <Private/AudioPacket.h>

XTOOLS_NAMESPACE_BEGIN

class AudioConnection;
typedef XTools::ref_ptr<AudioConnection> AudioConnectionPtr;

class AudioProcessor : public RefCounted
{
public:
	virtual void ProcessAudioPacket(AudioConnectionPtr& audioConnection, AudioPacketPtr& audioPacket) = 0;
};

DECLARE_PTR(AudioProcessor)

XTOOLS_NAMESPACE_END