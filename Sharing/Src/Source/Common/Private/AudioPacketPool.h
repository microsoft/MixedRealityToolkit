//////////////////////////////////////////////////////////////////////////
// AudioPacketPool.h
//
// Manages a pool of ready-to-go AudioPackets.
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include <queue>
#include <Private/AudioPacket.h>

XTOOLS_NAMESPACE_BEGIN


class AudioPacketPool : public RefCounted
{
public:
	AudioPacketPool(uint32 maxSampleCount);
	void Reserve(uint32 sampleCountForAllStreamsAndChannels, uint32 numToReserve);
	AudioPacketPtr AcquireAudioPacket(uint32 sampleCountForAllStreamsAndChannels);
	void ReturnAudioPacket(const AudioPacketPtr& audioPacket);
	uint32 GetDefaultMaxSampleCount(void) const;

private:
	// To keep it simple all packets right now can handle max possible allocation.
	uint32                      m_defaultQueueSizeSampleCount;
	std::queue<AudioPacketPtr>	m_audioPacketDefaultSizeQueue;
};

DECLARE_PTR(AudioPacketPool)

XTOOLS_NAMESPACE_END