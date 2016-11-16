//////////////////////////////////////////////////////////////////////////
// AudioPacket.h
//
// One packet of audio information
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once

XTOOLS_NAMESPACE_BEGIN


class AudioPacketPool;

class AudioPacket : public RefCounted
{
	friend class AudioPacketPool;

public:
	static const uint16 kMaxStreamCount = 7;
	static const uint16 kMaxChannelCount = 7;
	static const uint32 kMaxSampleCount = 1023; // must be >= 960 which is 20ms @ 48KHz

	struct Vector3
	{
		float x;
		float y;
		float z;
	};

	// unit 3D vector in global coordinate space specifying direction of audio
	typedef struct HrtfInfo {
		uint32  sourceID;		// user specified audio source id for this HRTF.  Eg. Ties this stream back to the avatar who spoke it.
		Vector3	position;		// position in 3D x,y,z coordinate space.
		Vector3 orientation;	// unity orientation vector in 3D
	} HrtfInfo;


	// Initialize an AudioPacket object so that it can handle the specified maximum number of samples.
	// This approach allows the creation of a pool of objects that require no memory allocation to reuse.
	AudioPacket(uint32 sampleCountForAllChannelsAndStreams);

	// Initialize the key values of the AudioPacket.  The audio samples and other supporting data
	// can then be initialized via the GetSamplePointerForStream and SetHrtfForStream
	void Initialize(uint64 timeReceivedAsMicrosecondsSinceEpoch, uint16 initialStreamCount, uint16 channelCount, uint32 sampleCount, uint32 sampleRate, bool isMuted, uint32 sequenceNumber);

	// Add a stream to the packet.  Returns true if successful.  Returns false if already at maximum stream count for this packet.
	bool AddStream(bool isSilent = false);

	// Reset to be used before putting back into the free pool.
	void Reset(void);

	// Used to get a point to the sample data. This allows access to samples without copying them
	float * GetSamplePointerForStream(uint16 whichStream, uint16 whichChannel);


	// Used to get a point to the sample data. This allows access to samples without copying them
	float const * GetSamplePointerForStream(uint16 whichStream, uint16 whichChannel) const;

	// Set Head Related Transform Funtion (HRTF) information for a stream
	void SetHrtfForStream(uint16 whichStream, HrtfInfo& hrtf);

	// Get Head Related Transform Funtion (HRTF) information fo a stream
	void GetHrtfForStream(uint16 whichStream, HrtfInfo& hrtfOut) const;

	// Calculate average amplitude
	float GetAverageAmplitudeForStream(uint16 whichStream, bool forceRecalculate = false);

	// Determine if HRTF information was provided for the specified stream.
	bool StreamHasHrtf(uint16 whichStream) const;

	inline uint16 GetStreamCount(void) const { return m_actualStreamCount; }
	inline uint16 GetChannelCount(void) const { return m_channelCount; }
	inline uint32 GetSampleCount(void) const { return m_sampleCount; }
	inline uint32 GetSampleRate(void) const { return m_sampleRate; }
	inline uint64 GetTimeReceived(void) const { return m_timeReceived; }
	inline uint32 GetSequenceNumber(void) const { return m_sequenceNumber; }
	inline bool GetIsMuted(void) const { return m_isMuted; }

private:

	// All information we track per audio stream
	typedef struct StreamInfo {
		HrtfInfo    hrtf;
		bool        isSilent;
		bool		isAverageAmplitudeCalculated;
		float       averageAmplitude;
		float *     audioSamples;
	} StreamInfo;

	bool              m_isMuted;
	uint64            m_timeReceived;				// specified as microseconds since the epoch (1/1/1970)
	uint16            m_actualStreamCount;		    // all streams must have same number of channels and samples
	uint16            m_maxStreamCount;		        // max streams that can be supported by this packet.
	uint16            m_channelCount;				// 1=mono, 2=stereo. Other values undefined and TBD.
	uint32            m_sampleCount;				// samples per channel per stream
	uint32            m_sampleRate;					// samples per second
	StreamInfo        m_streamInfo[kMaxStreamCount];
	scoped_array<float>           m_audioSampleBuffer;			      // guaranteed to be big enough to hold m_streamCount * m_channelCount * m_sampleCount float samples
	uint32            m_audioSampleBufferAllocatedSize;
	uint32            m_sequenceNumber;


};

DECLARE_PTR(AudioPacket)

XTOOLS_NAMESPACE_END