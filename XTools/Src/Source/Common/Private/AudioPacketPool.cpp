//////////////////////////////////////////////////////////////////////////
// AudioPacketPool.cpp
//
// Manages a pool of AudioPacket objects, helping to avoid frequent allocations
// when processing audio packets.
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AudioPacketPool.h"
#include <Private/AudioPacket.h>


XTOOLS_NAMESPACE_BEGIN


AudioPacketPool::AudioPacketPool(uint32 defaultQueueSizeSampleCount)
: m_defaultQueueSizeSampleCount(defaultQueueSizeSampleCount)
{
}


uint32 AudioPacketPool::GetDefaultMaxSampleCount(void) const
{
	return m_defaultQueueSizeSampleCount;
}


void AudioPacketPool::Reserve(uint32 sampleCountForAllStreamsAndChannels, uint32 numToReserve)
{
	// For future expansion to allow a variety of sizes for RAM efficiency
	// For now, fail if we try anything else
	XTASSERT(sampleCountForAllStreamsAndChannels <= m_defaultQueueSizeSampleCount);

	for (size_t i = 0; i < numToReserve; ++i)
	{
		m_audioPacketDefaultSizeQueue.push(new AudioPacket(sampleCountForAllStreamsAndChannels));
	}
}


AudioPacketPtr AudioPacketPool::AcquireAudioPacket(uint32 sampleCountForAllStreamsAndChannels)
{
	AudioPacketPtr newAudioPacket;

	// For future expansion to allow a variety of sizes for RAM efficiency
	// For now, fail if we try anything else
	XTASSERT(sampleCountForAllStreamsAndChannels <= m_defaultQueueSizeSampleCount);

	if (!m_audioPacketDefaultSizeQueue.empty())
	{
		newAudioPacket = m_audioPacketDefaultSizeQueue.front();
		m_audioPacketDefaultSizeQueue.pop();
	}
	else
	{
		LogWarning("Increasing size of audio packet pool for 2 channels and 4096 samples");
		newAudioPacket = new AudioPacket(m_defaultQueueSizeSampleCount);
	}

	return newAudioPacket;
}


void AudioPacketPool::ReturnAudioPacket(const AudioPacketPtr& audioPacket)
{
	audioPacket->Reset();

	m_audioPacketDefaultSizeQueue.push(audioPacket);
}

XTOOLS_NAMESPACE_END