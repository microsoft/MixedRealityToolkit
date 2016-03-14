//////////////////////////////////////////////////////////////////////////
// AudioSessionProcessor.h
//
// Provides common functionality for handling audio connections.
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once
#include <Private/AudioConnection.h>
#include <Private/AudioPacketPool.h>
#include <Private/XSessionProcessor.h>

#include <map>

XTOOLS_NAMESPACE_BEGIN

// =======================================================================
// Interface for providing audio session services, including managing 
// AudioConnections and establishing the processing pipeline.
// =======================================================================
class AudioSessionProcessor : public XSessionProcessor
{
public:
	static const uint32 kMasterSampleRate = 48000;
	static const uint16 kProminentHrtfStreamCount = 0;
	static const uint16 kMasterSamplesPerPacket = kMasterSampleRate / 50;		// ie. 20ms of data per packet
	static const uint32 kAudioPacketPoolReserveCount = 100;

	AudioSessionProcessor();

protected:
	typedef std::map<ConnectionGUID, AudioConnectionPtr> AudioConnectionMap;

	AudioConnectionMap			m_audioConnections;
};

DECLARE_PTR(AudioSessionProcessor)

XTOOLS_NAMESPACE_END