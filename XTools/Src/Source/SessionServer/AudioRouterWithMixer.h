//////////////////////////////////////////////////////////////////////////
// AudioRouterWithMixer.h
//
// Manages aligning and routing audio to the right places including
// mixing some audio streams while keeping prominent streams separate
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

#include "AudioRouterWithMixer.h"
#include <Private/AudioProcessor.h>
#include <Private/AudioConnection.h>
#include <Private/AudioPacketPool.h>
#include <Private/AudioWavFile.h>

#include <queue>


XTOOLS_NAMESPACE_BEGIN

class AudioRouterWithMixer : public AudioProcessor
{

private:

	class ConnectionQueue : public RefCounted {
	public:
		enum State
		{
			StartingUp,		// Never wait for a connection that is just starting up and has not yet sent a packet
			FirstPacket,	// Used to conditionally introduce a one packet delay to avoid future starvation
			Normal,			// Normal operation where packets are arriving in a timely fashion
			HopelesslyLate	// Used to avoid causing other conneections to wait for this one
		};

		static const uint32 kMaxLatePacketsForHopelesslyLate = 10;	//keeps a stalled connection from permanently causing all others to wait and introducing unnecessary latency

		ConnectionQueue(AudioConnectionPtr audioConnection, AudioPacketPoolPtr& audioPacketPool, uint32 masterSampleRate, uint16 masterSamplesPerPacket, bool streamToWavFile);
		~ConnectionQueue(void);
		void Push(uint64 currentTimeInMicroseconds, AudioPacketPtr& audioPacket);
		AudioPacketPtr Peek(void);
		AudioPacketPtr Pop(void);
		bool IsEmpty(void) const;
		bool IsHopelesslyLate(void) const;
		bool IsNewConnection(void) const;
		bool IsWaitCandidate(void) const;
		bool IsProminentHrtf(void) const;
		bool HasBeenMuted(void) const;
		void MarkLateAndIgnored(void);
		void PrepareInputAndOutputAudioPackets(uint16 streamCount);
		void ReturnAudioPacketsToPool(void);
		uint32 QueuePacketCount(void);
		void SendAudioPacket(void);
		void SendAudioPacket(AudioPacketPtr& audioPacket);
		void AddMixMinusSelfToOutputStream(uint16 whichInStream, float * mixedAudioSampleBuffer, AudioPacket::HrtfInfo& hrtfInfo, bool removeSelfFromAudio);
		void AddHrtfOutputStream(uint16 whichInStream, AudioPacketPtr& inHrtfPacket);
		void PrepareLatePacket(float * mixedAudioSampleBuffer, AudioPacket::HrtfInfo& hrtfInfo);
		void BalanceQueue(uint32 targetPacketCount, uint64 oldestTimeReceived, float maxAmplitudeForBalancingPackets);
		bool ProcessSpeakingDetection(void);
		AudioConnectionGUID GetConnectionGUID(void) const;

		inline float Clamp(float x, float a, float b) {
			return x < a ? a : (x > b ? b : x);
		}

		AudioConnectionPtr          m_audioConnection;
		AudioPacketPoolPtr			m_audioPacketPool;
		State						m_state;
		bool                        m_isProminentHrtf;
		bool                        m_isSendingHrtf;
		uint32                      m_masterSampleRate;
		uint16                      m_masterSamplesPerPacket;
		bool                        m_streamToWavFile;
		std::queue<AudioPacketPtr>  m_audioPacketQueue;
		uint32					    m_lateAndIgnoredSinceLastPacket;
		uint32						m_latePacketNeedingCompensationCount;
		bool                        m_lastPacketWasMuted;

		float                       m_speechAmplitudeThreshold;
		uint32                      m_speechPacketThreshold;
		uint32                      m_silencePacketThreshold;

		// working data for immediate processing
		float                       m_inputAverageAmplitude;
		AudioPacketPtr              m_inputAudioPacket;   //place to access current input packet
		AudioPacketPtr              m_outputAudioPacket;  //place to build output packet

		// for hrtf prominent speaker detection
		uint32                      m_continuousSpeechPacketCount;
		uint32                      m_continuousSilencePacketCount;
		bool                        m_isSpeaking;

		// statistics and other diagnostics info
		uint32                      m_totalPacketsReceived;
		uint64                      m_timeFirstPacketReceived;
		uint64                      m_timeLastPacketReceived;
		uint64                      m_maxTimeBetweenPackets;
		uint32						m_totalLatePacketCount;
		AudioWavFilePtr             m_wavFile;
	};

	DECLARE_PTR(ConnectionQueue)

	typedef std::vector<ConnectionQueuePtr> ConnectionQueueVector;
	typedef std::map<AudioConnectionGUID, ConnectionQueuePtr> ConnectionQueueMap;



public:

	static const float kDefaultMaxAmplitudeForBalancingPackets;				// value must be declared in cpp file
	static const uint64 kDefaultMaxTimeToWaitForLatePacketInMilliseconds = 250;
	static const uint64 kMaxPacketAgeBeforeBalancingInMicroseconds = 200 * 1000;
	static const uint32 kMinPacketWaitingDeltaToBalance = 2;


	AudioRouterWithMixer(AudioPacketPoolPtr& audioPacketPool, uint32 masterSampleRate, uint16 masterSamplesPerPacket, uint16 prominentHrtfStreamCount);

	// AudioProcessor Methods
	virtual void ProcessAudioPacket(AudioConnectionPtr& audioConnection, AudioPacketPtr& audioPacket) XTOVERRIDE;

	// Other methods
	void AddAudioConnection(AudioConnectionPtr& audioConnection);
	void RemoveAudioConnection(AudioConnectionPtr& audioConnection);
	void ProcessAllStateChanges(uint64 currentTimeInMicroseconds);
	void MixAllAudioStreams(ConnectionQueueVector& connections);
	void PrepareInputAndOutputAudioPackets(ConnectionQueueVector& connections, uint16 streamCount);
	void CreateMinusOneOutputStreams(ConnectionQueueVector& connections, uint16 whichInStream);
	void CreateHrtfOutputStreams(ConnectionQueueVector& readyConnections, ConnectionQueueVector& prominentConnections);
	void PrepareLatePackets(ConnectionQueueVector& lateConnections);
	void ReturnAudioPackets(ConnectionQueueVector& connections);
	void SendOutputAudioPackets(ConnectionQueueVector& connections);
	bool AreConnectionsReadyForProcessing(ConnectionQueueVector& readyConnectionsOut, ConnectionQueueVector& lateConnectionsOut);
	void BalanceQueues(ConnectionQueueVector& connections);
	void IdentifyProminentHrtfStreams(ConnectionQueueVector& readyConnections, ConnectionQueueVector& prominentConnections, ConnectionQueueVector& mixedConnections);
	uint16 CalculateOutputStreamCount(void);
	float CalculateTotalAverageAmplitude(ConnectionQueueVector& prominentConnections);
	bool DelayPacket(ConnectionQueuePtr& connQueue, uint64 oldestTimeReceived);
	void LogStatistics();
	uint64 GetCurrentMicrosecondsSinceEpoch(void);
	uint32 ProcessSpeakingDetection(ConnectionQueueVector& connections);
	void SetMostProminentHrtfInMixedStream(ConnectionQueueVector& connections);


private:
	// parameters
	uint32                m_masterSampleRate;
	uint16                m_masterSamplesPerPacket;
	uint64                m_maxTimeToWaitForLatePacketInMilliseconds;				// max milliseconds between earliest and latest packet
	AudioPacketPoolPtr    m_audioPacketPool;
	ConnectionQueueMap    m_connectionQueueMap;
	uint16				  m_prominentHrtfStreamCount;
	bool				  m_removeSelfFromAudio;
	float                 m_maxAmplitudeForBalancingPackets;

	// for one iteration of audio processing
	uint64                m_currentProcessingTime;
	float                 m_averageAmplitude;
	float                 m_mixedAudioSampleBuffer[AudioPacket::kMaxSampleCount];
	uint32                m_mixedStreamCount;
	AudioPacket::HrtfInfo m_mixedHrtfInfo;		// temporary solution of putting most prominent speaker hrtf info into the mixed audio packet

	// for efficiency, these are allocated here once instead of on stack for each round of processing.

	// TODO(michhoff) Using these vectors (instead of stack allocated vectors) to reduce memory allocations
	// appears to cause the ref_ptrs to be remove references more than they should be when clear() is
	// called and this causes crashes. Until this can be debugged, they are instantiated locally in 
	// the ProcessAllStateChanges() method.

	/*******
	ConnectionQueueVector m_readyConnections;
	ConnectionQueueVector m_lateConnections;
	ConnectionQueueVector m_prominentConnections;
	ConnectionQueueVector m_mixedConnections;
	******/

	// used for late packet arrival logic
	uint32                m_maxPacketsWaiting;
	uint32                m_2ndToMaxPacketsWaiting;
	uint32                m_minPacketsWaiting;

	// used for diagnostics
	uint32				  m_waitCandidatePacketCountThisPeriod;
	uint32                m_latePacketCountThisPeriod;
	uint32                m_totalLatePacketCount;
	bool                  m_streamToWavFile;

};

DECLARE_PTR(AudioRouterWithMixer)

XTOOLS_NAMESPACE_END