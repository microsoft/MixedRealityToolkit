//////////////////////////////////////////////////////////////////////////
// AudioConnection.cpp
//
// Manages one connection for audio processing
//
// Copyright (C) 2014 Microsoft Corp.  All Rights Reserved
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include <Private/AudioConnection.h>

XTOOLS_NAMESPACE_BEGIN

void AudioConnection::AddAudioProcessor(AudioProcessorPtr processor, ProcessingDirection direction)
{ 
	switch ( direction )
	{
	case Input:
		m_processorPipelineIn.push_back(processor);
		break;
	case Output:
		m_processorPipelineOut.push_back(processor);
		break;
	}
}

void AudioConnection::RemoveAudioProcessor(AudioProcessorPtr processor, ProcessingDirection direction)
{ 
	switch ( direction )
	{
	case Input:
		m_processorPipelineIn.remove(processor);
		break;
	case Output:
		m_processorPipelineOut.remove(processor);
		break;
	}
}

XTOOLS_NAMESPACE_END