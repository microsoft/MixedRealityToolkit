//////////////////////////////////////////////////////////////////////////
// AudioConnectionBaraboo.h
//
// Manages one network connection for audio processing with
// network traffic formatted for Baraboo
//
// Note: The separation of the AudioConnection interface is currently murky
//  and should be refactored if we add another connection type.
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once
#include <Private/AudioConnection.h>
#include <Private/AudioPacketPool.h>
#include <Private/AudioProcessor.h>

XTOOLS_NAMESPACE_BEGIN


class AudioConnectionClient : public AudioConnection, public NetworkConnectionListener
{
public:
	AudioConnectionClient(const NetworkConnectionPtr& connection, AudioPacketPoolPtr& audioPacketPool);
	void ProcessInMessage(NetworkInMessage& message, uint64 timeReceivedAsMicroscondsSinceEpoch);

	// AudioProcessor methods:
	virtual AudioConnectionGUID GetConnectionGUID(void) const XTOVERRIDE;
	virtual bool IsMuted(void) const XTOVERRIDE;
	virtual bool IsConnected(void) const XTOVERRIDE;
	virtual bool SupportsHrtf(void) const XTOVERRIDE;
	virtual void SendAudioPacket(AudioPacketPtr& audioPacket) XTOVERRIDE;

	// NetworkConnectionListener methods:
	virtual void OnMessageReceived(const NetworkConnectionPtr& connection, NetworkInMessage& message) XTOVERRIDE;
	virtual void OnConnected(const NetworkConnectionPtr& connection) XTOVERRIDE;
	virtual void OnConnectFailed(const NetworkConnectionPtr& connection) XTOVERRIDE;
	virtual void OnDisconnected(const NetworkConnectionPtr& connection) XTOVERRIDE;

private:

	static const byte kCurrentAudioPacketVersion = 1;

	enum AudioSampleRateType : byte
	{
		Extended = 0,		// read as uint32 from stream
		Rate16000,
		Rate44100,
		Rate48000
	};

	enum AudioSampleDataType : byte
	{
		Float = 0,
		UInt16
	};


	enum AudioCodecType : byte
	{
		None = 0,
		Speex
	};


	typedef struct PackedAudioHeader {
		uint32 version : 3;				// version. 0 = extended version
		uint32 streamCount : 3;			// total number of streams
		uint32 channelCount : 3;		// total number of channels per stream.  1=mono, 2=stereo
		uint32 sampleRateType : 2;	    // audio sample rate. 0 = extended sample rate, 1=16000 Hz, 2=44100 Hz, 3=48000 Hz
		uint32 sampleDataType : 2;	    // sample data type.  AudioSample::Type enum
		uint32 sampleCount : 10;		// sample count per channel per stream
		uint32 codecType : 2;			// codec type. AudioCodec::Type enum
		uint32 isMuted : 1;				// stream is muted.  If true (1), then no audio stream data expected
		uint32 reserved : 6;
	} PackedAudioHeader;

	void ConvertNetworkInMessageToAudioPacket(NetworkInMessage& inMessage, uint64 timeReceivedAsMicrosecondsSinceEpoch, AudioPacketPtr& audioPacket);
	void ConvertAudioPacketToNetworkOutMessage(AudioPacketPtr& audioPacket, NetworkOutMessagePtr& outMessage);


	const NetworkConnectionPtr	m_connection;
	AudioPacketPoolPtr			m_audioPacketPool;
	bool						m_isMuted;
	bool						m_isConnected;
};

DECLARE_PTR(AudioConnectionClient)

XTOOLS_NAMESPACE_END
