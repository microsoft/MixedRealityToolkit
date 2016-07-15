// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License. See LICENSE in the project root for license information.

//////////////////////////////////////////////////////////////////////////
// AudioConnection.cpp
// Manages one connection for audio processing
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
