//////////////////////////////////////////////////////////////////////////
// AudioSessionProcessorClient.cpp
//
// Listens for audioSample messages then processes them and distributes
// appropriately to all connections.
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AudioSessionProcessorClient.h"
#include <Private/AudioConnectionClient.h>

XTOOLS_NAMESPACE_BEGIN

AudioSessionProcessorClient::AudioSessionProcessorClient()
{
	uint16 maxStreams = 3;		// one mixed mono and 2 HRTF mono streams.
	uint16 maxChannels = 1;
	uint32 maxSamplesPerPacket = 960;
	uint32 defaultQueueSizeSampleCount = maxSamplesPerPacket * maxStreams * maxChannels;

	m_audioPacketPool = new AudioPacketPool(defaultQueueSizeSampleCount);
	m_audioPacketPool->Reserve(defaultQueueSizeSampleCount, kAudioPacketPoolReserveCount);
}


void AudioSessionProcessorClient::AddConnection(const NetworkConnectionPtr& connection)
{
	// TODO: Right now we only have Baraboo connections to deal with, but we'll need to refactor if we add new client types.
	AudioConnectionPtr newAudioConnectionClient = new AudioConnectionClient(connection, m_audioPacketPool);

	// TODO: Set up processing pipeline

	ConnectionGUID connectionGUID = connection->GetConnectionGUID();

	m_audioConnections[connectionGUID] = newAudioConnectionClient;
}


void AudioSessionProcessorClient::RemoveConnection(const NetworkConnectionPtr& connection)
{
	ConnectionGUID connectionGUID = connection->GetConnectionGUID();
	AudioConnectionPtr audioConnection;

	audioConnection = m_audioConnections[connectionGUID];
	XTASSERT(audioConnection != NULL);

	// TODO: Tear down processing pipeline

	m_audioConnections.erase(connectionGUID);
}


XTOOLS_NAMESPACE_END