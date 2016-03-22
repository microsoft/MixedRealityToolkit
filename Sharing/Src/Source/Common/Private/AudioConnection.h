//////////////////////////////////////////////////////////////////////////
// AudioConnection.h
//
// Manages one connection for audio processing
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#pragma once
#include <Private/AudioPacket.h>
#include <Private/AudioProcessor.h>

#include <list>

XTOOLS_NAMESPACE_BEGIN

typedef uint64 AudioConnectionGUID;

enum ProcessingDirection
{
	Input = 0,
	Output
};

class AudioConnection : public RefCounted
{
public:

	typedef enum AudioConnectionType : uint32
	{
		SecondaryClient = 1	// The client connected through a tunnel, formerly known as Baraboo.
	} AudioConnectionType;

	virtual AudioConnectionGUID GetConnectionGUID(void) const = 0;
	virtual bool IsMuted(void) const = 0;
	virtual bool IsConnected(void) const = 0;
	virtual bool SupportsHrtf(void) const = 0;
	virtual void SendAudioPacket(AudioPacketPtr& audioPacket) = 0;

	void AddAudioProcessor(AudioProcessorPtr processor, ProcessingDirection direction);
	void RemoveAudioProcessor(AudioProcessorPtr processor, ProcessingDirection direction);

protected:
	std::list<AudioProcessorPtr>	m_processorPipelineIn;
	std::list<AudioProcessorPtr>	m_processorPipelineOut;
};

DECLARE_PTR(AudioConnection)

XTOOLS_NAMESPACE_END
