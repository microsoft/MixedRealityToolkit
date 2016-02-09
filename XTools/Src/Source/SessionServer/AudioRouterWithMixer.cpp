//////////////////////////////////////////////////////////////////////////
// AudioRouterWithMixer.cpp
//
// Manages aligning and routing audio to the right places including
// mixing some audio streams while keeping prominent streams separate
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "stdafx.h"
#include "AudioRouterWithMixer.h"

#include <chrono>
#include <string>

//#define AUDIO_DEBUG
#define AUDIO_ONE_SECOND_STATISTICS

XTOOLS_NAMESPACE_BEGIN

const float AudioRouterWithMixer::kDefaultMaxAmplitudeForBalancingPackets = 0.05f;


AudioRouterWithMixer::AudioRouterWithMixer(AudioPacketPoolPtr& audioPacketPool, uint32 masterSampleRate, uint16 masterSamplesPerPacket, uint16 prominentHrtfStreamCount)
: m_audioPacketPool(audioPacketPool)
, m_masterSampleRate(masterSampleRate)
, m_masterSamplesPerPacket(masterSamplesPerPacket)
, m_prominentHrtfStreamCount(prominentHrtfStreamCount)
, m_maxTimeToWaitForLatePacketInMilliseconds(kDefaultMaxTimeToWaitForLatePacketInMilliseconds)
, m_totalLatePacketCount(0)
, m_maxAmplitudeForBalancingPackets(kDefaultMaxAmplitudeForBalancingPackets)
, m_removeSelfFromAudio(true)              // Use this for diagnostics to hear yourself in the mixed audio stream
, m_streamToWavFile(false)                 // Use this for diagnostics to record all streams to wav files in current directory. WAV file completed when you leave session.
{

}


// Each received audio packet from all connections arrive here.  All audio processing is triggered by
// these received packets.  
void AudioRouterWithMixer::ProcessAudioPacket(AudioConnectionPtr& audioConnection, AudioPacketPtr& audioPacket)
{
	uint64 currentTimeInMicroseconds = GetCurrentMicrosecondsSinceEpoch();

	AudioConnectionGUID guid = audioConnection->GetConnectionGUID();
	ConnectionQueuePtr connectionQueue = m_connectionQueueMap[guid];

	XTASSERT(m_masterSamplesPerPacket == audioPacket->GetSampleCount());
	XTASSERT(m_masterSampleRate == audioPacket->GetSampleRate());

	connectionQueue->Push(currentTimeInMicroseconds, audioPacket);
	ProcessAllStateChanges(currentTimeInMicroseconds);
}


void AudioRouterWithMixer::AddAudioConnection(AudioConnectionPtr& audioConnection)
{
	ConnectionQueuePtr connectionQueue = new ConnectionQueue(audioConnection, m_audioPacketPool, m_masterSampleRate, m_masterSamplesPerPacket, m_streamToWavFile);

	AudioConnectionGUID guid = audioConnection->GetConnectionGUID();

	m_connectionQueueMap[guid] = connectionQueue;
}


void AudioRouterWithMixer::RemoveAudioConnection(AudioConnectionPtr& audioConnection)
{
	AudioConnectionGUID guid = audioConnection->GetConnectionGUID();

	XTASSERT(m_connectionQueueMap.find(guid) != m_connectionQueueMap.end());
	m_connectionQueueMap.erase(guid);
}


void AudioRouterWithMixer::ProcessAllStateChanges(uint64 currentTimeInMicroseconds)
{
	if (m_connectionQueueMap.size() > 0)
	{
		// TODO(michhoff) Turning these vectors into instance variables to reduce memory allocations
		// appears to cause the ref_ptrs to be remove references more than they should be when clear() is
		// called and this causes crashes.  Until this can be debugged, we actually use local variables
		// instead.

		ConnectionQueueVector m_readyConnections;
		ConnectionQueueVector m_lateConnections;
		ConnectionQueueVector m_mixedConnections;
		ConnectionQueueVector m_prominentConnections;

		m_readyConnections.clear();			// done this way to avoid all the allocations of loading up the vectors here on the stack.
		m_lateConnections.clear();

		m_currentProcessingTime = currentTimeInMicroseconds;

		XTASSERT(m_readyConnections.size() == 0);
		while (AreConnectionsReadyForProcessing(m_readyConnections, m_lateConnections))
		{
			uint16 outStreamCount = CalculateOutputStreamCount();
			PrepareInputAndOutputAudioPackets(m_readyConnections, outStreamCount);
			PrepareInputAndOutputAudioPackets(m_lateConnections, outStreamCount);

			IdentifyProminentHrtfStreams(m_readyConnections, m_prominentConnections, m_mixedConnections);
			SetMostProminentHrtfInMixedStream(m_readyConnections);

			MixAllAudioStreams(m_mixedConnections);
			CreateMinusOneOutputStreams(m_readyConnections, 0);
			CreateHrtfOutputStreams(m_readyConnections, m_prominentConnections);

			PrepareLatePackets(m_lateConnections);

			m_averageAmplitude = CalculateTotalAverageAmplitude(m_prominentConnections);

			SendOutputAudioPackets(m_readyConnections);
			SendOutputAudioPackets(m_lateConnections);

			ReturnAudioPackets(m_readyConnections);
			ReturnAudioPackets(m_lateConnections);

			BalanceQueues(m_readyConnections);

#if defined(AUDIO_ONE_SECOND_STATISTICS)
			LogStatistics();
#endif

			m_readyConnections.clear();
			m_lateConnections.clear();
			m_mixedConnections.clear();
			m_prominentConnections.clear();
		}
	}
}

uint16 AudioRouterWithMixer::CalculateOutputStreamCount(void)
{
	ConnectionQueueMap::iterator cqIter;
	uint16 hrtfStreamCandidateCount = 0;

	if (m_prominentHrtfStreamCount)
	{
		// See if we have at least the desired number of hrtf streams available to us.

		for (cqIter = m_connectionQueueMap.begin();
			cqIter != m_connectionQueueMap.end();
			cqIter++)
		{
			ConnectionQueuePtr connQueue = cqIter->second;

			if (connQueue->m_isSendingHrtf && connQueue->m_state == ConnectionQueue::State::Normal)
			{
				if (++hrtfStreamCandidateCount == m_prominentHrtfStreamCount)
				{
					break;
				}
			}
		}
	}

	return hrtfStreamCandidateCount + 1;
}


uint64 AudioRouterWithMixer::GetCurrentMicrosecondsSinceEpoch(void)
{
	// Note: this provides about 1 ms accuracy even though asking for microseconds.
	return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count();
}


bool AudioRouterWithMixer::AreConnectionsReadyForProcessing(ConnectionQueueVector& readyConnectionsOut, ConnectionQueueVector& lateConnectionsOut)
{
	ConnectionQueueMap::iterator cqIter;
	uint64 oldestTimeReceived = 0;
	uint32 totalPacketCount = 0;

	m_waitCandidatePacketCountThisPeriod = 0;
	m_latePacketCountThisPeriod = 0;
	m_maxPacketsWaiting = 0;
	m_2ndToMaxPacketsWaiting = 0;
	m_minPacketsWaiting = 0x7fffffff;

	/* First let's see if we are in a late situation */
	for (cqIter = m_connectionQueueMap.begin();
		cqIter != m_connectionQueueMap.end();
		cqIter++)
	{
		ConnectionQueuePtr connQueue = cqIter->second;
		if (connQueue->m_audioConnection->IsConnected())
		{

			uint32 numPackets = connQueue->QueuePacketCount();

			totalPacketCount += numPackets;

			// used for balancing queues to deal with late packets
			if (m_maxPacketsWaiting < numPackets)
			{
				m_2ndToMaxPacketsWaiting = m_maxPacketsWaiting;
				m_maxPacketsWaiting = numPackets;
			}
			else if (m_2ndToMaxPacketsWaiting < numPackets)
			{
				m_2ndToMaxPacketsWaiting = numPackets;
			}

			if (m_minPacketsWaiting > numPackets)
			{
				m_minPacketsWaiting = numPackets;
			}

			if (connQueue->IsWaitCandidate())
			{
				m_waitCandidatePacketCountThisPeriod++;
			}
			else if (!connQueue->IsEmpty())
			{
				AudioPacketPtr audioPacket = connQueue->Peek();
				uint64 receivedTime = audioPacket->GetTimeReceived();
				if (!oldestTimeReceived || oldestTimeReceived > receivedTime)
				{
					oldestTimeReceived = receivedTime;
				}
			}
		}
	}

	if (totalPacketCount == 0) {
		return false;
	}

	if (m_waitCandidatePacketCountThisPeriod && oldestTimeReceived > m_currentProcessingTime - (m_maxTimeToWaitForLatePacketInMilliseconds * 1000)) {
		return false;	// we can wait longer for the late packets
	}

	for (cqIter = m_connectionQueueMap.begin();
		cqIter != m_connectionQueueMap.end();
		cqIter++)
	{
		ConnectionQueuePtr connQueue = cqIter->second;
		if (connQueue->m_audioConnection->IsConnected())
		{
			if (connQueue->IsEmpty())
			{
				if (connQueue->m_state == ConnectionQueue::State::Normal || connQueue->m_state == ConnectionQueue::State::HopelesslyLate)
				{
					connQueue->MarkLateAndIgnored();
					lateConnectionsOut.push_back(connQueue);
					m_latePacketCountThisPeriod++;
				}
			}
			else
			{
				if (!DelayPacket(connQueue, oldestTimeReceived))
				{
					readyConnectionsOut.push_back(connQueue);
				}
			}
		}
	}

	m_totalLatePacketCount += m_latePacketCountThisPeriod;

	return readyConnectionsOut.size() > 0;
}


// Once we receive the very 1st packet on a connection, we should not be too hasty in using
// it. Unless it is the oldest received packet, let's wait one round so that it effectively
// has a 1 packet buffer and is less likely to cause a stall later on.
bool AudioRouterWithMixer::DelayPacket(ConnectionQueuePtr& connQueue, uint64 oldestTimeReceived)
{
	if (connQueue->m_state == ConnectionQueue::State::FirstPacket)
	{
		AudioPacketPtr audioPacket = connQueue->Peek();
		if (audioPacket->GetTimeReceived() > oldestTimeReceived)
			return true;
	}
	return false;
}


void AudioRouterWithMixer::PrepareInputAndOutputAudioPackets(ConnectionQueueVector& connections, uint16 streamCount)
{
	ConnectionQueueVector::iterator cqIter;

	for (cqIter = connections.begin();
		cqIter != connections.end();
		cqIter++)
	{
		(*cqIter)->PrepareInputAndOutputAudioPackets(streamCount);
	}
}


void AudioRouterWithMixer::IdentifyProminentHrtfStreams(ConnectionQueueVector& readyConnectionsIn, ConnectionQueueVector& prominentConnectionsOut, ConnectionQueueVector& mixedConnectionsOut)
{
	m_mixedHrtfInfo.sourceID = 0;
	m_mixedHrtfInfo.position = { 0, 0, 0 };
	m_mixedHrtfInfo.orientation = { 0, 0, 0 };

	if (m_prominentHrtfStreamCount && m_connectionQueueMap.size() <= m_prominentHrtfStreamCount + 1)
	{
		ConnectionQueueVector::iterator cqIter;

		for (cqIter = readyConnectionsIn.begin();
			cqIter != readyConnectionsIn.end();
			cqIter++)
		{
			prominentConnectionsOut.push_back(*cqIter);
			(*cqIter)->m_isProminentHrtf = true;
		}

		mixedConnectionsOut.clear();
	}
	else
	{
		ProcessSpeakingDetection(readyConnectionsIn);


		// Now move all the rest of the ready connections to the mixedConnections vector for mixing.
		mixedConnectionsOut.clear();
		ConnectionQueueVector::iterator cqIter;

		for (cqIter = readyConnectionsIn.begin();
			cqIter != readyConnectionsIn.end();
			cqIter++)
		{
			if (!(*cqIter)->m_isProminentHrtf)
			{
				mixedConnectionsOut.push_back(*cqIter);
			}
		}
	}
}

uint32 AudioRouterWithMixer::ProcessSpeakingDetection(ConnectionQueueVector& connections)
{
	ConnectionQueueVector::iterator cqIter;
	uint32 speakingCount = 0;

	for (cqIter = connections.begin();
		cqIter != connections.end();
		cqIter++)
	{
		if ((*cqIter)->ProcessSpeakingDetection())
		{
			speakingCount++;
			(*cqIter)->m_isProminentHrtf = false;  // Until we can handle multiple streams (mixed + prominent) then all must be false
		}
	}
	return speakingCount;
}


// This is a temporary solution to set the avatar source ID and hrtf info into the mixed stream
// until support for multiple streams is added to the client side.
void AudioRouterWithMixer::SetMostProminentHrtfInMixedStream(ConnectionQueueVector& connections)
{
	ConnectionQueueVector::iterator cqIter;
	float maxAmplitudeSoFar = 0.0f;
	uint32 currentSpeaker = m_mixedHrtfInfo.sourceID;
	AudioPacket::HrtfInfo mostProminentSpeakerInfo;

	mostProminentSpeakerInfo.sourceID = 0;

	for (cqIter = connections.begin();
		cqIter != connections.end();
		cqIter++)
	{
		ConnectionQueuePtr connQueue = *cqIter;

		if (connQueue->m_isSpeaking)
		{	
			AudioPacket::HrtfInfo currentHrtfInfo;

			AudioPacketPtr inputAudioPacket = connQueue->m_inputAudioPacket;
			inputAudioPacket->GetHrtfForStream(0, currentHrtfInfo);

			// preference is to keep same speaker unless they are done speaking.
			if (currentHrtfInfo.sourceID == currentSpeaker)
			{
				mostProminentSpeakerInfo = currentHrtfInfo;		//pick up other changes in these values
				break;
			}

			// otherwise, find the most prominent speaker
			float inputAverageAmplitude = inputAudioPacket->GetAverageAmplitudeForStream(0);

			if (maxAmplitudeSoFar < inputAverageAmplitude)
			{
				maxAmplitudeSoFar = inputAverageAmplitude;
				mostProminentSpeakerInfo = currentHrtfInfo;			// best so far
			}
		}
	}

	m_mixedHrtfInfo = mostProminentSpeakerInfo;
}


void AudioRouterWithMixer::MixAllAudioStreams(ConnectionQueueVector& connections)
{
	m_mixedStreamCount = (uint16)connections.size();

	for (uint32 f = 0; f < m_masterSamplesPerPacket; f++)
	{
		m_mixedAudioSampleBuffer[f] = 0.0;
	}

	ConnectionQueueVector::iterator cqIter;

	for (cqIter = connections.begin();
		cqIter != connections.end();
		cqIter++)
	{
		ConnectionQueuePtr connQueue = *cqIter;
		if (connQueue->m_inputAudioPacket->GetIsMuted() == false) {
			float * audioPacketSamples = connQueue->m_inputAudioPacket->GetSamplePointerForStream(0, 0);

			for (uint32 f = 0; f < m_masterSamplesPerPacket; f++)
			{
				m_mixedAudioSampleBuffer[f] += audioPacketSamples[f];
			}
		}
	}
}


float AudioRouterWithMixer::CalculateTotalAverageAmplitude(ConnectionQueueVector& prominentConnections) {

	// First calculate amplitude of mixed audio

	float averageAmplitude = 0.0f;

	for (uint32 f = 0; f < m_masterSamplesPerPacket; f++)
	{
		averageAmplitude += fabs(m_mixedAudioSampleBuffer[f]);
	}

	averageAmplitude /= m_masterSamplesPerPacket;

	// Now add in averages for each of the prominent streams.

	ConnectionQueueVector::iterator cqIter;

	for (cqIter = prominentConnections.begin();
		cqIter != prominentConnections.end();
		cqIter++)
	{
		averageAmplitude += (*cqIter)->m_outputAudioPacket->GetAverageAmplitudeForStream(0);
	}

	averageAmplitude /= (prominentConnections.size() + 1);   // plus 1 for mixed audio
	return averageAmplitude;
}


void AudioRouterWithMixer::LogStatistics() {
	static size_t s_lastConnectionCount = 0;
	static float  s_averageAmplitude = 0.0;
	static uint32 s_amplitudeMeasurementCount = 0;
	static uint64 s_lastOneSecondSummaryTime = 0;

	s_amplitudeMeasurementCount++;
	s_averageAmplitude += m_averageAmplitude;

	if (s_lastOneSecondSummaryTime == 0 || s_lastOneSecondSummaryTime < m_currentProcessingTime - 1000000)
	{
		string connectionStats = "Connections [connected,qsize,maxlat,needcomp,speaking,totpkts]:";
		ConnectionQueueMap::iterator cqIter;

		for (cqIter = m_connectionQueueMap.begin();
			cqIter != m_connectionQueueMap.end();
			cqIter++)
		{
			ConnectionQueuePtr cq = cqIter->second;
			char statsBuffer[1024];
			float maxTimeMS;

			maxTimeMS = (float)cq->m_maxTimeBetweenPackets / 1000.0f;

			sprintf_s(statsBuffer, sizeof(statsBuffer), "[%s, %lld, %.2f, %ld, %s, %ld] ",
				cq->m_audioConnection->IsConnected() ? "Y" : "N",
				cq->m_audioPacketQueue.size(),
				maxTimeMS,
				cq->m_latePacketNeedingCompensationCount,
				cq->m_isSpeaking ? "Y" : "N",
				cq->m_totalPacketsReceived
				);

			cq->m_maxTimeBetweenPackets = 0;
			connectionStats += statsBuffer;
		}

		//LogInfo("Audio: avg vol:%f   max q:%d. %s",
		//	s_averageAmplitude / s_amplitudeMeasurementCount, m_maxPacketsWaiting, connectionStats.c_str());
		s_averageAmplitude = 0.0;
		s_amplitudeMeasurementCount = 0;
		s_lastOneSecondSummaryTime = m_currentProcessingTime;
	}
}


void AudioRouterWithMixer::CreateMinusOneOutputStreams(ConnectionQueueVector& connections, uint16 whichInStream)
{
	if (m_mixedStreamCount)
	{
		ConnectionQueueVector::iterator cqIter;

		for (cqIter = connections.begin();
			cqIter != connections.end();
			cqIter++)
		{
			if (!(*cqIter)->IsProminentHrtf())
			{
				(*cqIter)->AddMixMinusSelfToOutputStream(whichInStream, m_mixedAudioSampleBuffer, m_mixedHrtfInfo, m_removeSelfFromAudio);
			}
		}
	}
}

void AudioRouterWithMixer::CreateHrtfOutputStreams(ConnectionQueueVector& readyConnections, ConnectionQueueVector& prominentConnections)
{
	if (prominentConnections.size())
	{
		bool includeSelf = (m_removeSelfFromAudio == false && prominentConnections.size() <= m_prominentHrtfStreamCount);

		ConnectionQueueVector::iterator cqProminentIter;

		for (cqProminentIter = readyConnections.begin();
			cqProminentIter != readyConnections.end();
			cqProminentIter++)
		{
			uint16 whichInStream = 0;
			AudioConnectionGUID prominentGUID = (*cqProminentIter)->GetConnectionGUID();
			AudioPacketPtr inProminentAudioPacket = (*cqProminentIter)->m_inputAudioPacket;
			ConnectionQueueVector::iterator cqOutIter;

			for (cqOutIter = readyConnections.begin();
				cqOutIter != readyConnections.end();
				cqOutIter++)

			if (includeSelf || (*cqOutIter)->GetConnectionGUID() != prominentGUID)
			{
				(*cqOutIter)->AddHrtfOutputStream(whichInStream, inProminentAudioPacket);
			}
		}
	}
}

void AudioRouterWithMixer::PrepareLatePackets(ConnectionQueueVector& lateConnections)
{
	if (m_mixedStreamCount)
	{
		ConnectionQueueVector::iterator cqIter;

		for (cqIter = lateConnections.begin();
			cqIter != lateConnections.end();
			cqIter++)
		{
			(*cqIter)->PrepareLatePacket(m_mixedAudioSampleBuffer, m_mixedHrtfInfo);
		}
	}

	// if no mixed streams, then silence will be sent instead.
}

void AudioRouterWithMixer::ReturnAudioPackets(ConnectionQueueVector& connections)
{
	ConnectionQueueVector::iterator cqIter;

	for (cqIter = connections.begin();
		cqIter != connections.end();
		cqIter++)
	{
		(*cqIter)->ReturnAudioPacketsToPool();
	}
}


void AudioRouterWithMixer::SendOutputAudioPackets(ConnectionQueueVector& connections)
{
	ConnectionQueueVector::iterator cqIter;

	for (cqIter = connections.begin();
		cqIter != connections.end();
		cqIter++)
	{
		(*cqIter)->SendAudioPacket();
	}
}


void AudioRouterWithMixer::BalanceQueues(ConnectionQueueVector& connections)
{
	if (m_maxPacketsWaiting - m_minPacketsWaiting >= kMinPacketWaitingDeltaToBalance)
	{
		uint32 targetPacketCount = max(2, m_2ndToMaxPacketsWaiting - 1); // minus one because ready connections have been popped.
		//uint32 targetPacketCount = m_minPacketsWaiting + 1;
		uint64 oldestReceiveTime = m_currentProcessingTime - kMaxPacketAgeBeforeBalancingInMicroseconds;
		ConnectionQueueVector::iterator cqIter;

		for (cqIter = connections.begin();
			cqIter != connections.end();
			cqIter++)
		{
			(*cqIter)->BalanceQueue(targetPacketCount, oldestReceiveTime, m_maxAmplitudeForBalancingPackets);
		}
	}
}


//////////////////////////////////////////////////////
// ConnectionQueue methods
//////////////////////////////////////////////////////

AudioRouterWithMixer::ConnectionQueue::ConnectionQueue(
	AudioConnectionPtr audioConnection,
	AudioPacketPoolPtr& audioPacketPool,
	uint32 masterSampleRate,
	uint16 masterSamplesPerPacket,
	bool streamToWavFile)
	: m_audioConnection(audioConnection)
	, m_audioPacketPool(audioPacketPool)
	, m_state(State::StartingUp)
	, m_masterSampleRate(masterSampleRate)		        // expected sample rate for all received and fabricated packets
	, m_masterSamplesPerPacket(masterSamplesPerPacket)	// expected sample count for all received and sent packets
	, m_lastPacketWasMuted(false)
	, m_totalLatePacketCount(0)
	, m_latePacketNeedingCompensationCount(0)
	, m_lateAndIgnoredSinceLastPacket(0)
	, m_streamToWavFile(streamToWavFile)
	, m_totalPacketsReceived(0)
	, m_timeFirstPacketReceived(0)
	, m_timeLastPacketReceived(0)
	, m_maxTimeBetweenPackets(0)
	, m_isProminentHrtf(false)
	, m_isSendingHrtf(false)
	, m_isSpeaking(false)
	, m_speechAmplitudeThreshold(0.0015f)			//  0.01 start.
	, m_speechPacketThreshold(20)
	, m_silencePacketThreshold(50)					// 100ms start.
{
	if (m_streamToWavFile) {
		char fileName[1024];

		sprintf_s(fileName, sizeof(fileName), "xtools_wav_%08llx.wav", audioConnection->GetConnectionGUID());
		m_wavFile = new AudioWavFile(fileName);
		m_wavFile->WriteHeader(m_masterSampleRate, 1);

		LogInfo("*** WARNING: Writing all samples to WAV file %s in current working directory. ***", fileName);
	}
}


AudioRouterWithMixer::ConnectionQueue::~ConnectionQueue(void)
{
	if (m_streamToWavFile)
	{
		m_wavFile->WriteFinalize();
	}
}


void AudioRouterWithMixer::ConnectionQueue::Push(uint64 timeReceived, AudioPacketPtr& audioPacket)
{
	m_audioPacketQueue.push(audioPacket);

	// Gather statistics
	if (m_timeLastPacketReceived)
	{
		uint64 deltaTime = max(1, timeReceived - m_timeLastPacketReceived);

		if (m_maxTimeBetweenPackets < deltaTime || !m_maxTimeBetweenPackets)
		{
			m_maxTimeBetweenPackets = deltaTime;
		}
	}

	m_timeLastPacketReceived = timeReceived;
	m_totalPacketsReceived++;

	m_isSendingHrtf = audioPacket->StreamHasHrtf(0);  // NOTE: incoming packets assumed to only provide one stream

	switch (m_state)
	{
	case State::StartingUp:
		m_state = State::FirstPacket;
		m_timeFirstPacketReceived = timeReceived;
		break;

	case State::FirstPacket:
		m_state = State::Normal;
		break;

	case State::HopelesslyLate:
		m_state = State::Normal;
		break;

	case State::Normal:
		break;
	}
}


AudioPacketPtr AudioRouterWithMixer::ConnectionQueue::Peek(void)
{
	return m_audioPacketQueue.front();
}


AudioPacketPtr AudioRouterWithMixer::ConnectionQueue::Pop(void)
{
	AudioPacketPtr audioPacket = m_audioPacketQueue.front();

	m_lastPacketWasMuted = audioPacket->GetIsMuted();
	m_lateAndIgnoredSinceLastPacket = 0;  // reset this for next time
	m_audioPacketQueue.pop();
	return audioPacket;
}


bool AudioRouterWithMixer::ConnectionQueue::IsEmpty(void) const
{
	return m_audioPacketQueue.size() == 0;
}


bool AudioRouterWithMixer::ConnectionQueue::IsProminentHrtf(void) const
{
	return m_isProminentHrtf;
}


bool AudioRouterWithMixer::ConnectionQueue::HasBeenMuted(void) const
{
	return m_lastPacketWasMuted || m_audioConnection->IsMuted();
}


void AudioRouterWithMixer::ConnectionQueue::MarkLateAndIgnored(void)
{
	m_totalLatePacketCount++;
	m_latePacketNeedingCompensationCount++;
	if (++m_lateAndIgnoredSinceLastPacket > kMaxLatePacketsForHopelesslyLate)
	{
		m_state = State::HopelesslyLate;
	}
}


bool AudioRouterWithMixer::ConnectionQueue::IsHopelesslyLate(void) const
{
	return m_state == State::HopelesslyLate;
}


bool AudioRouterWithMixer::ConnectionQueue::IsNewConnection(void) const
{
	return m_totalPacketsReceived < 20;
}



void AudioRouterWithMixer::ConnectionQueue::SendAudioPacket(void)
{
	if (m_outputAudioPacket->GetStreamCount() == 0)
	{
		m_outputAudioPacket->AddStream(true);
	}

	if (m_streamToWavFile)
	{
		m_wavFile->WriteSamples(m_outputAudioPacket->GetSamplePointerForStream(0, 0), m_outputAudioPacket->GetSampleCount());
	}

	m_audioConnection->SendAudioPacket(m_outputAudioPacket);

}


void AudioRouterWithMixer::ConnectionQueue::SendAudioPacket(AudioPacketPtr& audioPacket)
{
	if (m_streamToWavFile)
	{
		m_wavFile->WriteSamples(audioPacket->GetSamplePointerForStream(0, 0), audioPacket->GetSampleCount());
	}

	m_audioConnection->SendAudioPacket(audioPacket);

}


void AudioRouterWithMixer::ConnectionQueue::PrepareInputAndOutputAudioPackets(uint16 streamCount)
{
	uint64 timeReceived;
	uint32 sequenceNumber;

	if (m_audioPacketQueue.size() > 0)
	{
		m_inputAudioPacket = Pop();
		m_inputAverageAmplitude = m_inputAudioPacket->GetAverageAmplitudeForStream(0);
		timeReceived = m_inputAudioPacket->GetTimeReceived();
		sequenceNumber = m_inputAudioPacket->GetSequenceNumber();
	}
	else
	{
		m_inputAudioPacket = NULL;
		timeReceived = 0;
		sequenceNumber = 0;
	}

	uint16 channelCount = 1;
	uint32 totalSamples = streamCount * channelCount * m_masterSamplesPerPacket;
	m_outputAudioPacket = m_audioPacketPool->AcquireAudioPacket(totalSamples);
	m_outputAudioPacket->Initialize(timeReceived, 0, channelCount, m_masterSamplesPerPacket, m_masterSampleRate, false, sequenceNumber);
}


void AudioRouterWithMixer::ConnectionQueue::AddMixMinusSelfToOutputStream(uint16 whichInStream, float * mixedAudioSampleBuffer, AudioPacket::HrtfInfo& hrtfInfo, bool removeSelfFromAudio)
{
	uint16 channelCount = 1;
	uint32 sampleCount = m_masterSamplesPerPacket * channelCount;
	uint16 whichOutStream = m_outputAudioPacket->GetStreamCount();

	if (m_outputAudioPacket->AddStream())
	{
		m_outputAudioPacket->SetHrtfForStream(whichOutStream, hrtfInfo);

		float * outSamples = m_outputAudioPacket->GetSamplePointerForStream(whichOutStream, 0);
		XTASSERT(outSamples != NULL);

		if (removeSelfFromAudio && !m_lastPacketWasMuted && m_inputAudioPacket != NULL)
		{
			float * inSamples = m_inputAudioPacket->GetSamplePointerForStream(whichInStream, 0);
			while (sampleCount--)
			{
				*outSamples++ = Clamp(*mixedAudioSampleBuffer++ - *inSamples++, -1.0f, 1.0f);
			}
		}
		else
		{
			while (sampleCount--)
			{
				*outSamples++ = Clamp(*mixedAudioSampleBuffer++, -1.0f, 1.0f);
			}
		}
	}
}


void AudioRouterWithMixer::ConnectionQueue::AddHrtfOutputStream(uint16 whichInStream, AudioPacketPtr& inAudioPacket)
{
	uint16 channelCount = 1;
	uint32 sampleCount = m_masterSamplesPerPacket * channelCount;
	uint16 whichOutStream = m_outputAudioPacket->GetStreamCount();

	if (m_outputAudioPacket->AddStream())
	{
		AudioPacket::HrtfInfo hrtfInfo;
		float * inSamples = inAudioPacket->GetSamplePointerForStream(whichInStream, 0);
		float * outSamples = m_outputAudioPacket->GetSamplePointerForStream(whichOutStream, 0);

		XTASSERT(inSamples != NULL);
		XTASSERT(outSamples != NULL);

		if (inAudioPacket->GetIsMuted())
		{
			while (sampleCount--)
			{
				*outSamples++ = 0.0f;
			}
		}
		else
		{
			while (sampleCount--)
			{
				*outSamples++ = Clamp(*inSamples++, -1.0f, 1.0f);
			}
		}

		inAudioPacket->GetHrtfForStream(whichInStream, hrtfInfo);
		m_outputAudioPacket->SetHrtfForStream(whichOutStream, hrtfInfo);
	}
}


void AudioRouterWithMixer::ConnectionQueue::PrepareLatePacket(float * mixedAudioSampleBuffer, AudioPacket::HrtfInfo& hrtfInfo)
{
	uint16 whichInStream = 0;
	AddMixMinusSelfToOutputStream(whichInStream, mixedAudioSampleBuffer, hrtfInfo, false);
}


void AudioRouterWithMixer::ConnectionQueue::ReturnAudioPacketsToPool(void)
{
	if (m_inputAudioPacket != NULL)
	{
		m_audioPacketPool->ReturnAudioPacket(m_inputAudioPacket);
		m_inputAudioPacket = NULL;
	}

	if (m_outputAudioPacket != NULL)
	{
		m_audioPacketPool->ReturnAudioPacket(m_outputAudioPacket);
		m_outputAudioPacket = NULL;
	}
}


uint32 AudioRouterWithMixer::ConnectionQueue::QueuePacketCount(void)
{
	return (uint32)m_audioPacketQueue.size();
}


void AudioRouterWithMixer::ConnectionQueue::BalanceQueue(uint32 targetPacketCount, uint64 oldestAllowedTimeReceived, float maxAmplitudeForBalancingPackets)
{
	bool getRidOfPacket = true;

	while (getRidOfPacket && m_audioPacketQueue.size() > targetPacketCount) {
		AudioPacketPtr audioPacket = m_audioPacketQueue.front();
		getRidOfPacket = false;

		if (audioPacket->GetTimeReceived() < oldestAllowedTimeReceived)
		{
			getRidOfPacket = true;
#if defined(AUDIO_DEBUG)
			LogInfo("Audio: Balancing audio queue for connection %08lx.  Too old.", m_audioConnection->GetConnectionGUID());
#endif
		}
		else if (m_latePacketNeedingCompensationCount > 0 || IsNewConnection()) 
		{
			float avgAmplitude = audioPacket->GetAverageAmplitudeForStream(0);

			if (avgAmplitude < maxAmplitudeForBalancingPackets)
			{
				getRidOfPacket = true;
				if (!IsNewConnection() && m_latePacketNeedingCompensationCount > 0)
				{
					m_latePacketNeedingCompensationCount--;
				}
#if defined(AUDIO_DEBUG)
				LogInfo("Audio: Balancing audio queue for connection %08lx.  Silent.", m_audioConnection->GetConnectionGUID());
#endif
			}
		}

		if (getRidOfPacket)
		{
			m_audioPacketQueue.pop();
			m_audioPacketPool->ReturnAudioPacket(audioPacket);
		}
		}
	}



bool AudioRouterWithMixer::ConnectionQueue::ProcessSpeakingDetection(void)
{
	if (m_state == State::Normal)
	{
		if (m_inputAverageAmplitude > m_speechAmplitudeThreshold)
		{
			if (++m_continuousSpeechPacketCount > m_speechPacketThreshold)
			{
				m_isSpeaking = true;
			}
			m_continuousSilencePacketCount = 0;
		}
		else
		{
			if (++m_continuousSilencePacketCount > m_silencePacketThreshold)
			{
				m_isSpeaking = false;
			}
			m_continuousSpeechPacketCount = 0;
		}
	}

	return m_isSpeaking;
}


bool AudioRouterWithMixer::ConnectionQueue::IsWaitCandidate(void) const
{
	return m_state == State::Normal && IsEmpty() && !IsNewConnection() && !HasBeenMuted();
}


AudioConnectionGUID AudioRouterWithMixer::ConnectionQueue::GetConnectionGUID(void) const
{
	return m_audioConnection->GetConnectionGUID();
}


XTOOLS_NAMESPACE_END