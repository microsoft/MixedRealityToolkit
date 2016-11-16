//////////////////////////////////////////////////////////////////////////
// AudioPacket.h
//
// One packet of audio information
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AudioPacket.h"

XTOOLS_NAMESPACE_BEGIN

AudioPacket::AudioPacket(uint32 sampleCountForAllChannelsAndStreams)
: m_actualStreamCount(0)
, m_maxStreamCount(0)
, m_channelCount(0)
, m_sampleCount(0)
, m_sampleRate(0)
, m_audioSampleBufferAllocatedSize(sampleCountForAllChannelsAndStreams)
{
	m_audioSampleBuffer = new float[m_audioSampleBufferAllocatedSize];
}


void AudioPacket::Initialize(uint64 timeReceivedAsMicroscondsSinceEpoch, uint16 initialStreamCount, uint16 channelCount, uint32 sampleCount, uint32 sampleRate, bool isMuted, uint32 sequenceNumber)
{

	m_isMuted = isMuted;
	m_timeReceived = timeReceivedAsMicroscondsSinceEpoch;
	m_actualStreamCount = initialStreamCount;
	m_maxStreamCount = (uint16)(m_audioSampleBufferAllocatedSize / (sampleCount * (uint32)channelCount));
	m_channelCount = channelCount;
	m_sampleCount = sampleCount;
	m_sampleRate = sampleRate;
	m_sequenceNumber = sequenceNumber;

	XTASSERT(m_actualStreamCount <= m_maxStreamCount);

	uint32 samplesPerStream = m_channelCount * m_sampleCount;

	for (uint16 stream = 0; stream < m_maxStreamCount; stream++)
	{
		m_streamInfo[stream].hrtf.sourceID = 0;
		m_streamInfo[stream].isSilent = false;
		m_streamInfo[stream].isAverageAmplitudeCalculated = false;
		m_streamInfo[stream].averageAmplitude = 0.0f;
		m_streamInfo[stream].audioSamples = m_audioSampleBuffer.get() + stream * samplesPerStream;
	}

}


bool AudioPacket::AddStream(bool isSilent)
{
	if (m_actualStreamCount < m_maxStreamCount)
	{
		m_streamInfo[m_actualStreamCount].hrtf.sourceID = 0;
		m_streamInfo[m_actualStreamCount].isSilent = isSilent;
		m_streamInfo[m_actualStreamCount].isAverageAmplitudeCalculated = false;
		m_streamInfo[m_actualStreamCount].averageAmplitude = 0.0f;
		m_actualStreamCount++;
		return true;
	}
	else
	{
		return false;
	}
}


void AudioPacket::Reset(void)
{
	m_timeReceived = 0;
	m_channelCount = 0;
	m_sampleCount = 0;
	m_actualStreamCount = 0;
	m_maxStreamCount = 0; 

	//nothing else really needs clearing, so don't do the work.
}


float AudioPacket::GetAverageAmplitudeForStream(uint16 whichStream, bool forceRecalculate)
{
	XTASSERT(whichStream < m_actualStreamCount);

	if ( !m_isMuted && !m_streamInfo[whichStream].isSilent)
	{
		if (!m_streamInfo[whichStream].isAverageAmplitudeCalculated || forceRecalculate)
		{
			float avgAmplitude = 0.0;
			float const * samplesPtr = GetSamplePointerForStream(whichStream, 0);

			for (uint32 s = 0; s < m_sampleCount; s++) {
				avgAmplitude += (float)fabs((double)*samplesPtr++);
			}

			m_streamInfo[whichStream].averageAmplitude = avgAmplitude / m_sampleCount;
			m_streamInfo[whichStream].isAverageAmplitudeCalculated = true;
		}
		return m_streamInfo[whichStream].averageAmplitude;
	}
	else
	{
		return 0.0f;
	}

}

float const * AudioPacket::GetSamplePointerForStream(uint16 whichStream, uint16 whichChannel) const
{
	XTASSERT(whichStream < m_actualStreamCount);
	XTASSERT(whichChannel < m_channelCount);

	return m_streamInfo[whichStream].audioSamples + (whichChannel * m_sampleCount);
}

float * AudioPacket::GetSamplePointerForStream(uint16 whichStream, uint16 whichChannel)
{
	XTASSERT(whichStream < m_actualStreamCount);
	XTASSERT(whichChannel < m_channelCount);

	return m_streamInfo[whichStream].audioSamples + (whichChannel * m_sampleCount);
}

void AudioPacket::SetHrtfForStream(uint16 whichStream, HrtfInfo& inHrtf)
{
	XTASSERT(whichStream < m_actualStreamCount);

	m_streamInfo[whichStream].hrtf = inHrtf;
}


void AudioPacket::GetHrtfForStream(uint16 whichStream, HrtfInfo& hrtfOut) const
{
	XTASSERT(whichStream < m_actualStreamCount);

	hrtfOut = m_streamInfo[whichStream].hrtf;
}


bool AudioPacket::StreamHasHrtf(uint16 whichStream) const
{
	XTASSERT(whichStream < m_actualStreamCount);

	return m_streamInfo[whichStream].hrtf.sourceID != 0;
}

XTOOLS_NAMESPACE_END
