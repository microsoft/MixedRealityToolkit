//////////////////////////////////////////////////////////////////////////
// AudioSessionProcessorServer.cpp
//
// Listens for audioSample messages then processes them and distributes
// appropriately to all connections.
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AudioSessionProcessorServer.h"
#include <Private/AudioConnectionClient.h>

XTOOLS_NAMESPACE_BEGIN

AudioSessionProcessorServer::AudioSessionProcessorServer()
{
	uint16 maxStreams = 3;		// one mixed mono and 2 HRTF mono streams.
	uint16 maxChannels = 1;
	uint32 maxSamplesPerPacket = 960; // 20ms @ 48kHZ
	uint32 defaultQueueSizeSampleCount = maxSamplesPerPacket * maxStreams * maxChannels;

	m_audioPacketPool = new AudioPacketPool(defaultQueueSizeSampleCount);
	m_audioPacketPool->Reserve(defaultQueueSizeSampleCount, kAudioPacketPoolReserveCount);

	m_routerWithMixer = new AudioRouterWithMixer(m_audioPacketPool, kMasterSampleRate, kMasterSamplesPerPacket, kProminentHrtfStreamCount);
	//m_compressor = new SpeexProcessor();
	//m_decompressor = new SpeexProcessor();
}


void AudioSessionProcessorServer::AddConnection(const NetworkConnectionPtr& connection)
{
	// TODO: Right now we only have Baraboo connections to deal with, but we'll need to refactor if we add new client types.
	AudioConnectionPtr newAudioConnectionClient = new AudioConnectionClient(connection, m_audioPacketPool);

	// Set up processing pipeline
	//newAudioConnectionClient->AddAudioProcessor(m_decompressor, Input);
	newAudioConnectionClient->AddAudioProcessor(m_routerWithMixer, Input);

	//newAudioConnectionClient->AddAudioProcessor(m_compressor, Output);

	ConnectionGUID connectionGUID = connection->GetConnectionGUID();

	m_audioConnections[connectionGUID] = newAudioConnectionClient;
	m_routerWithMixer->AddAudioConnection(newAudioConnectionClient);
}


void AudioSessionProcessorServer::RemoveConnection(const NetworkConnectionPtr& connection)
{
	ConnectionGUID connectionGUID = connection->GetConnectionGUID();
	AudioConnectionPtr audioConnection;

	audioConnection = m_audioConnections[connectionGUID];
	XTASSERT(audioConnection != NULL);

	m_routerWithMixer->RemoveAudioConnection(audioConnection);
	
	// Tear down processing pipeline
	//audioConnection->RemoveAudioProcessor(m_compressor, Input);
	audioConnection->RemoveAudioProcessor(m_routerWithMixer, Input);
	//audioConnection->RemoveAudioProcessor(m_decompressor, Output);

	m_audioConnections.erase(connectionGUID);
}


XTOOLS_NAMESPACE_END