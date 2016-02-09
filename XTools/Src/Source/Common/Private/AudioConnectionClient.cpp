//////////////////////////////////////////////////////////////////////////
// AudioCnnectionBaraboo.cpp
//
// Manages one network connection for audio processing with
// network traffic formatted for Baraboo//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AudioConnectionClient.h"

XTOOLS_NAMESPACE_BEGIN

AudioConnectionClient::AudioConnectionClient(const NetworkConnectionPtr& connection, AudioPacketPoolPtr& audioPacketPool)
: m_connection(connection)
, m_audioPacketPool(audioPacketPool)
, m_isMuted(false)
{
	connection->AddListener(MessageID::AudioSamples, this);
	m_isConnected = connection->IsConnected();

}

ConnectionGUID AudioConnectionClient::GetConnectionGUID(void) const
{
	return m_connection->GetConnectionGUID();
}


bool AudioConnectionClient::IsMuted(void) const
{
	return m_isMuted;
}


bool AudioConnectionClient::IsConnected(void) const
{
	return m_isConnected;
}


bool AudioConnectionClient::SupportsHrtf(void) const
{
	return true;
}


void AudioConnectionClient::OnConnected(const NetworkConnectionPtr& connection)
{
	XT_UNREFERENCED_PARAM(connection);	// Necessary for when XTASSET is not defined

	m_isConnected = true;
}

void AudioConnectionClient::OnConnectFailed(const NetworkConnectionPtr& connection)
{
	XT_UNREFERENCED_PARAM(connection);	// Necessary for when XTASSET is not defined

	m_isConnected = false;
}

void AudioConnectionClient::OnDisconnected(const NetworkConnectionPtr& connection)
{
	XT_UNREFERENCED_PARAM(connection);	// Necessary for when XTASSET is not defined

	m_isConnected = false;
}

void AudioConnectionClient::OnMessageReceived(const NetworkConnectionPtr& connection, NetworkInMessage& message)
{
	XT_UNREFERENCED_PARAM(connection);	// Necessary for when XTASSET is not defined

	uint64 timeReceivedAsMicrosecondsSinceEpoch =
		std::chrono::duration_cast<std::chrono::microseconds>(
		std::chrono::high_resolution_clock::now().time_since_epoch()).count();

	ProcessInMessage(message, timeReceivedAsMicrosecondsSinceEpoch);
}


void AudioConnectionClient::ProcessInMessage(NetworkInMessage& inMessage, uint64 timeReceivedAsMicrosecondsSinceEpoch)
{
	// TODO(michhoff): Need a way to peek at stream, channel and sample counts from incoming message to ask for correct audio packet from pool
	if ( m_processorPipelineIn.size() > 0 )
	{
		AudioPacketPtr audioPacket = m_audioPacketPool->AcquireAudioPacket(m_audioPacketPool->GetDefaultMaxSampleCount());
		ConvertNetworkInMessageToAudioPacket(inMessage, timeReceivedAsMicrosecondsSinceEpoch, audioPacket);
		AudioConnectionPtr audioConnection = this;

		for ( auto itor = m_processorPipelineIn.begin(); itor != m_processorPipelineIn.end(); ++itor)
		{
			(*itor)->ProcessAudioPacket(audioConnection, audioPacket);
		}
	}
}

void AudioConnectionClient::SendAudioPacket(AudioPacketPtr& audioPacket)
{
	// Run packet through out-bound pipeline.
	if ( m_processorPipelineOut.size() > 0 )
	{
		AudioConnectionPtr audioConnection = this;
		for ( auto itor = m_processorPipelineOut.begin(); itor != m_processorPipelineOut.end(); ++itor )
		{
			(*itor)->ProcessAudioPacket(audioConnection, audioPacket);
		}
	}

	NetworkOutMessagePtr outMessage = m_connection->CreateMessage(MessageID::AudioSamples);
	ConvertAudioPacketToNetworkOutMessage(audioPacket, outMessage);

	if (m_connection->IsConnected())
	{
		m_connection->Send(outMessage, MessagePriority::Immediate, MessageReliability::ReliableOrdered, MessageChannel::Audio, true);
	}
}


void AudioConnectionClient::ConvertNetworkInMessageToAudioPacket(NetworkInMessage& inMessage, uint64 timeReceivedAsMicrosecondsSinceEpoch, AudioPacketPtr& audioPacket)
{
	uint32 sampleRate = 0;
	uint32 sampleCount = 0;
	uint32 sequenceNumber;
	uint16 version;
	AudioSampleRateType sampleRateType;
	AudioSampleDataType sampleDataType;
	AudioCodecType codecType;

	PackedAudioHeader packedHdr;
	uint16 bytesRead = 0;

	byte headerSize = inMessage.ReadByte();
	bytesRead += sizeof(byte);

	uint32 * packedHeaderAsUInt32Ptr = (uint32 *)&packedHdr;

	*packedHeaderAsUInt32Ptr = inMessage.ReadUInt32();
	bytesRead += sizeof(uint32);

	version = packedHdr.version;
	uint32 incomingSampleCount = packedHdr.sampleCount;
	sampleDataType = (AudioSampleDataType)packedHdr.sampleDataType;
	sampleRateType = (AudioSampleRateType)packedHdr.sampleRateType;
	codecType = (AudioCodecType)packedHdr.codecType;
	sequenceNumber = (uint32)packedHdr.reserved;  // TODO(michhoff) This is used for debugging latency

	m_isMuted = packedHdr.isMuted;	// TODO(michhoff) - until we have a separate message packet for sending/receiving changes in muted state

	if (version == 0)
	{
		version = inMessage.ReadByte();	//get optional extended version
		bytesRead += 1;
	}

	switch (packedHdr.sampleRateType)
	{
	case AudioSampleRateType::Extended:
		XTASSERT(false); // not supported
		sampleRate = inMessage.ReadUInt32();
		bytesRead += sizeof(uint32);
		break;

	case AudioSampleRateType::Rate16000:
		sampleRate = 16000;
		sampleCount = (48000 / 16000) * incomingSampleCount;
		break;

	case AudioSampleRateType::Rate44100:
		XTASSERT(false); // not supported
		sampleRate = 44100;
		break;

	case AudioSampleRateType::Rate48000:
		sampleRate = 48000;
		sampleCount = incomingSampleCount;
		break;
	}

	XTASSERT(version == 1);
	XTASSERT(sampleDataType == AudioSampleDataType::Float);
	XTASSERT(codecType == AudioCodecType::None);


	// throw away any header padding (or future header info we don't know about)
	while (bytesRead < headerSize)
	{
		inMessage.ReadByte();
		bytesRead++;
	}

	uint32 packetSampleRate = 48000; // force to 48000 - we're resampling, if needed
	audioPacket->Initialize(timeReceivedAsMicrosecondsSinceEpoch, packedHdr.streamCount, packedHdr.channelCount, sampleCount, packetSampleRate, m_isMuted, sequenceNumber);

	if (packedHdr.isMuted == 0) {
		for (uint16 stream = 0; stream < packedHdr.streamCount; stream++) {
			AudioPacket::HrtfInfo hrtf;

			inMessage.ReadFloat();	// read, but throw away any average amplitude coming from client (For now)

			hrtf.sourceID = inMessage.ReadUInt32();
			if (hrtf.sourceID != 0)
			{
				hrtf.position.x = inMessage.ReadFloat();
				hrtf.position.y = inMessage.ReadFloat();
				hrtf.position.z = inMessage.ReadFloat();
				hrtf.orientation.x = inMessage.ReadFloat();
				hrtf.orientation.y = inMessage.ReadFloat();
				hrtf.orientation.z = inMessage.ReadFloat();
			}
			else
			{
				hrtf.position.x = hrtf.position.y = hrtf.position.z = 0.0;
				hrtf.orientation.x = hrtf.orientation.y = hrtf.orientation.z = 0.0;
			}

			audioPacket->SetHrtfForStream(stream, hrtf);

			for (uint16 channel = 0; channel < packedHdr.channelCount; channel++)
			{
				float * sampleOutPtr = audioPacket->GetSamplePointerForStream(stream, channel);

				if (sampleCount)
				{
					inMessage.ReadArray((byte *)sampleOutPtr, incomingSampleCount * sizeof(float));

					if (incomingSampleCount != sampleCount)
					{
						// resample from 16000 to 48000 - everything after here assumes 48000
						XTASSERT(sampleRate == 16000 && packetSampleRate == 48000);
						for (int ii = incomingSampleCount - 1; ii >= 0; --ii)
						{
							sampleOutPtr[ii * 3 + 2] = sampleOutPtr[ii];
							sampleOutPtr[ii * 3 + 1] = sampleOutPtr[ii];
							sampleOutPtr[ii * 3 + 0] = sampleOutPtr[ii];
						}
					}
				}
			}
		}
	}
}


void AudioConnectionClient::ConvertAudioPacketToNetworkOutMessage(AudioPacketPtr& audioPacket, NetworkOutMessagePtr& outMessage)
{
	byte currentVersion = kCurrentAudioPacketVersion;
	byte headerSize = sizeof(byte)+sizeof(uint32);
	uint16 streamCount = audioPacket->GetStreamCount();
	uint16 channelCount = audioPacket->GetChannelCount();
	uint32 sampleCount = audioPacket->GetSampleCount();
	uint32 sampleRate = audioPacket->GetSampleRate();

	AudioSampleRateType rateType;

	if (sampleRate == 16000)
	{
		rateType = AudioSampleRateType::Rate16000;
	}
	else if (sampleRate == 44100)
	{
		rateType = AudioSampleRateType::Rate44100;
	}
	else if (sampleRate == 48000)
	{
		rateType = AudioSampleRateType::Rate48000;
	}
	else
	{
		rateType = AudioSampleRateType::Extended;
		headerSize += sizeof(uint32);	// for extended rate value
	}

	XTASSERT(streamCount <= AudioPacket::kMaxStreamCount);
	XTASSERT(channelCount <= AudioPacket::kMaxChannelCount);
	XTASSERT(sampleCount <= AudioPacket::kMaxSampleCount);

	PackedAudioHeader packedHeader;

	if (currentVersion <= 7)
	{
		packedHeader.version = kCurrentAudioPacketVersion;
	}
	else
	{
		packedHeader.version = 0;
		headerSize += sizeof(byte);		// for extended version byte
	}

	packedHeader.streamCount = streamCount;
	packedHeader.channelCount = channelCount;
	packedHeader.sampleRateType = (uint16)rateType;
	packedHeader.sampleDataType = AudioSampleDataType::Float;
	packedHeader.sampleCount = sampleCount;
	packedHeader.codecType = AudioCodecType::None;
	packedHeader.isMuted = false;
	packedHeader.reserved = audioPacket->GetSequenceNumber();	// TODO(michhoff) we roundtrip the sequence number to debug latency

	outMessage->Write(headerSize);
	outMessage->Write(*(uint32*)&packedHeader);

	if (currentVersion > 7)
	{
		outMessage->Write((byte)kCurrentAudioPacketVersion);
	}

	if (rateType == AudioSampleRateType::Extended)
	{
		outMessage->Write((uint32)sampleRate);
	}

	for (uint16 stream = 0; stream < streamCount; stream++)
	{
		AudioPacket::HrtfInfo hrtf;

		audioPacket->GetHrtfForStream(stream, hrtf);
		outMessage->Write(audioPacket->GetAverageAmplitudeForStream(stream));
		outMessage->Write((uint32)hrtf.sourceID);
		if (hrtf.sourceID != 0) {
			outMessage->Write(hrtf.position.x);
			outMessage->Write(hrtf.position.y);
			outMessage->Write(hrtf.position.z);
			outMessage->Write(hrtf.orientation.x);
			outMessage->Write(hrtf.orientation.y);
			outMessage->Write(hrtf.orientation.z);
		}

		for (uint16 channel = 0; channel < channelCount; channel++)
		{
			float * sampleOutPtr = audioPacket->GetSamplePointerForStream(stream, channel);

			outMessage->WriteArray((const byte *)sampleOutPtr, sampleCount * sizeof(float));
		}
	}
}


XTOOLS_NAMESPACE_END